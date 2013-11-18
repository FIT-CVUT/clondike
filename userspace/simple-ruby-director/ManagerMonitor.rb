require 'MonitoringMessages'

# This class is responsible for monitoring node connectivity
class ManagerMonitor
	# Time withouth heartbeat in second when node is considered dead
	DEAD_TIMEOUT = 60

	def initialize(interconnection, membershipManager, nodeRepository, filesystemConnector)
		@interconnection = interconnection
		@membershipManager = membershipManager
		@currentNodeId = nodeRepository.selfNode.nodeId
		@filesystemConnector = filesystemConnector
		# In seconds
		@heartBeatPeriod = 10

		@interconnection.addReceiveHandler(HeartBeatMessage, HeartBeatHandler.new(membershipManager, nodeRepository, filesystemConnector)) if ( interconnection )
	end

	# Starts background threads
	def start
		ExceptionAwareThread.new() {
			heartBeatingThread();
		}

		ExceptionAwareThread.new() {
			updateAllNodesStatusThread()
		}

		ExceptionAwareThread.new() {
			purgeDeadNodesThread();
		}
	end
	private
	def heartBeatingThread()
		while true do
			emitHeartBeats()
			sleep @heartBeatPeriod # This is not correct, we should actually sleep just the remaining portion of time..
		end
	end

	# Thread updating status of nodes based on last hearthbeat seen
	def updateAllNodesStatusThread()
		while true do
			now = Time.now()
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
		$log.debug "Will Emit the HeartBeat"
		@interconnection.dispatchToSlot(managerSlot, message)
	end
end

class HeartBeatHandler
	def initialize(membershipManager, nodeRepository, filesystemConnector)
		@membershipManager = membershipManager
		@nodeRepository = nodeRepository
		@filesystemConnector = filesystemConnector
	end

	# TODO: Proper sync of this method?
	def handleFrom(heartBeatMessage, fromManagerSlot)
		nodeId = heartBeatMessage.nodeId
		#p heartBeatMessage

		# Will update detached manageres from filesystem; This fix disconnecting
		fsDetachedManagers = FilesystemNodeBuilder.new().parseDetachedManagers(@filesystemConnector, @nodeRepository);
		fsDetachedManagers.each { |detachedManager|
			if detachedManager.nil?
				$log.debug "Weird: From FileSystem I read nil detachedManager. I skip it."
				pp fsDetachedManagers
				next
			end

			if @membershipManager.detachedManagers[detachedManager.coreNodeSlotIndex].nil?
				@membershipManager.detachedManagers[detachedManager.coreNodeSlotIndex] = detachedManager
				$log.debug "Adding new detached manager #{detachedManager}"
				pp detachedManager
			end
		}
		pp @membershipManager.detachedManagers
		
		# Will update core nodes from filesystem; This fix disconnecting after 1 node left
		fsCoreManager = FilesystemNodeBuilder.new().parseCoreManager(@filesystemConnector, @nodeRepository);
		if fsCoreManager.connectedNodesCount != @membershipManager.coreManager.connectedNodesCount
			$log.warn "membershipManager.CoreManager and newly read from filesystem coreManager have difference in count of nodes!"
			pp @membershipManager.coreManager
			pp fsCoreManager
		end
		pp @membershipManager.coreManager

		# fsCoreManager.each { |detachedManager|
		# 	if @membershipManager.detachedManagers[detachedManager.coreNodeSlotIndex].nil?
		# 		@membershipManager.detachedManagers[detachedManager.coreNodeSlotIndex] = detachedManager
		# 		$log.debug "Adding new core manager #{detachedManager}"
		# 	end
		# }

		if ( fromManagerSlot.slotType == CORE_MANAGER_SLOT )
			node = @membershipManager.coreManager.detachedNodes[fromManagerSlot.slotIndex]
			if node.nodeId.nil?
				$log.debug "Updated core node id #{nodeId} from a heartbeat message"
				node, isNew = @nodeRepository.getOrCreateNode(nodeId, node.ipAddress)
				@membershipManager.coreManager.unregisterDetachedNode(fromManagerSlot.slotIndex)
				@membershipManager.coreManager.registerDetachedNode(fromManagerSlot.slotIndex,node)
			end

			node.updateLastHeartBeatTime();
		else
			node = @membershipManager.detachedManagers[fromManagerSlot.slotIndex].coreNode
			if node.nodeId.nil?
				$log.debug "Updated detached node id #{nodeId} from a heartbeat message"
				node, isNew = @nodeRepository.getOrCreateNode(nodeId, node.ipAddress)
				@membershipManager.detachedManagers[fromManagerSlot.slotIndex].coreNode = node
			end

			node.updateLastHeartBeatTime();
		end
	end
end
