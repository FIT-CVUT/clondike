#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Usage: $0 name-of-interface"	
	exit 1
fi

ifconfig $1  | awk '{print $2}' | sed -n '2p' | cut -d ":" -f2