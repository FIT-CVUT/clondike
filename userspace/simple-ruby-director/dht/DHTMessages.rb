class LookUpNodeIdRequestMessage
  attr_reader :lookUpNodeId
  attr_reader :countClosestNodes
  def initialize(lookUpNodeId, countClosestNodes)
    @lookUpNodeId = lookUpNodeId
    @countClosestNodes = countClosestNodes
  end
end

class LookUpNodeIdRequestMessageHandler
  def initialize(interconnection, nodeRepository)
    @interconnection = interconnection
    @nodeRepository = nodeRepository
  end
  def handleFrom(message, from)
    closestNodes = @nodeRepository.getClosestNodesTo(message.lookUpNodeId, message.countClosestNodes)
    closestNodesWithoutSelfNodeClass = []
    closestNodes.each { |node|
      closestNodesWithoutSelfNodeClass.push(Node.new(node.nodeId, node.networkAddress)) if node == @nodeRepository.selfNode
      closestNodesWithoutSelfNodeClass.push(node)
    }
    responseMessage = LookUpNodeIdResponseMessage.new(closestNodesWithoutSelfNodeClass)
    @interconnection.dispatch(from, responseMessage)
  end
end

class LookUpNodeIdResponseMessage
  attr_reader :listOfClosestNodes
  def initialize(listOfClosestNodes=[])
    @listOfClosestNodes = listOfClosestNodes
  end
end

class LookUpNodeIdResponseMessageHandler
  def initialize(interconnection, nodeRepository, requestedNodes)
    @interconnection = interconnection
    @nodeRepository = nodeRepository
    @requestedNodes = requestedNodes
  end
  # handle LookUpNodeIdResponseMessage
  def handleFrom(message, from)
    fromNode = @nodeRepository.getNodeWithNetworkAddress(from)
    return if fromNode.nil?
    message.listOfClosestNodes.each { |node|
      @nodeRepository.insertIfNotExists(node)
    }
    @requestedNodes.synchronize {
      @requestedNodes.add(fromNode.nodeId)
    }
  end
end
