# The storage for contact any node
class NetworkAddress
	attr_reader :ipAddress
	attr_reader :port

	def initialize(ipAddress, port)
		@ipAddress = ipAddress
		@port = port
	end
end
