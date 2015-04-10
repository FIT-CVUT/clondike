require 'NetworkAddress'
require 'trust/Identity'

class DHTMessageDispatcher
  MAXLEN_RECEIVE_MESSAGE = 60000

  def initialize(localNetworkAddress)
    @port = localNetworkAddress.port
    @ipAddress = localNetworkAddress.ip

    @socket = UDPSocket.new
    @socket.bind(@ipAddress, @port)

    @nodeRepository = nil
  end

  def registerNodeRepository(nodeRepository)
    @nodeRepository = nodeRepository
  end

  # Schedules message to be sent
  # TO:
  #   One of:
  #     a nil for send to all known nodes or
  #     a nodeId of the target node or
  #     a NetworkAddress of the target node
  # MESSAGE: Message object to be sent
  def dispatch(to, message)
    if to.nil?
      if @nodeRepository.nil?
        $log.warn "DHTDispatcher:#{__LINE__} #{__method__}: We have not yet registered NodeRepository. #{message.inspect}, #{to.inspect}"
        return
      end
      @nodeRepository.getAllNodes.each { |node|
        next if node.nodeId == @nodeRepository.selfNode.nodeId
        $log.debug "SEND TO #{node}\n#{message}"
        @socket.send(Marshal.dump(message), 0, node.networkAddress.ip, node.networkAddress.port)
      }
    elsif
      contact = to
      if to.class != NetworkAddress
        if @nodeRepository.nil?
          $log.warn "DHTMessageDispatcher:#{__LINE__} #{__method__}: We have not yet registered NodeRepository. #{message.inspect}, #{to.inspect}"
          return
        end
        contact = @nodeRepository.getContactToNode(to)
        if contact.nil?
          $log.warn "DHTMessageDispatcher:#{__LINE__} #{__method__}: We have not yet in NodeRepository the node with address #{to}, message #{message}."
          return
        end
      elsif @nodeRepository.nil? != true
        dispatchToNode = @nodeRepository.getNodeWithNetworkAddress(contact)
        if dispatchToNode.nil? != true && dispatchToNode.nodeId == @nodeRepository.selfNode.nodeId
          $log.warn "#DHTMessegeDispatcher:#{__LINE__} #{__method__}: Ignore message that should be send to 'this' node (selfNode)"
          return
        end
      end

      $log.debug("SEND TO #{contact}\n#{message}")
    
      @socket.send(Marshal.dump(message), 0, contact.ip, contact.port)
    end
  end

  def receive
    recvData, addr = @socket.recvfrom(MAXLEN_RECEIVE_MESSAGE)
    message = Marshal.load(recvData)
    from = NetworkAddress.new(addr.last, addr[1])
    $log.debug("RECEIVE FROM #{from}\n#{message}")

    [message, from]
  end
end
