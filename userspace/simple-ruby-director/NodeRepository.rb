require 'monitor'
require 'dht/BucketManager'
require 'BroadcastEnds'

#This class keeps track of all nodes we've ever heard about
class NodeRepository
  # Information about "this" node
  attr_reader :selfNode

  def initialize(selfNode, messageDispatcher)
    @selfNode = selfNode
    @bucketManager = BucketManager.new(@selfNode)
    @bucketManager.extend(MonitorMixin)

    @weKnowAll = false # For measure
    @messageDispatcher = messageDispatcher # For measure

    ExceptionAwareThread.new {
      loop do
        $log.debug "In NodeRepository are nodes:"
        getAllNodes.each { |node|
          $log.debug "  #{node}"
        }
        sleep(30)
      end
    }
  end

  def getNode(nodeId)
    @bucketManager.synchronize {
      return @bucketManager.getNode(nodeId)
    }
  end

  def insertOrReplaceNode(node)
    @bucketManager.synchronize {
      @bucketManager.insertOrReplaceNode(node)
    }
    checkBroadcastDiscoverEnd() # For measure
  end

  def insertIfNotExists(node)
    @bucketManager.synchronize {
      @bucketManager.insertOrReplaceNode(node) if @bucketManager.getNode(node.nodeId).nil?
    }
    checkBroadcastDiscoverEnd() # For measure
  end

  # Include self node
  def getAllNodes
    nodesClone = nil
    @bucketManager.synchronize {
      nodesClone = @bucketManager.getAllNodes.dup
    }
    return nodesClone
  end

  def getContactToNode(nodeId)
    node = nil
    @bucketManager.synchronize {
      node = @bucketManager.getNode(nodeId)
    }
    return nil if node.nil?
    return node.networkAddress
  end

  # the node with the exact nodeId is NOT include in the returned array of nodes
  def getClosestNodesTo(nodeId, count = DHTConfig::K)
    closestNodes = nil
    @bucketManager.synchronize {
      closestNodes = @bucketManager.getClosestNodesTo(nodeId, count).dup
    }
    return closestNodes
  end

  def updateLastHeartBeatTime(networkAddress)
    getNodesCopy.each { |node|
      if node.networkAddress == networkAddress
        @bucketManager.synchronize {
          @bucketManager.updateLastHeartBeatTime(node.nodeId)
        }
        return
      end
    }
  end

  def updateInfo(nodeId, nodeInfo)
    @bucketManager.synchronize {
      @bucketManager.updateInfo(nodeId, nodeInfo)
    }
  end

  def updateState(nodeId, nodeState)
    @bucketManager.synchronize {
      @bucketManager.updateState(nodeId, nodeState)
    }
  end

  # Count of known 'remote' nodes. Does not include self
  def knownNodesCount
    return(getAllNodes.length-1)
  end

  # For binding tasks on IP Addresses in MeasuementPlan
  def getNodeWithIp(nodeIp)
    return nil if nodeIp.nil?
    getAllNodes.each { |node|
      return node if node.networkAddress.ip == nodeIp
    }
    return nil
  end

  def getNodeWithNetworkAddress(networkAddress)
    return nil if networkAddress.class != NetworkAddress
    getAllNodes.each { |node|
      return node if node.networkAddress == networkAddress
    }
    return nil
  end

  private

  # For measure
  def checkBroadcastDiscoverEnd
    if @weKnowAll == false && (knownNodesCount()+1) >= BroadcastEnds::NODES
      $log.debug "Bootstrap: Successfully Completed! SendedMsgs #{@messageDispatcher.countSendedMsgs} RecvedMsgs #{@messageDispatcher.countRecvedMsgs}"
      @weKnowAll = true
    end
  end
end
