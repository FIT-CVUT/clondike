#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

pkill ruby
sleep 1
$CLONDIKE_SCRIPTS/clondike-init &
