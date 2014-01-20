require 'test/unit'

require 'dht/Config'
require 'dht/BucketManager'
require 'Node'
require 'NetworkAddress'

class BucketManagerTest < Test::Unit::TestCase
  def setup
    @networkAddress = NetworkAddress.new("127.0.0.1", "12345")
    @biggestNodeId = DHTConfig::SIZE_NODE_ID_SPACE-1
    @nodeWithBiggestNodeId = Node.new(@biggestNodeId.to_s(16),@networkAddress)
    @nodeWithSmallestNodeId = Node.new("0",@networkAddress)
    puts ""
  end
  def test_getClosestNodesTo_SmallestNodeId
    @currentNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@currentNode)
    assert_equal([@currentNode], @bucketManager.getClosestNodesTo(@currentNode))
    assert_equal([@currentNode], @bucketManager.getClosestNodesTo(@nodeWithBiggestNodeId))

    nodeList = []
    for nodeId in 1..(DHTConfig::K)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.update(newNode)
      nodeList.unshift(newNode)
    end

    assert_equal(nodeList, @bucketManager.getClosestNodesTo(@nodeWithBiggestNodeId))
  end
  def test_getClosestNodesTo_BiggestNodeId
    @currentNode = @nodeWithBiggestNodeId
    @bucketManager = BucketManager.new(@currentNode)
    assert_equal([@currentNode], @bucketManager.getClosestNodesTo(@currentNode))
    assert_equal([@currentNode], @bucketManager.getClosestNodesTo(@nodeWithSmallestNodeId))

    nodeList = []
    for nodeId in (@biggestNodeId-DHTConfig::K)..(@biggestNodeId-1)
      newNode = Node.new(nodeId.to_s(16), @networkAddress)
      @bucketManager.update(newNode)
      nodeList.push(newNode)
    end

    assert_equal(nodeList, @bucketManager.getClosestNodesTo(@nodeWithSmallestNodeId))
  end
  def test_update
    @currentNode = @nodeWithSmallestNodeId
    @bucketManager = BucketManager.new(@currentNode)
    timestamp = @currentNode.lastHeartBeatTime
    sleep(1)
    @bucketManager.update(@currentNode)
    assert_not_equal(timestamp, @bucketManager.getClosestNodesTo(@currentNode)[0].lastHeartBeatTime)
  end
  def test_TryInsertNonValidNode
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
