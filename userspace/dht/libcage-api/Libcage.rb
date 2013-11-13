require File.dirname(__FILE__) + '/LibcageModule'

class Libcage
	include LibcageModule

	def initialize(localPort)
		@libcageNode = LibcageModule.libcage_open localPort
		@localPort = localPort
	end

	def connect(destHostname, destPort)
		LibcageModule.libcage_join @libcageNode, destHostname, destPort
	end

	# Try to connect to global world Clondike cluster via hardcoded address, DNS
	def join
		#TODO
	end

	def print_state
		LibcageModule.libcage_print_state @libcageNode
	end

	# Find randomly another unknown new peer (node) in cluster
	def find_new_peer
		#TODO
	end

	def dispatch
		LibcageModule.libcage_dispatch
	end
end
