#!/bin/bash

MIGRATION_PIPE="/var/run/clondike.pipe"

echo $1 >> ${MIGRATION_PIPE}
