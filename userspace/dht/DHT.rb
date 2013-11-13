require File.dirname(__FILE__) + '/libcage-api/Libcage'

class DHT < Libcage
	def initialize(port)
		super port
	end
end
