#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

$CLONDIKE_SCRIPTS/stop.sh

$CLONDIKE_SCRIPTS/clondike-init &
