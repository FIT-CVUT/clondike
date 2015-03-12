# CLASS IN PROGRESS, Not implemented YET :( Meant to be replacement of all networking, and should be able to handle both local network and as well wide area networks
class Network

  def initialize(port=nil, ip_address=nil)
  end

  def messageReceived()
  end

  # Schedules message to be sent
  # TO: Either null (broadcast) or a nodeId (publicKey) of the target node
  # MESSAGE: Message object to be sent
  # DELIVERY OPTIONS: Parameters of this request (retransmissions, etc..)
  # DELIVERY CALLBACK: Callback method to call when the message was either delivered, or send, or considered undeliverable
  def sendMessageTo(to, message, deliveryOptions = nil, deliveryCallback = nil)
    #
    # We want this method as non-blocking for better scalability
    #
    # The API from C++ Library will be blocking,
    # becouse the extending asynchronous callbacks are hellishly difficult.
    # http://burgestrand.se/articles/asynchronous-callbacks-in-ruby-c-extensions.html
    #
    # TODO: Create new thread with call the C++ API
    #
    $log.debug "Network.rb: Send to #{to} the memessage #{message}"
  end

  # Locates required count of nodes
  def findNodes(nodeCriteria, requiredCount)
  end

  # Tries to connect remote node. Optionally notifies about connection progress via callback method
  def connect(node, connectionCallback = nil)
  end
end
