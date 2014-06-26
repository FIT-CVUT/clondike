require 'Manager'
require 'trust/TrustMessages.rb'
require 'Node'
require 'dht/Config'
require 'dht/Bootstrap'
require 'dht/DHTMessages'

require 'pp'

# This class keeps internal both core and detached node managers. Via these manager
# it keeps track about membership info about all cluster in which this machine
# participates
#
# Initially, it parses membership information from file
# Later, it responds to various external events, to keep membership information
# up to date
#
# This class is repsonsible for connecting new nodes (or disconnecting existing if they are no longer required) include bootstrapping
# When some other class wants to connect/disconnect remote nodes (for example LoadBalancer), it should tell this class
class MembershipManager
  # Manager of the core node
  attr_accessor :coreManager
  # Array of managers of all detached nodes on this node (every connection to remote CCN has its own dedicated datached manager)
  # Detached managers are placed in the array in the slot corresponding to their
  # slot index in kernel module
  # Slots with no manager are nil
  attr_accessor :detachedManagers

  # How many peers should manager try to keep connected even if there are no explicit peers requirements from higher layers.
  attr_accessor :minimumConnectedPeers

  def initialize(filesystemConnector, nodeRepository, trustManagement, interconnection)
    @filesystemConnector = filesystemConnector
    @nodeRepository = nodeRepository
    @trustManagement = trustManagement
    @interconnection = interconnection
    @minimumConnectedPeers = 50

    # We will use this messeges for connecting other node
    @interconnection.addReceiveHandler(PublicKeyDisseminationMessage, self)

    # Init core manager reference, if there is a core node registered on this machine
    @coreManager = FilesystemNodeBuilder.new().parseCoreManager(@filesystemConnector, @nodeRepository);

    # Check via fs api if there is dn registered
    @detachedManagers = FilesystemNodeBuilder.new().parseDetachedManagers(@filesystemConnector, @nodeRepository);

    @bootstrap = Bootstrap.new(@filesystemConnector, @nodeRepository, @trustManagement, @interconnection)
    @bootstrap.start()

    startAutoConnectingThread()
  end

  def prepareClusterForMeasurement(targetNumberOfNodes)
    getMorePeersToNodeRepository(targetNumberOfNodes)
    waitForVerificationNodes(targetNumberOfNodes)
    waitForAllFsConnection(targetNumberOfNodes)
  end

  def startAutoConnectingThread
    ExceptionAwareThread.new {
      loop do
        sleep(10)
        @nodeRepository.getAllNodes.each { |node|
          next if node == @nodeRepository.selfNode
          if @trustManagement.isVerified?(node.nodeId)
            unless containsDetachedNode(node)
              if node.networkAddress.class == NetworkAddress
                $log.debug "MembershipManager: AutoConnectingThread tries connect address #{node.networkAddress}"
                res = @filesystemConnector.connect(node.networkAddress, "") # TODO: fix missing authentication data!
              elsif
                $log.warn "Network address is invalid! #{node.networkAddress}"
              end
            end
          elsif
            # Not verified yet
            connectToNode(node)
          end
        }
      end
    }
  end

  def canConnect(authenticationData)
    verifyResult = @trustManagement.verifyAuthentication(authenticationData)
    return verifyResult != nil
  end

  # This callback is invoked, when a new remote node has connected to our core node
  def nodeConnected(address, slotIndex)
    chunks = address.split(":")
    if chunks.length != 2
      $log.error "MembershipManager: nodeConnected: invalid newtork address #{address}"
      require 'Util'
      showBacktrace()
      return nil
    end
    networkAddress = NetworkAddress.new(chunks[0], chunks[1])
    placeHolderNode = @nodeRepository.getNodeWithIp(networkAddress.ip)
    if placeHolderNode.nil?
      placeHolderNode = Node.new(nil, networkAddress) # Just a placeholder node with no id, not even registered to repository
    end
    $log.debug("Adding detached placeholder node: #{slotIndex} .. #{placeHolderNode}")
    @coreManager.registerDetachedNode(slotIndex, placeHolderNode)
  end

  # This callback is invoked, when remote node is disconnected (could be both core or detached remote node)
  def nodeDisconnected(managerSlot)
    $log.debug("Node disconnected: #{managerSlot}")
    if ( managerSlot.slotType == DETACHED_MANAGER_SLOT )
      $log.warn("Slot #{managerSlot.slotIndex} is already empty!") if !@detachedManagers[managerSlot.slotIndex]
      @detachedManagers[managerSlot.slotIndex] = nil
    else
      if ( !@coreManager.detachedNodes[managerSlot.slotIndex] )
        $log.warn("Core manager slot #{managerSlot.slotIndex} is already empty!")
        return
      end

      @coreManager.unregisterDetachedNode(managerSlot.slotIndex)
    end
  end

  def connectToNode(node)
    ExceptionAwareThread.new() {
      nodeIpAddress = node.networkAddress.ip
      nodePublicKey = @trustManagement.getKey(node.nodeId);

      if nodePublicKey then
        session = nil
        # If the node is verified already, we wont to verified again
        if @trustManagement.isVerified?(node.nodeId)
          negotiationSession = @trustManagement.getSession(node.nodeId)
          session = Session.new(nodePublicKey, negotiationSession.proof)
        elsif
          $log.debug("Trying verify #{node}")
          session = @trustManagement.authenticate(node.nodeId, nodePublicKey)
        end

        if session then
          # The Proof is prooved in trust/AuthenticationDispatcher.rb
          succeeded = @filesystemConnector.connect(node.networkAddress, session.authenticationProof)
          $log.info("Connection attempt to #{node.networkAddress} with proof #{session.authenticationProof} #{succeeded ? 'succeeded' : 'failed'}.")
          if succeeded
            slotIndex = @filesystemConnector.findDetachedManagerSlot(nodeIpAddress)
            if slotIndex.nil?
              $log.debug "MembershipManager.rb: connectToNode: no slotIndex find for node:"
              pp node
            else
              @detachedManagers[slotIndex] = DetachedNodeManager.new(node, slotIndex)
            end
          end
        end
      elsif
        # Do not know the key yet
        publicKeyDisseminationMessage = PublicKeyDisseminationMessage.new(@nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey, true)
        @interconnection.dispatch(node.nodeId, publicKeyDisseminationMessage)
      end
    }
  end

  def handleFrom(message, from)
    @trustManagement.registerKey(message.nodeId, message.publicKey)

    node = Node.new(message.nodeId, from)
    @nodeRepository.insertIfNotExists(node)

    unless containsDetachedNode(node)
      $log.debug "MembershipManager: PublicKeyDisseminationMessageHandler: connectToNode!"
      connectToNode(node)
    end

    if message.sendMeYours
      publicKeyDisseminationMessage = PublicKeyDisseminationMessage.new(@nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey)
      @interconnection.dispatch(from, publicKeyDisseminationMessage)
    end
  end

  private

  def containsDetachedNode(node)
    @detachedManagers = FilesystemNodeBuilder.new().parseDetachedManagers(@filesystemConnector, @nodeRepository);

    @detachedManagers.each { |manager|
      if ( manager != nil && node != nil && manager.coreNode.nodeId == node.nodeId )
        return true
      end
    }
    return false
  end

  def getMorePeersToNodeRepository(targetNumberOfNodes)
    $log.debug "getMorePeersToNodeRepository: start"
    @nodeRepository.printListOfAllNodes()
    lookUpNodeIdRequestMessage = LookUpNodeIdRequestMessage.new(@nodeRepository.selfNode.nodeId, targetNumberOfNodes, @nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey)
    while targetNumberOfNodes > (@nodeRepository.knownNodesCount() + 1)
      $log.debug "getMorePeersToNodeRepository: send requests to all known nodes"
      @interconnection.dispatch(nil, lookUpNodeIdRequestMessage)
      sleep(1)
    end
    $log.debug "getMorePeersToNodeRepository: end"
    @nodeRepository.printListOfAllNodes()
  end

  def waitForVerificationNodes(targetNumberOfNodes)
    $log.debug "waitForVerificationNodes: start"
    nodes = @nodeRepository.getAllRemoteNodes()
    loop {
      verifiedNodes = 1
      nodes.each { |node|
        verifiedNodes += 1 if @trustManagement.isVerified?(node.nodeId)
      }
      break if verifiedNodes >= targetNumberOfNodes
      sleep(1)
    }
    $log.debug "waitForVerificationNodes: end"
  end

  def waitForAllFsConnection(targetNumberOfNodes)
    $log.debug "waitForAllFsConnection: start"
    nodes = @nodeRepository.getAllRemoteNodes()
    loop {
      verifiedNodes = 1
      nodes.each { |node|
        verifiedNodes += 1 if containsDetachedNode(node)
      }
      break if verifiedNodes >= targetNumberOfNodes
      sleep(1)
    }
    $log.debug "waitForAllFsConnection: end"
  end
end
