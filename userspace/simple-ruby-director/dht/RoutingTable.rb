#
# Implements Kademlia DHT Routing
# Based on a paper:
#   Kademlia: A Peer-to-Peer Information System Based on the XOR Metric, Petar Maymounkov and David Mazieresi, 2002
#

require 'dht/BucketManager'

class RoutingTable
  def initialize(currentNode, bootstrapNodes = [])
    @currentNode = currentNode
    @bootstrapNodes = bootstrapNodes
    startNode
    join
  end

  def getMorePeers(count=1)
  end

  def lookUp(nodeId)
  end

  # on each receive message will update routing table
  def update(node)
  end

  private
  def startNode
  end

  def join
  end

  def keepAliveThread
  end
end
