#!/bin/bash

#
# Drop all your local changes and commits,
# fetch the latest history from the server
# and point your local master branch at it
#

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh
cd $CLONDIKE_HOME
git fetch origin
git reset --hard origin/master
