require 'Node'
require 'Manager'

#This class is responsible for initial parsing of node information from
#the filesystem (at the director startup-time)
class FilesystemNodeBuilder

  #Checks, if core node manager exists, and if so it all its current connections are parsed
  #Used for initial state read on startup
  #
  # Example:
  #   irb(main):022:0> nb.parseCoreManager(fs, n)
  #   D, [2014-01-29T10:18:27.468357 #12698] DEBUG -- : CNN: registering node  [192.168.1.2:40415]
  #   D, [2014-01-29T10:18:27.468629 #12698] DEBUG -- : CNN: registering node  [192.168.1.3:55747]
  #   D, [2014-01-29T10:18:27.469894 #12698] DEBUG -- : CNN: registering node  [192.168.1.4:52661]
  #   => #<CoreNodeManager:0x0000000232b818 @detachedNodes=[#<Node:0x00000002329ab8 @nodeId=nil, @networkAddress=#<NetworkAddress:0x00000002329b08 @ip="192.168.1.4", @port=52661, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:18:27 -0600, @state=0>, #<Node:0x0000000232a4e0 @nodeId=nil, @networkAddress=#<NetworkAddress:0x0000000232a530 @ip="192.168.1.3", @port=55747, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:18:27 -0600, @state=0>, #<Node:0x0000000232af08 @nodeId=nil, @networkAddress=#<NetworkAddress:0x0000000232af58 @ip="192.168.1.2", @port=40415, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:18:27 -0600, @state=0>], @connectedNodesCount=3>
  #
  def parseCoreManager (filesystemConnector, nodeRepository)
    #$log.debug "Parsing core managers, filesystemConnector = #{filesystemConnector}"
    return nil if !filesystemConnector.checkForCoreManager
    #$log.debug "Core manager present"
    parseCoreNodeManager(filesystemConnector, nodeRepository, CoreNodeManager.new)
  end

  #Reads all currently connected detached managers
  #Used for initial state read on startup
  #
  # Example:
  #   irb(main):020:0> nb.parseDetachedManagers(fs, n)
  #   D, [2014-01-29T10:15:56.624333 #12698] DEBUG -- : PEN: registering node  [192.168.1.2:54321]
  #   D, [2014-01-29T10:15:56.624660 #12698] DEBUG -- : PEN: registering node  [192.168.1.3:54321]
  #   D, [2014-01-29T10:15:56.624900 #12698] DEBUG -- : PEN: registering node  [192.168.1.4:54321]
  #   => [#<DetachedNodeManager:0x000000022e0188 @coreNode=#<Node:0x000000022e05e8 @nodeId=nil, @networkAddress=#<NetworkAddress:0x000000022e0660 @ip="192.168.1.4", @port=54321, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:15:56 -0600, @state=0>, @coreNodeSlotIndex=0>, #<DetachedNodeManager:0x000000022e0a48 @coreNode=#<Node:0x000000022e0ea8 @nodeId=nil, @networkAddress=#<NetworkAddress:0x000000022e0f20 @ip="192.168.1.3", @port=54321, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:15:56 -0600, @state=0>, @coreNodeSlotIndex=1>, #<DetachedNodeManager:0x000000022e1380 @coreNode=#<Node:0x000000022e1830 @nodeId=nil, @networkAddress=#<NetworkAddress:0x000000022e1880 @ip="192.168.1.2", @port=54321, @protocol="tcp">, @nodeInfo=nil, @staticNodeInfo=nil, @lastHeartBeatTime=2014-01-29 10:15:56 -0600, @state=0>, @coreNodeSlotIndex=2>]
  #
  def parseDetachedManagers (filesystemConnector, nodeRepository)
    #$log.debug "Parsing detached managers"
    return nil if !filesystemConnector.checkForDetachedManager
    #$log.debug "Detached manager present"
    parseDetachedManagerNodes(filesystemConnector, nodeRepository)
  end

  private

  def parseCoreNodeManager(filesystemConnector, nodeRepository, coreNodeManager)
    filesystemConnector.forEachCoreManagerNodeDir() { |slotIndex, fullFileName|
      ipAddress, port = readDataFromPeernameFile("#{fullFileName}/connections/ctrlconn/peername")
      placeHolderNode = nodeRepository.getNodeWithIp(ipAddress) # TODO: rewrite with ip + port
      if placeHolderNode.nil?
        placeHolderNode = Node.new(nil, NetworkAddress.new(ipAddress, port)) # Just a placeholder node with no id, not even registered to repository
      end
      $log.debug "CNN: registering node #{placeHolderNode}"
      coreNodeManager.registerDetachedNode(slotIndex.to_i, placeHolderNode)
    }
    coreNodeManager
  end

  def parseDetachedManagerNodes(filesystemConnector, nodeRepository)
    detachedManagers = []
    filesystemConnector.forEachDetachedManagerDir() { |slotIndex, fullFileName|
      ipAddress, port = readDataFromPeernameFile("#{fullFileName}/connections/ctrlconn/peername")
      placeHolderNode = nodeRepository.getNodeWithIp(ipAddress) # TODO: rewrite with ip + port
      if placeHolderNode.nil?
        placeHolderNode = Node.new(nil, NetworkAddress.new(ipAddress, port)) # Just a placeholder node with no id, not even registered to repository
      end
      if placeHolderNode.networkAddress.port == 0
        $log.debug "parseDetachedManagerNodes: Ignore node #{placeHolderNode}"
        next
      end
      $log.debug "PEN: registering node #{placeHolderNode}"
      detachedManagers[slotIndex.to_i] = DetachedNodeManager.new(placeHolderNode, slotIndex.to_i)
    }
    detachedManagers
  end

  def readDataFromPeernameFile(pathToFile)
    ipAddressAndPort = nil
    begin
      File.open(pathToFile, "r") { |peernameFile|
        ipAddressAndPortInline = peernameFile.readline("\0")
        ipAddressAndPort = ipAddressAndPortInline.split(":")
      }
      return [nil, nil] if ipAddressAndPort.length != 2
      ipAddressAndPort[0] = ipAddressAndPort[0].gsub(/\s+/, "")
      ipAddressAndPort[1] = ipAddressAndPort[1].gsub(/\s+/, "")
      $log.debug "readDataFromPeernameFile: #{ipAddressAndPort[0]}:#{ipAddressAndPort[1]}"
    rescue => err
      $log.error "readDataFromPeernameFile: #{err.message} \n#{err.backtrace.join("\n")}"
      return [nil, nil]
    end
    ipAddressAndPort
  end
end
