#!/bin/bash

$PROXY_USER=
$PROXY_PASSWORD=

PROXY="192.168.88.1:3128"
export HTTP_PROXY="http://${PROXY_USER}:${PROXY_PASSWORD}@${PROXY}"
export http_proxy="http://${PROXY_USER}:${PROXY_PASSWORD}@${PROXY}"
export HTTPS_PROXY="https://${PROXY_USER}:${PROXY_PASSWORD}@${PROXY}"
export https_proxy="https://${PROXY_USER}:${PROXY_PASSWORD}@${PROXY}"

dhclient eth0

cd /root/clondike
git fetch origin
git reset --hard origin/master
/root/clondike/scripts/restart-director.sh
