require File.dirname(__FILE__) + '/LibcageModule'

class Libcage
	include LibcageModule

	def initialize(localPort)
		@libcageNode = LibcageModule.libcage_open localPort
		@localPort = localPort
	end

	def join(destHostname, destPort)
		LibcageModule.libcage_join @libcageNode, destHostname, destPort
	end

	def print_state
		LibcageModule.libcage_print_state @libcageNode
	end

	def dispatch
		LibcageModule.libcage_dispatch
	end
end
