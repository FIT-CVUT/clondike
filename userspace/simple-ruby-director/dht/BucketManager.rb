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

  # the node with the exact nodeId is NOT include in the returned array of nodes
  def getClosestNodesTo(nodeId, count = DHTConfig::K)
    closestNodes = []
    startIndexBucket = getBucketIndexFor(nodeId)
    minimalDistance = 0
    while closestNodes.length < count do
      index = getClosestBucketIndexFor(nodeId, minimalDistance)
      break if index.nil?
      sortedBucketNodesList = @buckets[index].nodes.sort { |x,y|
        x.getDistanceTo(nodeId) <=> y.getDistanceTo(nodeId)
      }
      sortedBucketNodesList.each { |bucketNode|
        closestNodes.push(bucketNode) if (closestNodes.length < count) && (bucketNode.nodeId != nodeId)
      }
      minimalDistance = getDifferenceDistancesWithIndexes(startIndexBucket, index)+1
    end
    closestNodes
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

  # TODO: Too long
  def getClosestBucketIndexFor(nodeId, minimalDistance = 0)
    startIndex = getBucketIndexFor(nodeId)
    if minimalDistance == 0
      return startIndex
    end

    toLower = {:index => startIndex, :distance => 0}
    toHigher = {:index => startIndex, :distance => 0}

    while toLower[:distance] < minimalDistance && toHigher[:distance] < minimalDistance
      if toLower[:index] > 0 && toHigher[:index] < (@buckets.length-1)
        if getDifferenceDistancesWithIndexes(startIndex, toLower[:index]-1) < getDifferenceDistancesWithIndexes(startIndex, toHigher[:index]+1)
          toLower[:index] -= 1
          toLower[:distance] = getDifferenceDistancesWithIndexes(startIndex, toLower[:index])
        else
          toHigher[:index] += 1
          toHigher[:distance] = getDifferenceDistancesWithIndexes(startIndex, toHigher[:index])
        end
      elsif toLower[:index] > 0
        toLower[:index] -= 1
        toLower[:distance] = getDifferenceDistancesWithIndexes(startIndex, toLower[:index])
      elsif toHigher[:index] < (@buckets.length-1)
        toHigher[:index] += 1
        toHigher[:distance] = getDifferenceDistancesWithIndexes(startIndex, toHigher[:index])
      else
        # We searched all buckets
        return nil
      end
    end
    if toLower[:distance] > minimalDistance
      return toLower[:index]
    end
    return toHigher[:index]
  end

  def getBucketIndexFor(nodeId)
    checkValiditiyNodeId(nodeId)
    # TODO: instead this should be smart map function
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
end
