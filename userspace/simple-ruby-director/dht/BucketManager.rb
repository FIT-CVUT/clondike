require 'dht/Config'
require 'dht/Bucket'

class BucketManager
  def initialize(currentNode)
    checkValidityNode(currentNode)
    @currentNode = currentNode
    bucket = Bucket.new
    bucket.update(@currentNode)
    @buckets = []
    @buckets.push(bucket)
  end

  def getClosestNodesTo(node, count = DHTConfig::K)
    checkValidityNode(node)
    closestNodes = []
    startIndexBucket = getBucketIndexFor(node)
    minimalDistance = 0
    while closestNodes.length < count do
      index = getClosestBucketIndexFor(node, minimalDistance)
      break if index.nil?
      sortedBucketNodesList = @buckets[index].nodes.sort { |x,y|
        x.getDistanceTo(node) <=> y.getDistanceTo(node)
      }
      sortedBucketNodesList.each { |bucketNode|
        closestNodes.push(bucketNode) if closestNodes.length < count
      }
      minimalDistance = getDifferenceDistancesWithIndexes(startIndexBucket, index)+1
    end
    closestNodes
  end

  def update(node)
    checkValidityNode(node)
    bucketIndex = getBucketIndexFor(node)
    if @buckets[bucketIndex].contain?(@currentNode) && @buckets[bucketIndex].filled? && (@buckets[bucketIndex].contain?(node) == false)
      splitBucketWithIndex(bucketIndex)
      update(node) # Recursive
    else
      @buckets[bucketIndex].update(node)
    end
  end

  private

  def checkValidityNode(node)
    if node.nil?
      raise "Node cannot be nil"
      return false
    end
    if node.nodeId.hex < 0 || node.nodeId.hex > DHTConfig::SIZE_NODE_ID_SPACE-1
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

  def getClosestBucketIndexFor(node, minimalDistance = 0)
    startIndex = getBucketIndexFor(node)
    if minimalDistance == 0
      return startIndex
    end

    toLower = {:index => startIndex, :distance => 0}
    toHigher = {:index => startIndex, :distance => 0}

    actualIndex = startIndex

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

  def getBucketIndexFor(node)
    # TODO: instead this should be smart map function
    @buckets.each_index { |bucketIndex|
      if @buckets[bucketIndex].hasKeySpaceOverlapWith?(node)
        return bucketIndex
      end
    }
    raise "Did not find bucket for node ", node
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
