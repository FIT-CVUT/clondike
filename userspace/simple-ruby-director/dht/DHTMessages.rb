require 'thread'
require 'Node'

class LookUpNodeIdRequestMessage
  attr_reader :lookUpNodeId
  attr_reader :countClosestNodes
  attr_reader :nodeId    # for learning new node
  attr_reader :publicKey # for learning new node
  def initialize(lookUpNodeId, countClosestNodes, nodeId, publicKey)
    @lookUpNodeId = lookUpNodeId
    @countClosestNodes = countClosestNodes
    @nodeId = nodeId
    @publicKey = publicKey
  end
end

class LookUpNodeIdRequestMessageHandler
  def initialize(interconnection, nodeRepository, trustManagement)
    @interconnection = interconnection
    @nodeRepository = nodeRepository
    @trustManagement = trustManagement
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
    responseMessage = LookUpNodeIdResponseMessage.new(closestNodesWithoutSelfNodeClass, @nodeRepository.selfNode.nodeId, @trustManagement.localIdentity.publicKey)
    @interconnection.dispatch(from, responseMessage)

    # We try add the requesting node to our routing table
    requestingNode = Node.new(message.nodeId, from)
    @nodeRepository.insertIfNotExists(requestingNode)
    @trustManagement.registerKey(message.nodeId, message.publicKey) if @trustManagement.getKey(message.nodeId).nil?
  end
end

class LookUpNodeIdResponseMessage
  attr_reader :listOfClosestNodes
  attr_reader :nodeId    # for learning new node
  attr_reader :publicKey # for learning new node
  def initialize(listOfClosestNodes, nodeId, publicKey)
    @listOfClosestNodes = listOfClosestNodes
    @nodeId = nodeId
    @publicKey = publicKey
  end
end

class LookUpNodeIdResponseMessageHandler
  def initialize(interconnection, nodeRepository, requestedNodes, bootstrap, trustManagement)
    @interconnection = interconnection
    @nodeRepository = nodeRepository
    @requestedNodes = requestedNodes
    @bootstrap = bootstrap
    @bootstrapMessageWasMet = false
    @bootstrapMessageWasMetSemaphore = Mutex.new
    @trustManagement = trustManagement
  end
  # handle LookUpNodeIdResponseMessage
  def handleFrom(message, from)
    fromNode = @nodeRepository.getNodeWithNetworkAddress(from)
    if fromNode.nil?
      $log.warn "LookUpNodeIdResponseMessageHandler: We received a lookup nodeId message response from unknown node!"
      return 
    end

    closestNodes = ""
    message.listOfClosestNodes.each { |node|
      @nodeRepository.insertIfNotExists(node)
      closestNodes << ", " unless closestNodes.empty?
      closestNodes << "#{node}"
    }
    $log.debug "DHT: From the node #{from} I have learned about nodes: #{closestNodes}"

    @requestedNodes.synchronize {
      @requestedNodes.add(fromNode.nodeId)
    }

    # The block of code bootstrapMessageWasMetSemaphore.. could be removed
    @bootstrapMessageWasMetSemaphore.synchronize {
      if @bootstrapMessageWasMet == false && @bootstrap.kClosestNodesWereRequested
        @bootstrapMessageWasMet = true
        $log.debug "Bootstrap process successful completed!"
        @nodeRepository.printListOfAllNodes
      end
    }
    # The end of block of code bootstrapMessageWasMetSemaphore

    respondingNode = Node.new(message.nodeId, from)
    @nodeRepository.insertIfNotExists(respondingNode)
    @trustManagement.registerKey(message.nodeId, message.publicKey) if @trustManagement.getKey(message.nodeId).nil?
  end
end
