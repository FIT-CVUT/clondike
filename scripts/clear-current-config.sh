#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/configuration.sh

rm -rf $CLONDIKE_SIMPLE_RUBY_DIRECTOR/conf/*
echo "This directory should be empty before starting Clondike" > $CLONDIKE_SIMPLE_RUBY_DIRECTOR/conf/README
