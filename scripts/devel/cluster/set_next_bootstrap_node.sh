#!/bin/bash

NODE=`ifconfig eth0.91  | awk '{print $2}' | sed -n '2p' | cut -d ":" -f2 | cut -d "." -f4`
NEXT_NODE=$((NODE-1))
if [ $NEXT_NODE -eq 0 ]; then
  NEXT_NODE=1
fi
IP_NEXT_NODE="192.168.1.$NEXT_NODE"

echo "$IP_NEXT_NODE:54321" > /root/clondike/userspace/simple-ruby-director/BootstrapList.txt
