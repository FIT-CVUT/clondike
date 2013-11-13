require 'NodeInfo'

# Enum of states of the node
class NodeState
	# Standard state
	ALIVE=0
	# When the liveness monitoring suspects the node to be dead
	SUSPECTED=1
	# As far as we can say, the node died (crashed, disconnected without letting know..)
	# The node is eligible for removal from registered list of nodes
	DEAD=2
end

# Class, containing information about one cluster node
class Node
	# Unidue id of the node
	attr_reader :id # Cause warning: Object#id will be deprecated; use Object#object_id; TODO: rename id to nodeId
	# IP address, where is the node located
	attr_reader :ipAddress
	# Globally distributed information about node (like load, etc..)
	attr_reader :nodeInfo
	# Node state
	attr_reader :state
	# Timestamp of last heartbeat
	attr_reader :lastHeartBeatTime

	# Globally distributed information about node that does not change in time
	attr_accessor :staticNodeInfo

	def initialize(id, ipAddress)
		@id = id
		@ipAddress = ipAddress
		@nodeInfo = nil # We have no info in the beginning
		@staticNodeInfo = nil
		@lastHeartBeatTime = Time.now()
		@state = NodeState::ALIVE
	end

	def updateInfo(nodeInfo)
		#$log.debug "[Id: #{id}]UpdateInfo: #{@nodeInfo} -> #{nodeInfo}"
		@nodeInfo = nodeInfo if (!@nodeInfo || (@nodeInfo.timestamp < nodeInfo.timestamp))
	end

	def updateLastHeartBeatTime()
		@state = NodeState::ALIVE
		@lastHeartBeatTime = Time.now()
	end

	def markDead()
		$log.info "Marking node #{ipAddress} dead"
		@state = NodeState::DEAD
	end

	def ==(other)
		other.class == Node && @id == other.id
	end

	def to_s
		"Node - #{@ipAddress}"
	end
end

# Special case of node... current node ;)
# The difference is, it does not get nodeInfo from outside, but it is provided
# directly by the NodeInfoProvider
class CurrentNode<Node
	def initialize(id, ipAddress, staticInfo)
		super(id, ipAddress)
		@staticNodeInfo = staticInfo
	end

	# Method called on listener on NodeInfoProvider
	def notifyChange(nodeInfoWithId)
		# On change notification, we just update self info
		updateInfo(nodeInfoWithId.nodeInfo)
	end

	def self.createCurrentNode(nodeInfoProvider, localIP)
		CurrentNode.new(nodeInfoProvider.getCurrentId, localIP, nodeInfoProvider.getCurrentStaticInfo)
	end
end
