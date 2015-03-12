# Generic template for the heart-beat message
class HeartBeatMessage
  # Id of the sender node
  attr_reader :nodeId

  def initialize(nodeId)
    @nodeId = nodeId
  end
end

# HeartBeatMessages send through a kernel link channels
class HeartBeatKernelLinkMessage < HeartBeatMessage
end

# HeartBeatMessages send through a Ruby socket channel (MessageDispatcher)
class HeartBeatRubySocketMessage < HeartBeatMessage
end

