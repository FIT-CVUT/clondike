#!/bin/bash
# skript vztvarejici adresarovou strukturu pro Clondike

mkdir -p /clondike/ccn/mig
cd /clondike/ccn
touch listen stop-listen-one stop-listen-all stop listening-on nodes
cd mig
touch migrate-home emigrate-ppm-p migproc
mkdir -p /clondike/pen/mig
cd /clondike/pen
touch listen stop-listen-one stop-listen-all stop listening-on nodes
cd mig
touch migrate-home emigrate-ppm-p migproc

mkdir -p /mnt/local/proc /mnt/proxy /mnt/clondike
