require 'test/unit'

require 'dht/Config'
require 'dht/BucketManager'
require 'Node'
require 'NetworkAddress'
require 'set'

class BucketManagerTest < Test::Unit::TestCase
  def setup
    @networkAddress = NetworkAddress.new("127.0.0.1", "12345")
    @biggestNodeId = DHTConfig::SIZE_NODE_ID_SPACE-1
    @nodeWithBiggestNodeId = Node.new(@biggestNodeId.to_s(16),@networkAddress)
    @nodeWithSmallestNodeId = Node.new("0",@networkAddress)
  end

  def getRandomNode
    Node.new(rand(@biggestNodeId).to_s(16), @networkAddress)
  end

  def test_extensiveTestRandomData
    @selfNode = getRandomNode
    @nodes = Set.new
    @nodes.add(@selfNode)
    @bucketManager = BucketManager.new(@selfNode)
    for i in 0..1000
      newNode = getRandomNode
      @nodes.add(newNode)
      @bucketManager.insertOrReplaceNode(newNode)
    end

    @nodes.each { |node|
      assert_equal(node, @bucketManager.getNode(node.nodeId))
    }
  end

  def test_getNode
    @selfNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@selfNode)
    for nodeId in 1..(DHTConfig::K)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.insertOrReplaceNode(newNode)
    end
    assert_equal(@selfNode, @bucketManager.getNode(@selfNode.nodeId))
    assert_equal(nil, @bucketManager.getNode(@nodeWithBiggestNodeId.nodeId))
  end

  def test_getAllNodes
    @selfNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@selfNode)
    nodeList = [@selfNode]
    for nodeId in 1..(DHTConfig::K)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.insertOrReplaceNode(newNode)
      nodeList.push(newNode)
    end

    allNodes = @bucketManager.getAllNodes
    nodeList.each { |node|
      assert allNodes.include?(node)
    }
  end

  def test_getClosestNodesTo_SmallestNodeId
    @selfNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@selfNode)
    assert_equal([], @bucketManager.getClosestNodesTo(@selfNode.nodeId))
    assert_equal([@selfNode], @bucketManager.getClosestNodesTo(@nodeWithBiggestNodeId.nodeId))

    nodeList = []
    for nodeId in 1..(DHTConfig::K)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.insertOrReplaceNode(newNode)
      nodeList.unshift(newNode)
    end

    assert_equal(nodeList, @bucketManager.getClosestNodesTo(@nodeWithBiggestNodeId.nodeId))
  end

  def test_getClosestNodesTo_BiggestNodeId
    @selfNode = @nodeWithBiggestNodeId
    @bucketManager = BucketManager.new(@selfNode)
    assert_equal([], @bucketManager.getClosestNodesTo(@selfNode.nodeId))
    assert_equal([@selfNode], @bucketManager.getClosestNodesTo(@nodeWithSmallestNodeId.nodeId))

    nodeList = []
    for nodeId in (@biggestNodeId-DHTConfig::K)..(@biggestNodeId-1)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.insertOrReplaceNode(newNode)
      nodeList.push(newNode)
    end

    assert_equal(nodeList, @bucketManager.getClosestNodesTo(@nodeWithSmallestNodeId.nodeId))
  end

  def test_updateLastHeartBeatTime
    @selfNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@selfNode)
    timestamp = @selfNode.lastHeartBeatTime
    sleep(1)
    @bucketManager.updateLastHeartBeatTime(@selfNode.nodeId)
    assert_not_equal(timestamp, @bucketManager.getNode(@selfNode.nodeId).lastHeartBeatTime)
  end

  def test_TryInsertNonValidNodes
    @nonValidNode1 = Node.new((-1).to_s(16),@networkAddress)
    @nonValidNode2 = Node.new((@biggestNodeId+1).to_s(16),@networkAddress)

    assert_raise RuntimeError do
      @bucketManager = BucketManager.new(@nonValidNode1)
    end

    assert_raise RuntimeError do
      @bucketManager = BucketManager.new(@nonValidNode2)
    end
  end
end
