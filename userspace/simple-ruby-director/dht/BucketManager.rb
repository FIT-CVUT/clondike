#
# Please, care about unit tests if you modify this file
# From simple-ruby-director run:
#   $ ruby -I . test/dht/BucketManagerTest.rb
#
# Implements Kademlia DHT Routing Table
# Based on a paper:
#   Kademlia: A Peer-to-Peer Information System Based on the XOR Metric, Petar Maymounkov and David Mazieresi, 2002
#
require 'dht/Config'
require 'dht/Bucket'

class BucketManager
  def initialize(selfNode)
    checkValiditiyNodeId(selfNode.nodeId)
    @selfNode = selfNode
    bucket = Bucket.new
    bucket.insertOrReplaceNode(@selfNode)
    @buckets = []
    @buckets.push(bucket)
  end

  def getNode(nodeId)
    bucketIndex = getBucketIndexFor(nodeId)
    @buckets[bucketIndex].nodes.each { |node|
      return node if node.nodeId == nodeId
    }
    return nil
  end

  def insertOrReplaceNode(node)
    bucketIndex = getBucketIndexFor(node.nodeId)
    if @buckets[bucketIndex].contain?(@selfNode.nodeId) && @buckets[bucketIndex].filled? && (@buckets[bucketIndex].contain?(node.nodeId) == false)
      splitBucketWithIndex(bucketIndex)
      insertOrReplaceNode(node) # Recursive
    else
      @buckets[bucketIndex].insertOrReplaceNode(node)
    end
  end

  def getAllNodes
    allNodes=[]
    @buckets.each { |bucket|
      allNodes.push(bucket.nodes)
    }
    return allNodes.flatten
  end

  def getClosestNodesTo(nodeId, count = DHTConfig::K)
    getKClosestKnownNodes(nodeId, getAllNodes, count)
  end

  def purgeNode(nodeId)
    bucketIndex = getBucketIndexFor(nodeId)
    @buckets[bucketIndex].purgeNode(nodeId)
  end

  def updateLastHeartBeatTime(nodeId)
    bucketIndex = getBucketIndexFor(nodeId)
    @buckets[bucketIndex].updateLastHeartBeatTime(nodeId)
  end

  def updateInfo(nodeId, nodeInfo)
    bucketIndex = getBucketIndexFor(nodeId)
    @buckets[bucketIndex].updateInfo(nodeId, nodeInfo)
  end

  def updateState(nodeId, nodeState)
    bucketIndex = getBucketIndexFor(nodeId)
    @buckets[bucketIndex].updateState(nodeId, nodeState)
  end

  private

  def checkValiditiyNodeId(nodeId)
    if nodeId.nil?
      raise "NodeId cannot be nil"
      return false
    end
    if nodeId.hex < 0 || nodeId.hex > DHTConfig::SIZE_NODE_ID_SPACE-1
      raise "Non-valid NodeId"
      return false
    end
    return true
  end

  def getDifferenceDistances(distance1, distance2)
    distances = [distance1, distance2]
    return (distances.max - distances.min)
  end

  def getDifferenceDistancesWithIndexes(indexFirstBucket, indexSecondBucket)
    diffDistFrom = getDifferenceDistances(@buckets[indexFirstBucket].range[:from], @buckets[indexSecondBucket].range[:from])
    diffDistTo = getDifferenceDistances(@buckets[indexFirstBucket].range[:to], @buckets[indexSecondBucket].range[:to])
    return(diffDistFrom+diffDistTo)
  end

  def getBucketIndexFor(nodeId)
    checkValiditiyNodeId(nodeId)
    # TODO: instead the below should be a smart map function
    @buckets.each_index { |bucketIndex|
      if @buckets[bucketIndex].hasKeySpaceOverlapWith(nodeId)
        return bucketIndex
      end
    }
    raise "Did not find bucket for nodeId ", nodeId
  end

  def splitBucketWithIndex(bucketIndex)
    bucketLower, bucketHigher = @buckets[bucketIndex].split
    if @buckets.delete_at(bucketIndex).nil?
      raise "We did not delete the correct bucket!"
    end
    @buckets.insert(bucketIndex, bucketHigher)
    @buckets.insert(bucketIndex, bucketLower)
  end

  def getKClosestKnownNodes(to_node_id, known_nodes, count)
    k_closest_known_nodes = []
    for i in 1..count
      closest = getClosestKnownNode(to_node_id, k_closest_known_nodes, known_nodes)
      k_closest_known_nodes.push closest unless closest.nil?
    end
    return k_closest_known_nodes
  end

  def getClosestKnownNode(to_node_id, k_closest_known_nodes, known_nodes)
    closest = nil
    known_nodes.each { |node|
      if k_closest_known_nodes.include? node
        # skip this node
      elsif to_node_id == node.nodeId
        # skip this node
      elsif closest.nil?
        closest = node
      elsif ((to_node_id.hex)^(node.nodeId.hex) < (to_node_id.hex)^(closest.nodeId.hex))
        closest = node
      end
    }
    return closest
  end

end
