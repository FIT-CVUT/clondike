require 'set'
require 'monitor'

require 'dht/Config'
require 'dht/DHTMessages'

# This class is responsible for initial bootstraping, initial discovery nodes
# Implements Kademlia based discovery
class Bootstrap
  TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC = 8

  def initialize(filesystemConnector, nodeRepository, trustManagement, interconnection)
    @filesystemConnector = filesystemConnector
    @nodeRepository = nodeRepository
    @trustManagement = trustManagement
    @interconnection = interconnection

    @selfNodeId = @nodeRepository.selfNode.nodeId

    # A set of nodeIds of nodes already asked for LookUpNodeIdRequestMessage
    @requestedNodes = Set.new
    @requestedNodes.extend(MonitorMixin)

   @interconnection.addReceiveHandler(LookUpNodeIdRequestMessage, LookUpNodeIdRequestMessageHandler.new(@interconnection, @nodeRepository))
   @interconnection.addReceiveHandler(LookUpNodeIdResponseMessage, LookUpNodeIdResponseMessageHandler.new(@interconnection, @nodeRepository, @requestedNodes))
  end

  def start
    ExceptionAwareThread.new() {
      initialJoinAtLeastOneNode()
      $log.debug "Bootstrap: Successful joined at least to one node!"

      until kClosestNodesWereRequested do
        alphaClosestNodes = getClosestNodes(DHTConfig::ALPHA)
        alphaClosestNodes.each { |node|
          @requestedNodes.synchronize {
            initLookUpNodeIdRequest(node) unless @requestedNodes.include?(node.nodeId)
          }
        }
        sleep(TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC)
      end
      $log.debug "Bootstrap: Successful Completed!"
      @nodeRepository.getAllNodes.each { |node|
        $log.debug "  #{node}"
      }
    }
  end

  private

  def initLookUpNodeIdRequest(node)
    $log.debug "Sending LookUpNodeIdRequest to #{node}"
    lookUpNodeIdRequestMessage = LookUpNodeIdRequestMessage.new(@selfNodeId, DHTConfig::K)
    #@interconnection.dispatch(node.nodeId, lookUpNodeIdRequestMessage, DeliveryOptions::ACK_8_SEC)
    @interconnection.dispatch(node.nodeId, lookUpNodeIdRequestMessage)
  end

  def kClosestNodesWereRequested
    kClosestNodes = getClosestNodes(DHTConfig::K)
    kClosestNodes.each { |node|
      unless @requestedNodes.include?(node.nodeId)
        $log.debug "Bootsrap: kClosestNodesWereRequested fail due node #{node.inspect}, #{@requestedNodes.inspect}"
        return false
      end
    }
    $log.debug "Bootsrap: kClosestNodesWereRequested: TRUE!"
    return true
  end

  def getClosestNodes(count)
    @nodeRepository.getClosestNodesTo(@selfNodeId, count)
  end

  def initialJoinAtLeastOneNode
    @publicKeyDisseminationMessage = PublicKeyDisseminationMessage.new(@nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey, true)

    bootstrapNodes = @filesystemConnector.getBootstrapNodes().shuffle
    bootstrapNodes.delete_if { |networkAddress| networkAddress == @nodeRepository.selfNode.networkAddress }

    dispatchAsyncPKDMessages(bootstrapNodes)

    sendingMessagesToBootstrapNodesThread = ExceptionAwareThread.new {
      loop {
        sleep(TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC)
        break if bootstrapNodes.empty?
        dispatchAsyncPKDMessages(bootstrapNodes)
      }
    }

    # Wait for appear some 'remote' nodes in NodeRepository
    # TODO: Nicer will be non-active waiting via Monitor, VariableCondition, Wait and Signal/Broadcast
    loop do
      break if areWeJoinedAtLeastOneNode?
      sleep(1)
    end
    sendingMessagesToBootstrapNodesThread.kill
  end

  def dispatchAsyncPKDMessages(bootstrapNodes)
    for index in 1..[DHTConfig::ALPHA, bootstrapNodes.length].min do
      choosedAddress = bootstrapNodes.shift
      bootstrapNodes.push(choosedAddress)
      #@interconnection.dispatch(choosedAddress, @publicKeyDisseminationMessage, DeliveryOptions::ACK_8_SEC)
      @interconnection.dispatch(choosedAddress, @publicKeyDisseminationMessage)
    end
  end

  def areWeJoinedAtLeastOneNode?
    return (@nodeRepository.knownNodesCount > 0) ? true : false
  end
end

