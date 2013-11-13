#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

pkill ruby
$CLONDIKE_SCRIPTS/clear-current-config.sh
sleep 1
$CLONDIKE_SCRIPTS/clondike-init &
