#!/bin/ruby

#
# The simulator of Clondike network DHT bootstrap
# It counts the count of messages
#

require 'set'
require 'pp'
require 'openssl'

if ARGV.length != 3
  puts "Usage:"
  puts "  $ ruby simulator.rb <K> <NUMBER_OF_NODES> <NUMBER_OF_PERIODS>"
  puts ""
  puts "Example:"
  puts "  $ ruby simulator.rb 8 1024 10"
  puts ""
  exit 1
end

K = ARGV[0].to_i
NODES_NUM = ARGV[1].to_i
PERIODS = ARGV[2].to_i

class Node
  attr_accessor :node_id
  attr_accessor :ip
  attr_accessor :known_nodes # indexes to array $nodes
  attr_accessor :sended_messages
  attr_accessor :recved_messages

  def initialize(ip_address, node_id = nil)
    @node_id = node_id
    @node_id = rand(2**160) if @node_id.nil?
    #@node_id = OpenSSL::Digest::SHA1.new(OpenSSL::PKey::RSA.new(1024).public_key.to_s).to_s.hex # +- similary like as in Clondike
    @ip = ip_address # index to array $nodes
    @known_nodes = Set.new
    @known_nodes.add(@ip) # self
    @sended_messages = 0
    @recved_messages = 0
  end

  def self.show_nodes
    $nodes.each { |node|
      print "#{node.ip}: "
      node.known_nodes.each { |idx_known_node|
        print "#{idx_known_node} "
      }
      puts ""
    }
  end

  def bootstrap(node_ip)
    @sended_messages += 1; $nodes[node_ip].recved_messages += 1 # PublicKeyDissemination self, sendMeYours
    $nodes[node_ip].sended_messages += 1; @recved_messages += 1 # PublicKeyDissemination node_ip
    @known_nodes.add node_ip
    $nodes[node_ip].known_nodes.add @ip

    requested_nodes = Set.new

    loop {
      k_closest_nodes_idx = get_k_closest_known_nodes(@node_id)
      k_closest_nodes_idx.each { |closest_node_idx|
        next if requested_nodes.include? closest_node_idx
        next if closest_node_idx == @ip

        #print "#{@ip+1} --> #{closest_node_idx+1} : "
        #$nodes[closest_node_idx].get_k_closest_known_nodes(@node_id).each { |closest| print "#{closest+1} " }
        #puts ""

        requested_nodes.add closest_node_idx
        @known_nodes.merge $nodes[closest_node_idx].get_k_closest_known_nodes(@node_id)
        @sended_messages += 1; $nodes[closest_node_idx].recved_messages += 1 # LookUpRequest
        $nodes[closest_node_idx].sended_messages += 1; @recved_messages += 1 # LookUpResponse

        $nodes[closest_node_idx].known_nodes.add @ip # the requested node learns me
      }
      k_closest_contacted_node = 0
      max_k_closest_contacted_node = 0
      get_k_closest_known_nodes(@node_id).each { |idx|
        k_closest_contacted_node += 1 if requested_nodes.include? idx
        max_k_closest_contacted_node += 1
      }
      break if k_closest_contacted_node == max_k_closest_contacted_node
    }
    #print "#{@ip+1} bootstrapped and knows "
    #@known_nodes.each { |node| print "#{node+1} " }
    #puts ""
    #$nodes.each_index { |idx|
    #  puts "  #{$nodes[idx].ip+1}: #{$nodes[idx].node_id^@node_id}"
    #}
    #puts "#{@ip+1} #{@sended_messages} #{@recved_messages}"
    #puts ""
    $messages[@ip] += @sended_messages
  end

  def get_k_closest_known_nodes(to_node_id)
    k_closest_known_nodes = []
    for i in 1..K
      closest = get_closest_known_node(to_node_id, k_closest_known_nodes)
      k_closest_known_nodes.push closest unless closest.nil?
    end
    return k_closest_known_nodes.dup
  end

  private

  def get_closest_known_node(to_node_id, k_closest_known_nodes)
    closest = nil
    @known_nodes.each { |node_ip|
      if k_closest_known_nodes.include? node_ip
        # skip this node
      elsif to_node_id == $nodes[node_ip].node_id
        # skip this node
      elsif closest.nil?
        closest = node_ip
      elsif (to_node_id^$nodes[node_ip].node_id < to_node_id^$nodes[closest].node_id)
        closest = node_ip
      end
    }
    return closest
  end
end

$messages = []
for i in 0..(NODES_NUM-1)
  $messages[i] = 0
end

$nodes = []

# Info about progress
$progress = 0
$period = 0
begin
  Thread.new {
    loop {
      sleep(1);
      progress_bar = (100*$progress)/(NODES_NUM-1)
      $stderr << "\rPeriod #{$period}: #{progress_bar} % completed  "
    }
  }
rescue => err
  $stderr << "Error in thread: #{err.message} \n#{err.backtrace.join("\n")}"
end

# The Computation
for period in 1..PERIODS
  $period = period
  $nodes[0] = Node.new(0)
  for i in 1..(NODES_NUM-1)
    $nodes[i] = Node.new(i)
    $nodes[i].bootstrap(rand(i)) # join on a random node in the cluster
    # $nodes[i].bootstrap(i-1)   # join on a predecessor this node in the cluster

    $progress = i
  end
  $nodes.clear
end

# Print result
for i in 1..(NODES_NUM-1)
  puts "#{i+1} #{$messages[i].to_f/PERIODS.to_f}"
end

