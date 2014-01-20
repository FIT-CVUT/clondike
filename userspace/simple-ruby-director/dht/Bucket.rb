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

  def hasKeySpaceOverlapWith?(node)
    node.nodeId.hex >= @range[:from] && node.nodeId.hex <= @range[:to]
  end

  def contain?(node)
    @nodes.each { |bucketNode|
      return true if bucketNode == node
    }
    return false
  end

  def update(node)
    if hasKeySpaceOverlapWith?(node) == false
      return
    end

    updated = false
    updatedNodeIndex = 0
    @nodes.each_index { |bucketNodeIndex|
      if @nodes[bucketNodeIndex] == node
        @nodes[bucketNodeIndex].updateLastHeartBeatTime()
        updated = true
        updatedNodeIndex = bucketNodeIndex
      end
    }
    if updated == true
      # Move an actualized node to the end of a list
      updatedNode = @nodes.delete_at(updatedNodeIndex)
      @nodes.push(updatedNode)
    end
    if updated == false && filled? == false
      @nodes.push(node)
    end
  end

  def filled?
    @nodes.length >= DHTConfig::K
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
      if bucketLower.hasKeySpaceOverlapWith?(bucketNode)
        bucketLower.update(bucketNode)
      elsif bucketHigher.hasKeySpaceOverlapWith?(bucketNode)
        bucketHigher.update(bucketNode)
      else
        raise "Imposible happened" # TODO: Improve raise
      end
    }

    return bucketLower, bucketHigher
  end
end
