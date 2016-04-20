#!/bin/bash

CASSANDRA_IP=$1
BOOTSTRAP_IP=$2
IS_DAEMON=$3

echo ${CASSANDRA_IP}
echo ${BOOTSTRAP_IP}

sed -i "s/^\(cassandra_hosts='\).*\('.*\)/\1$CASSANDRA_IP\2/" /root/clondike/userspace/simple-ruby-director/clondike.conf
sed -i "s/^\(bootstrap='\).*\('.*\)/\1$BOOTSTRAP_IP\2/" /root/clondike/userspace/simple-ruby-director/clondike.conf

/root/clondike/kernel_simulator/clondike_kernel_simulator > /tmp/simulator.log 2> /tmp/simulator.err &

ruby -w -I /root/clondike/userspace/simple-ruby-director/ /root/clondike/userspace/simple-ruby-director/ClondikeInit.rb > /tmp/director.log 2>&1 &

if [[ $IS_DAEMON -eq 1 ]]; then
  tail -f /tmp/simulator.err
fi

bash
