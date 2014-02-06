#!/bin/bash
source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

# Stop the old one Ruby Director
pkill ruby 2>&1> /dev/null

# Soft disconnecting
find /clondike/*/nodes/[0-9]*/stop -exec bash -c "echo 1 > {} " \; 2>&1> /dev/null

# Hard disconnecting
pkill -9 ruby 2>&1> /dev/null
find /clondike/*/nodes/[0-9]*/kill -exec bash -c "echo 1 > {} " \; 2>&1> /dev/null

# stop migration processes
pkill make 2>&1> /dev/null
pkill cc1 2>&1> /dev/null
