DETACHED_MANAGER_SLOT = 0
CORE_MANAGER_SLOT = 1

class ManagerSlot
  attr_reader :slotIndex
  attr_reader :slotType

  def initialize(slotType, slotIndex)
    @slotIndex = slotIndex
    @slotType = slotType
  end

  def to_s
    if slotType == DETACHED_MANAGER_SLOT
      "DETACHET_MANAGER_SLOT/Index:#{slotIndex}"
    elsif slotType == CORE_MANAGER_SLOT
      "CORE_MANAGER_SLOT/Index:#{slotIndex}"
    else
      "Manager.rb:ManagerSlot:to_s: ERROR Impossible happened!"
    end
  end
end

class DetachedNodeManager
  # List of all nodes which I am connected to
  # Associated core node information (i.e. the core node on the "other end")
  # Holds node objects
  attr_accessor :coreNode
  # Slot, in which resides connection to the associated core node
  attr_reader :coreNodeSlotIndex

  def initialize(coreNode, coreNodeSlotIndex)
    @coreNode = coreNode
    @coreNodeSlotIndex = coreNodeSlotIndex
  end
end

class CoreNodeManager
  # List of all nodes which are connected to me
  # All associated detached nodes. They should be placed to their associated slots in the array
  # Holds Node objects
  attr_reader :detachedNodes

  attr_reader :connectedNodesCount

  def initialize
    @detachedNodes = []
    @connectedNodesCount = 0
  end

  def registerDetachedNode (slotIndex, node)
    # TODO: We log is as error instead of raising error, because this is actually happening ;( The most likely reason seems to be broken NetLink
    # communicaton, when some messages get lost (namely preceding node disconnected message in this case)
    if @detachedNodes[slotIndex] != nil
      #raise(ArgumentError,"Node already registered at index #{slotIndex}: Node id: #{@detachedNodes[slotIndex].nodeId}") if @detachedNodes[slotIndex] != nil
      $log.error "Core Node already registered at index #{slotIndex}: Node id: #{@detachedNodes[slotIndex].nodeId}"
    end
    @detachedNodes[slotIndex] = node
    @connectedNodesCount = @connectedNodesCount + 1
  end

  def unregisterDetachedNode (slotIndex)
    raise(ArgumentError, "No node registered at index #{slotIndex}") if @detachedNodes[slotIndex] == nil
    @detachedNodes[slotIndex] = nil
    @connectedNodesCount = @connectedNodesCount - 1
  end

  def nodeConnected(address, slotIndex)
    $log.error "Manager.rb:nodeConnected NOT IMPLEMENTED"
  end

  def containsNode(otherNode)
    result = false
    @detachedNodes.each { |node|
      if ( node.nodeId == otherNode.nodeId )
        result = true
        break
      end
    }

    return result
  end
end
