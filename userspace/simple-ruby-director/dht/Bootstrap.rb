require 'set'
require 'monitor'

require 'dht/Config'
require 'dht/DHTMessages'

# This class is responsible for initial bootstraping, initial discovery nodes
# Implements Kademlia based discovery
class Bootstrap
  TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC = 1

  def initialize(filesystemConnector, nodeRepository, trustManagement, interconnection)
    @filesystemConnector = filesystemConnector
    @nodeRepository = nodeRepository
    @trustManagement = trustManagement
    @interconnection = interconnection

    @selfNodeId = @nodeRepository.selfNode.nodeId

    # A set of nodeIds of nodes already asked for LookUpNodeIdRequestMessage
    @requestedNodes = Set.new
    @requestedNodes.extend(MonitorMixin)

    @interconnection.addReceiveHandler(LookUpNodeIdRequestMessage, LookUpNodeIdRequestMessageHandler.new(@interconnection, @nodeRepository, @trustManagement))
    @interconnection.addReceiveHandler(LookUpNodeIdResponseMessage, LookUpNodeIdResponseMessageHandler.new(@interconnection, @nodeRepository, @requestedNodes, self, @trustManagement))
  end

  def start
    ExceptionAwareThread.new() {
      initialJoinAtLeastOneNode()
      $log.debug "Bootstrap: Successful joined at least to one node!"

      until kClosestNodesWereRequested do
        kClosestNodes = getClosestNodes(DHTConfig::K)
        parallelConnection = 0
        kClosestNodes.each { |node|
          @requestedNodes.synchronize {
            unless @requestedNodes.include?(node.nodeId)
              initLookUpNodeIdRequest(node) if parallelConnection < DHTConfig::ALPHA
              parallelConnection += 1
            end
          }
        }
        sleep(TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC)
      end
    }
  end

  def kClosestNodesWereRequested
    kClosestNodes = getClosestNodes(DHTConfig::K)
    kClosestNodes.each { |node|
      unless @requestedNodes.include?(node.nodeId)
        # $log.debug "Bootsrap: kClosestNodesWereRequested fail due node #{node.inspect}, #{@requestedNodes.inspect}"
        return false
      end
    }
    $log.debug "Bootsrap: kClosestNodesWereRequested: TRUE!"
    return true
  end

  private

  def initLookUpNodeIdRequest(node)
    lookUpNodeIdRequestMessage = LookUpNodeIdRequestMessage.new(@selfNodeId, DHTConfig::K, @nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey)
    #@interconnection.dispatch(node.nodeId, lookUpNodeIdRequestMessage, DeliveryOptions::ACK_8_SEC)
    @interconnection.dispatch(node.nodeId, lookUpNodeIdRequestMessage)
  end

  def getClosestNodes(count)
    @nodeRepository.getClosestNodesTo(@selfNodeId, count)
  end

  def initialJoinAtLeastOneNode
    @publicKeyDisseminationMessage = PublicKeyDisseminationMessage.new(@nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey, true)

    bootstrapNodes = @filesystemConnector.getBootstrapNodes().shuffle
    bootstrapNodes.delete_if { |networkAddress| networkAddress == @nodeRepository.selfNode.networkAddress }

    dispatchAsyncPKDMessages(bootstrapNodes)

    loop {
      sleep(TIMEOUT_FOR_MESSAGE_RESPONSE_IN_SEC)
      break if bootstrapNodes.empty?
      break if areWeJoinedAtLeastOneNode?
      dispatchAsyncPKDMessages(bootstrapNodes)
    }
  end

  def dispatchAsyncPKDMessages(bootstrapNodes)
    for index in 1..[DHTConfig::ALPHA, bootstrapNodes.length].min do
      choosedAddress = bootstrapNodes.shift
      bootstrapNodes.push(choosedAddress)
      #@interconnection.dispatch(choosedAddress, @publicKeyDisseminationMessage, DeliveryOptions::ACK_8_SEC)
      @interconnection.dispatch(choosedAddress, @publicKeyDisseminationMessage)
      $log.debug "#{@nodeRepository.selfNode} --> #{choosedAddress}"
    end
  end

  def areWeJoinedAtLeastOneNode?
    return (@nodeRepository.knownNodesCount > 0) ? true : false
  end
end

