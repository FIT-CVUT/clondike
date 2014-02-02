#
# Please, care about unit tests if you modify this file
# From simple-ruby-director run:
#   $ ruby -I . test/dht/BucketManagerTest.rb
#

require 'dht/Config'

class Bucket
  attr_reader :nodes
  attr_reader :range
  def initialize (
    range = {
      :from => 0,
      :to   => DHTConfig::SIZE_NODE_ID_SPACE-1,
    })
    @nodes = []
    @range = range
  end

  def insertOrReplaceNode(node)
    updatedNodeIndex = nil
    @nodes.each_index { |nodeIndex|
      if @nodes[nodeIndex] == node
        updatedNodeIndex = nodeIndex
        @nodes[nodeIndex] = node.dup
      end
    }
    moveNodeAtTheEnd(updatedNodeIndex) if updatedNodeIndex.nil? != true
    if updatedNodeIndex.nil? && filled? == false
      @nodes.push(node)
    end
  end

  def contain?(nodeId)
    @nodes.each { |bucketNode|
      return true if bucketNode.nodeId == nodeId
    }
    return false
  end

  def filled?
    @nodes.length >= DHTConfig::K
  end

  def hasKeySpaceOverlapWith(nodeId)
    nodeId.hex >= @range[:from] && nodeId.hex <= @range[:to]
  end

  def split
    bucketLower = Bucket.new({
      :from => @range[:from],
      :to   => @range[:from]+(@range[:to]-@range[:from])/2,
    })
    bucketHigher = Bucket.new({
      :from   => @range[:from]+(@range[:to]-@range[:from])/2+1,
      :to   => @range[:to],
    })

    @nodes.each { |bucketNode|
      if bucketLower.hasKeySpaceOverlapWith(bucketNode.nodeId)
        bucketLower.insertOrReplaceNode(bucketNode)
      elsif bucketHigher.hasKeySpaceOverlapWith(bucketNode.nodeId)
        bucketHigher.insertOrReplaceNode(bucketNode)
      else
        raise "Impossible happened, needs fix the implementation of DHT Buckets"
      end
    }

    return bucketLower, bucketHigher
  end

  def updateLastHeartBeatTime(nodeId)
    updatedNodeIndex = nil
    @nodes.each_index { |nodeIndex|
      if @nodes[nodeIndex].nodeId == nodeId
        @nodes[nodeIndex].updateLastHeartBeatTime()
        updatedNodeIndex = nodeIndex
      end
    }
    return if updatedNodeIndex.nil?
    moveNodeAtTheEnd(updatedNodeIndex)
  end

  def updateInfo(nodeId, nodeInfo)
    @nodes.each { |node|
      if node.nodeId == nodeId
        node.updateInfo(nodeInfo)
        return
      end
    }
  end

  def updateState(nodeId, nodeState)
    @nodes.each { |node|
      if node.nodeId == nodeId
        node.updateState(nodeState)
        return
      end
    }
  end

  private

  # Move an actualized node to the end of a list of nodes
  def moveNodeAtTheEnd(updatedNodeIndex)
    updatedNode = @nodes.delete_at(updatedNodeIndex)
    @nodes.push(updatedNode)
  end
end
