require 'set'
require 'Util'
require 'NetworkAddress'
require 'monitor'

# This class interacts with the kernel module via its exported pseudo-fs
class FilesystemConnector

  def initialize
    @rootPath = "/clondike"
    @coreRootPath = "#{@rootPath}/ccn"
    @detachedRootPath = "#{@rootPath}/pen"

    @connectAddresses = Set.new
    @connectAddresses.extend(MonitorMixin)

    startWritingThread()

  end

  def startWritingThread
    ExceptionAwareThread.new {

      # The authString is pretty good idea, but this:
      #
      #    authString = authenticationData != nil ? "@#{authenticationData}" : ""
      #    `echo tcp:#{ipAddress}:54321#{authString} > #{@detachedRootPath}/connect`
      #
      # caused creating directory structure like this:
      #    /clondike/pen/nodes/192.168.0.4:54321
      #                                    ^^^^^
      # TODO: Fix the one that serving about /clondike/pen/connect and learn it about a authString
      
      loop {
        unless @connectAddresses.empty?
          contactedAddress = nil
          @connectAddresses.synchronize {
            @connectAddresses.each { |networkAddress|
              contactedAddress = networkAddress
              break
            }
          }
          $log.debug("echo tcp:#{contactedAddress.ip}:#{contactedAddress.port} > #{@detachedRootPath}/connect")
          timer = Time.now
          `echo tcp:#{contactedAddress.ip}:#{contactedAddress.port} > #{@detachedRootPath}/connect` # This is time expensive, units of seconds
          $log.debug("FilesystemConnector.connect(#{contactedAddress.ip}:#{contactedAddress.port}) took #{Time.now - timer} sec, Result: #{($? == 0?"SUCCESS":"FAILED")}")
          @connectAddresses.delete(contactedAddress)
        end
        sleep(1) # Active waiting TODO: make here waiting for signal from connect()
      }
    }
  end

  # Returns true, if core manager is registered on this computer
  def checkForCoreManager
    # No CCN support
    return false if !File.exists?(@coreRootPath)
    # Check, if CCN is listening (there is . and .., so if there are more then 2 element, there is a listening)
    (Dir.entries("#{@coreRootPath}/listening-on/").size  > 2)
  end

  # Iterates over all registered core manager node dirs
  def forEachCoreManagerNodeDir(&block)
    Dir.foreach("#{@coreRootPath}/nodes/") { |filename|
      fullPath = "#{@coreRootPath}/nodes/#{filename}";
      next if !File.directory?(fullPath)
      next if File.symlink?(fullPath)
      next if filename =~ /^\..?/ # Exclude . and ..
      block.call(filename, fullPath)
    }
  end

  # Returns true, if detached manager is registered on this computer
  def checkForDetachedManager
    File.exists?(@detachedRootPath)
  end

  # Iterates over all registered detached manager dirs
  def forEachDetachedManagerDir(&block)
    Dir.foreach("#{@detachedRootPath}/nodes/") { |filename|
      fullPath = "#{@detachedRootPath}/nodes/#{filename}";
      next if !File.directory?(fullPath)
      next if File.symlink?(fullPath)
      next if filename =~ /^\..?/ # Exclude . and ..
      block.call(filename, fullPath)
    }
  end

  #Returns slot, where is the detached manager with a specified ID connected
  def findDetachedManagerSlot(lookupIpAddress)
    resultSlotIndex = nil
    forEachDetachedManagerDir() { |slotIndex, fullFileName|
      File.open("#{fullFileName}/connections/ctrlconn/peername", "r") { |aFile|
        readData = aFile.readline("\0")
        ipAddress = readData.split(":").first
        if ipAddress == lookupIpAddress
          $log.debug("Found ip address #{ipAddress} at slot #{slotIndex}")
          resultSlotIndex = slotIndex.to_i
        end
      }
    }
    resultSlotIndex
  end

  def migrateHome(slotType, pid)
    root = getRoot(slotType)
    `echo #{pid}  > #{root}/mig/migrate-home`
  end

  def emigratePreemptively(index, pid)
    root = getRoot(CORE_MANAGER_SLOT)

    `echo #{pid} #{index}  > #{root}/mig/emigrate-ppm-p`
  end

  # Returns id of node, specified by its address
  def findNodeIdByAddress(ipAddress)
    #For now, we can simply return address, since it is used as
    #a node id, later we have to read that from fs
    #ipAddress
    raise "Unsupported operation"
  end

  #Tries to connect to a remote core node. Returns boolean informing about
  #the connection attempt result
  def connect(networkAddress, authenticationData)
    @connectAddresses.synchronize {
      @connectAddresses.add(networkAddress)
    }
    return true
  end

  # Attempt to gracefully disconnect node
  def disconnectNode(slotType, index)
    root = getRoot(slotType)
    `echo 1 > #{root}/nodes/#{index}/stop`
  end

  # Forcefully disconnects node
  def killNode(slotType, index)
    root = getRoot(slotType)
    `echo 1 > #{root}/nodes/#{index}/kill`
  end

  def getLocalNetworkAddress
    NetworkAddress.new(getListenData["ip"], getListenData["port"], getListenData["protocol"])
  end

  def getBootstrapNodes
    bootstrapListFile = "#{Dir.pwd}/BootstrapList.txt"
    bootstrapList = []
    File.open(bootstrapListFile, "r") do |file|
      file.each_line do |line|
        address = line.match(/([\d\.]+):?(\d+)/)
        unless address
          $log.warn "In reading #{bootstrapListFile} skip the line: '#{line.strip}'"
          next
        end
        ip = address[1].gsub(/\s+/, "")
	      port = address[2].gsub(/\s+/, "")
        bootstrapList.push(NetworkAddress.new(ip, port))
      end
    end
    bootstrapList.each { |addr|
      $log.debug("getBootstrapList: #{addr}")
    }
    return bootstrapList
  end

  private

  def getRoot(slotType)
    root = @detachedRootPath
    if ( slotType == CORE_MANAGER_SLOT )
      root = @coreRootPath
    end
    return root
  end

  # Read listen data from filesystem and return hash for example:
  #   {"protocol" => "tcp", "ip" => "192.168.1.1", "port" => "54321"}
  def getListenData
    protocol = ""
    ip = ""
    port = ""
    File.open("#{@coreRootPath}/listen", "r") { |listenFile|
      readData = listenFile.readline("\0")
      data = readData.split(":")
      protocol  = data[0].gsub(/\s+/, "")
      ip        = data[1].gsub(/\s+/, "")
      port      = data[2].gsub(/\s+/, "")
    }
    {
      "protocol"  => protocol,
      "ip"        => ip,
      "port"      => port,
    }
  end
end
