#!/bin/bash

find /clondike/*/nodes/[0-9]/kill -exec bash -c "echo 1 > {} " \;
