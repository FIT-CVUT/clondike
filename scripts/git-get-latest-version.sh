#!/bin/bash

#
# Drop all your local changes and commits,
# fetch the latest history from the server
# and point your local master branch at it
#

git fetch origin
git reset --hard origin/master
