require 'MonitoringMessages'

# This class is responsible for monitoring node connectivity
#
# Monitoring nodes via 2 channels:
#   1) Ruby socket
#   2) Kernel link
#
class ManagerMonitor
  # Time withouth heartbeat in second when node is considered dead
  DEAD_TIMEOUT = 60
  HEART_BEAT_PERIOD = 10  # In seconds

  def initialize(interconnection, membershipManager, nodeRepository, filesystemConnector)
    @interconnection = interconnection
    @membershipManager = membershipManager
    @nodeRepository = nodeRepository
    @currentNodeId = nodeRepository.selfNode.nodeId
    @filesystemConnector = filesystemConnector

    @interconnection.addReceiveHandler(HeartBeatKernelLinkMessage, HeartBeatKernelLinkHandler.new(membershipManager, @nodeRepository)) if ( interconnection )
    @interconnection.addReceiveHandler(HeartBeatRubySocketMessage, HeartBeatRubySocketHandler.new(@nodeRepository)) if ( interconnection )
  end

  # Starts background threads
  def start
    ExceptionAwareThread.new() {
      heartBeatingThread()
    }

    ExceptionAwareThread.new() {
      updateAllNodesStatusThread()
    }

    ExceptionAwareThread.new() {
      purgeDeadNodesThread()
    }
  end
  private
  def heartBeatingThread()
    while true do
      startTime = Time.now()
      emitHeartBeats()
      sleep(HEART_BEAT_PERIOD - (Time.now() - startTime))
    end
  end

  # Thread updating status of nodes based on last heartbeat message seen
  def updateAllNodesStatusThread()
    while true do
      now = Time.now()
      @nodeRepository.getAllRemoteNodes.each { |node|
        @nodeRepository.updateState(node.nodeId, NodeState::DEAD) if (now - node.lastHeartBeatTime) > DEAD_TIMEOUT
      }

      @membershipManager.coreManager.detachedNodes.each_with_index { |element, slotIndex|
        next if !element
        next if element.state == NodeState::DEAD
        element.markDead if ( now - element.lastHeartBeatTime > DEAD_TIMEOUT )
      }

      @membershipManager.detachedManagers.each_with_index { |element, slotIndex|
        next if !element
        next if element.coreNode.state == NodeState::DEAD
        element.coreNode.markDead if ( now - element.coreNode.lastHeartBeatTime > DEAD_TIMEOUT )
      }

      sleep 2
    end
  end

  # Thread purging dead nodes
  # TODO: Attemp kill only if disconnect fails.. and maybe add separate asynchronicity for it?
  def purgeDeadNodesThread()
    while true do
      @nodeRepository.getAllRemoteNodes.each { |node|
        next if node.state != NodeState::DEAD

        $log.info("Disconnecting #{node} due to too many missed heartbeats")
        @nodeRepository.purgeNode node.nodeId
      }

      @membershipManager.coreManager.detachedNodes.each_with_index { |element, slotIndex|
        next if !element
        next if element.state != NodeState::DEAD

        $log.info("Disconnecting core node index #{slotIndex} due to too many missed heartbeats")
        pp @membershipManager.coreManager.detachedNodes[slotIndex]

        @filesystemConnector.disconnectNode(CORE_MANAGER_SLOT, slotIndex)
        sleep 5
        @filesystemConnector.killNode(CORE_MANAGER_SLOT, slotIndex)
      }

      @membershipManager.detachedManagers.each_with_index { |element, slotIndex|
        next if !element
        next if element.coreNode.state != NodeState::DEAD

        $log.info("Disconnecting detached node index #{slotIndex} due to too many missed heartbeats")
        pp @membershipManager.detachedManagers[slotIndex]

        @filesystemConnector.disconnectNode(DETACHED_MANAGER_SLOT, slotIndex)
        sleep 5
        @filesystemConnector.killNode(DETACHED_MANAGER_SLOT, slotIndex)
      }

      sleep 2
    end
  end

  def emitHeartBeats
    # Ruby Socket
    @nodeRepository.getAllRemoteNodes.each { |node|
      @interconnection.dispatch(node.nodeId, HeartBeatRubySocketMessage.new(@nodeRepository.selfNode.nodeId))
    }

    # Kernel link channel
    @membershipManager.coreManager.detachedNodes.each_with_index { |element, slotIndex|
      next if !element
      emitHeartBeat(CORE_MANAGER_SLOT, slotIndex, element);
    }
    @membershipManager.detachedManagers.each_with_index { |element, slotIndex|
      next if !element
      emitHeartBeat(DETACHED_MANAGER_SLOT, slotIndex, element.coreNode);
    }
  end

  def emitHeartBeat(slotType, slotIndex, node)
    managerSlot = ManagerSlot.new(slotType, slotIndex)
    if ( !node )
      $log.warn("Failed to resolve node for slot #{managerSlot}")
    end

    message = HeartBeatMessage.new(@currentNodeId)
    @interconnection.dispatchToSlot(managerSlot, message)
  end
end

class HeartBeatKernelLinkHandler
  def initialize(membershipManager, nodeRepository)
    @membershipManager = membershipManager
    @nodeRepository = nodeRepository
  end

  # TODO: Proper sync of this method?
  def handleFrom(heartBeatMessage, fromManagerSlot)
    nodeId = heartBeatMessage.nodeId

    if ( fromManagerSlot.slotType == CORE_MANAGER_SLOT )
      node = @membershipManager.coreManager.detachedNodes[fromManagerSlot.slotIndex]
      if node != nil && node.nodeId.nil?
        $log.debug "Updated core node id #{nodeId} from a heartbeat message"
        #@nodeRepository.insertIfNotExists(node)
        @membershipManager.coreManager.unregisterDetachedNode(fromManagerSlot.slotIndex)
        @membershipManager.coreManager.registerDetachedNode(fromManagerSlot.slotIndex,node)
      end
      node.updateLastHeartBeatTime()
    else
      unless @membershipManager.detachedManagers[fromManagerSlot.slotIndex].nil?
        node = @membershipManager.detachedManagers[fromManagerSlot.slotIndex].coreNode
        if node.nodeId.nil?
          $log.debug "Updated detached node id #{nodeId} from a heartbeat message"
          #node, isNew = @nodeRepository.getOrCreateNode(nodeId, node.ipAddress)
          @membershipManager.detachedManagers[fromManagerSlot.slotIndex].coreNode = node
        end
        node.updateLastHeartBeatTime()
      end
    end
  end
end

class HeartBeatRubySocketHandler
  def initialize(nodeRepository)
    @nodeRepository = nodeRepository
  end

  # TODO: Proper sync of this method?
  def handleFrom(heartBeatMessage, fromNetworkAddress)
    @nodeRepository.updateLastHeartBeatTime fromNetworkAddress
    $log.debug "HeartBeatRubySocketHandler: #{heartBeatMessage};; #{fromNetworkAddress}"
  end
end
