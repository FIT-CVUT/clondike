# Maybe instead this we should use a standard ruby class Addrinfo
# The crate for contact node
class NetworkAddress
  attr_reader :ip
  attr_reader :port
  attr_reader :protocol

  def initialize(ip='127.0.0.1', port=54321, protocol='tcp')
    @ip = ip.to_s
    @port = port.to_i
    @protocol = protocol.to_s
  end

  def to_s
    "#{ip}:#{port}"
  end

  def ==(other)
    other.class == NetworkAddress && @ip == other.ip && @port == other.port
  end
end
