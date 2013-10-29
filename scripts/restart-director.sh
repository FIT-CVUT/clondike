#!/bin/sh

pkill ruby
./clear-current-config.sh
sleep 1
./clondike-init &
