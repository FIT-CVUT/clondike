#!/usr/bin/env ruby

require File.dirname(__FILE__) + '/../Libcage'

if (ARGV.length != 3 && ARGV.length != 1)
	puts 'Usage: ./example1.rb local_port [hostname dest_port]'
	puts 'Example:'
	puts '  $ ./example1.rb 10000 &'
	puts '  $ ./example1.rb 10001 localhost 10000'
	raise 'Bad input arguments!'
end

node = Libcage.new(ARGV[0].to_i)

if ARGV.length == 3
	node.connect ARGV[1].to_s, ARGV[2].to_i
end

node.print_state

if ARGV.length != 3
	node.dispatch
end
