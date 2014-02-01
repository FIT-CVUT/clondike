require 'NetworkAddress'

require 'pp'
require 'Util'

class DHTMessageDispatcher
  MAXLEN_RECEIVE_MESSAGE = 60000

  def initialize(localNetworkAddress)
    @port = localNetworkAddress.port
    @ipAddress = localNetworkAddress.ip

    @recievingSocket = UDPSocket.new
    @recievingSocket.bind("", @port)

    @sendingSocket = UDPSocket.new
    @sendingSocket.connect(@ipAddress, @port)

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
        $log.warn "DHT_Dispatcher:#{__LINE__} #{__method__}: We have not yet registered NodeRepository. #{message.inspect}, #{to.inspect}"
        return
      end
      @nodeRepository.getAllNodes.each { |node|
        next if node.nodeId == @nodeRepository.selfNode.nodeId
        $log.debug "SEND TO #{node}\n#{message}"
        #showBacktrace
        #@sendingSocket.send(Marshal.dump(message), 0, node.networkAddress.ip, node.networkAddress.port)
        @sendingSocket.send(Marshal.dump(message), 0, node.networkAddress.ip, @port) # FIXME: hardcoded port
      }
    elsif
      contact = to
      if to.class != NetworkAddress
        if @nodeRepository.nil?
          $log.warn "DHT_Dispatcher:#{__LINE__} #{__method__}: We have not yet registered NodeRepository. #{message.inspect}, #{to.inspect}"
          return
        end
        contact = @nodeRepository.getContactToNode(to)
        if contact.nil?
          $log.warn "DHT_Dispatcher:#{__LINE__} #{__method__}: We have not yet in NodeRepository the node with address #{to}, message #{message}."
          return
        end
      elsif @nodeRepository.nil? != true
        dispatchToNode = @nodeRepository.getNodeWithNetworkAddress(contact)
        if dispatchToNode.nil? != true && dispatchToNode.nodeId == @nodeRepository.selfNode.nodeId
          $log.warn "#DHT_Dispatcher:#{__LINE__} #{__method__}: Ignore message that should be send to 'this' node (selfNode)"
          return
        end
      end

      $log.debug "SEND TO #{contact}\n#{message}"
      #showBacktrace
      #@sendingSocket.send(Marshal.dump(message), 0, contact.ip, contact.port)
      @sendingSocket.send(Marshal.dump(message), 0, contact.ip, @port) # FIXME: hardcoded port
    end
  end

  def receive
    # Loop while we get a message from remote IP (has to ignore local messages)
    while (true) do
      recvData, addr = @recievingSocket.recvfrom(MAXLEN_RECEIVE_MESSAGE)
      break if ( !isLocalIP(addr.last))
    end
    message = Marshal.load(recvData)
    from = NetworkAddress.new(addr.last, addr[1])
    $log.debug("RECEIVE FROM #{from}\n#{message}")

    [message, from]
  end

  private

  # Detects whether a passed IP is local
  def isLocalIP(ipAddress)
    return @ipAddress == ipAddress
  end
end

