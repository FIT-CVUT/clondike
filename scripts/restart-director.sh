#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

# Stop the old one Director ruby
pkill ruby

# Hard disconnecting
find /clondike/*/nodes/[0-9]*/kill -exec bash -c "echo 1 > {} " \;

sleep 1
$CLONDIKE_SCRIPTS/clondike-init &
