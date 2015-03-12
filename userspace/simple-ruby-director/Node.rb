require 'NodeInfo'
require 'NetworkAddress'

# Enum of states of the node
# Improving: http://gistflow.com/posts/682-ruby-enums-approaches
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
  attr_reader :nodeId
  # IP address and port, where is the node located
  attr_reader :networkAddress
  # Globally distributed information about node (like load, etc..)
  attr_reader :nodeInfo
  # Node state
  attr_reader :state
  # Timestamp of last heartbeat
  attr_reader :lastHeartBeatTime

  # Globally distributed information about node that does not change in time
  attr_accessor :staticNodeInfo

  def initialize(nodeId, networkAddress)
    @nodeId = nodeId.to_s
    @networkAddress = networkAddress
    if @networkAddress.class != NetworkAddress
      $log.warn "Node.new: Appeared non-valid network address: #{@networkAddress}"
      require 'Util'
      showBacktrace()
    end
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

  def updateState(nodeState)
    $log.info "Marking node #{networkAddress} as #{nodeState}"
    @state = nodeState
  end

  def markDead
    @state = NodeState::DEAD
  end

  def ==(other)
    (other.class == Node || other.class == SelfNode) && @nodeId == other.nodeId
  end

  def getDistanceTo(nodeId)
    @nodeId.hex ^ nodeId.hex
  end

  def to_s
    #"#{nodeId} [#{networkAddress}]"
    "#{networkAddress}"
  end
end

# The difference is, it does not get nodeInfo from outside, but it is provided
# directly by the NodeInfoProvider
class SelfNode<Node
  def initialize(nodeId, networkAddress, staticInfo)
    super(nodeId, networkAddress)
    @staticNodeInfo = staticInfo
  end

  # Method called on listener on NodeInfoProvider
  def notifyChange(nodeInfoWithId)
    # On change notification, we just update self info
    updateInfo(nodeInfoWithId.nodeInfo)
  end

  def self.createSelfNode(nodeInfoProvider, networkAddress)
    SelfNode.new(nodeInfoProvider.getCurrentId, networkAddress, nodeInfoProvider.getCurrentStaticInfo)
  end
end
