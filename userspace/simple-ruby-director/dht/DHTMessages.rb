require 'thread'

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
      if node == @nodeRepository.selfNode
        closestNodesWithoutSelfNodeClass.push(Node.new(node.nodeId, node.networkAddress))
      else
        closestNodesWithoutSelfNodeClass.push(node)
      end
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
  def initialize(interconnection, nodeRepository, requestedNodes, bootstrap)
    @interconnection = interconnection
    @nodeRepository = nodeRepository
    @requestedNodes = requestedNodes
    @bootstrap = bootstrap
    @bootstrapMessageWasMet = false
    @bootstrapMessageWasMetSemaphore = Mutex.new
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
    @bootstrapMessageWasMetSemaphore.synchronize {
      if @bootstrapMessageWasMet == false && @bootstrap.kClosestNodesWereRequested
        @bootstrapMessageWasMet = true
        $log.debug "Bootstrap: Successful Completed! SendedMsgs #{@interconnection.messageDispatcher.countSendedMsgs} RecvedMsgs #{@interconnection.messageDispatcher.countRecvedMsgs}"
        @nodeRepository.getAllNodes.each { |node|
          $log.debug "  #{node}"
        }
      end
    }
  end
end
