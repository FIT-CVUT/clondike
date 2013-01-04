#!/bin/bash
# skript vztvarejici adresarovou strukturu pro Clondike

mkdir -p /clondike/ccn/mig
cd /clondike/ccn
mkdir nodes
touch listen stop-listen-one stop-listen-all stop listening-on
cd mig
touch migrate-home emigrate-ppm-p migproc
mkdir -p /clondike/pen/mig
cd /clondike/pen
mkdir nodes
touch listen stop-listen-one stop-listen-all stop listening-on
cd mig
touch migrate-home emigrate-ppm-p migproc

mkdir -p /mnt/local/proc /mnt/proxy /mnt/clondike
