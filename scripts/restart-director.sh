#!/bin/sh

killall ruby
sleep 1
cd /root/clondike/userspace/simple-ruby-director
nice -n -20 ruby Director.rb > /tmp/director.log 2>&1 &
