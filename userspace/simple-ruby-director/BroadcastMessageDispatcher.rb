require 'NetworkAddress'

class BroadcastMessageDispatcher
  MAXLEN_RECEIVE_MESSAGE = 60000

  def initialize(localNetworkAddress)
    @port = localNetworkAddress.port
    @ip = localNetworkAddress.ip

    @recvSocket = UDPSocket.new
    @recvSocket.bind("", @port)

    @sendSocket = UDPSocket.new
    @sendSocket.connect(@ip, @port)
    @sendSocket.setsockopt(Socket::SOL_SOCKET, Socket::SO_BROADCAST, 1)

    @nodeRepository = nil

    @countSendedMsgs = 0
    @countRecvedMsgs = 0
  end

  def registerNodeRepository(nodeRepository)
    @nodeRepository = nodeRepository
  end

  # Schedules message to be sent
  # TO:
  #   One of:
  #     a nil for send to broadcast
  #     a nodeId of the target node or
  #     a NetworkAddress of the target node
  # MESSAGE: Message object to be sent
  def dispatch(to, message)
    if to.nil?
      $log.debug "SEND #{@countSendedMsgs+=1}# TO BROADCAST\n#{message}"
      @sendSocket.send(Marshal.dump(message), 0, "255.255.255.255", @port)
    elsif
      contact = to
      if to.class != NetworkAddress
        if @nodeRepository.nil?
          $log.warn "BroadcastMessageDispatcher:#{__LINE__} #{__method__}: We have not yet registered NodeRepository. #{message.inspect}, #{to.inspect}"
          return
        end
        contact = @nodeRepository.getContactToNode(to)
        if contact.nil?
          $log.warn "BroadcastMessageDispatcher:#{__LINE__} #{__method__}: We have not yet in NodeRepository the node with address #{to}, message #{message}."
          return
        end
      elsif @nodeRepository.nil? != true
        dispatchToNode = @nodeRepository.getNodeWithNetworkAddress(contact)
        if dispatchToNode.nil? != true && dispatchToNode.nodeId == @nodeRepository.selfNode.nodeId
          $log.warn "#DHTMessegeDispatcher:#{__LINE__} #{__method__}: Ignore message that should be send to 'this' node (selfNode)"
          return
        end
      end

      $log.debug "SEND #{@countSendedMsgs+=1}# TO #{contact}\n#{message}"
      @sendSocket.send(Marshal.dump(message), 0, contact.ip, contact.port)
    end
  end

  def receive
    while (true) do
      recvData, addr = @recvSocket.recvfrom(MAXLEN_RECEIVE_MESSAGE)
      break if addr.last != @ip
    end
    message = Marshal.load(recvData)
    from = NetworkAddress.new(addr.last, addr[1])
    $log.debug("RECEIVE #{@countRecvedMsgs+=1}# FROM #{from}\n#{message}")
    from = NetworkAddress.new(addr.last, @port) # TODO: remove this hack of port number

    [message, from]
  end
end

