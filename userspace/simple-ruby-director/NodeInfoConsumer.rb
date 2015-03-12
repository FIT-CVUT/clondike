require 'set'

#This class is consuming information received by the info distribution
#strategy and distributing them to interested resources
class NodeInfoConsumer
  def initialize(nodeRepository)
    @nodeRepository = nodeRepository
    @updateListeners = Set.new
    @selfNodeId = @nodeRepository.selfNode.nodeId
  end

  def registerUpdateListener(listener)
    @updateListeners.add(listener)
  end

  # Called, when a new information is received
  def infoReceived(info)
    #We are not interested in info about us ;)
    return if info.nodeId == @selfNodeId

    @nodeRepository.updateInfo(info.nodeId, info.nodeInfo)
    node = @nodeRepository.getNode(info.nodeId)
    @updateListeners.each { |listener| listener.notifyUpdate(node, info.nodeInfo) }
  end
end
