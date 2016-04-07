#!/bin/bash

/root/clondike/kernel_simulator/clondike_kernel_simulator > /tmp/simulator.log &

ruby -w -I /root/clondike/userspace/simple-ruby-director/ /root/clondike/userspace/simple-ruby-director/ClondikeInit.rb > /tmp/director.log &
