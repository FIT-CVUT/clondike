#!/bin/bash
# skript vytvarejici adresarovou strukturu pro Clondike

mkdir -p /clondike/ccn/mig/migproc
cd /clondike/ccn
mkdir nodes listening-on mounter
touch listen stop-listen-one stop-listen-all stop 
cd mig
touch migrate-home emigrate-ppm-p
mkdir -p /clondike/pen/mig/migproc
cd /clondike/pen
mkdir nodes listening-on mounter
touch listen stop-listen-one stop-listen-all stop connect
cd mig
touch migrate-home emigrate-ppm-p

mkdir -p /mnt/local/proc /mnt/proxy /mnt/clondike
mkdir /root/clondike/userspace/simple-ruby-director/conf
