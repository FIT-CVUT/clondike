#!/bin/bash

#
#  Utility for run parallel some bash script at all subnet nodes
#
#  Prerequisites:
#     sshpass
#     ssh
#

if [ $# -ne 3  ]; then
    echo "Usage:"
    echo "  $ bash $0 <end_of_ip_start_node> <end_of_ip_end_node> <local_script>"
    echo
    echo "For example:"
    echo "  We have local script scripts/update_git_and_restart_clondike.sh and we have nodes with IP:"
    echo "    192.168.1.1"
    echo "    192.168.1.2"
    echo "    192.168.1.3"
    echo "    192.168.1.4"
    echo "    ..."
    echo "    192.168.1.24"
    echo "  We should run:"
    echo "    $ bash $0 1 24 scripts/update_git_and_restart_clondike.sh"
    echo 
    echo "The network address 192.168.1.x is hardcoded"
    echo
else
    NETWORK="192.168.1"
    IP_START_NODE=$1
    IP_END_NODE=$2
    LOCAL_SCRIPT=$3

    echo "Local script that will run remotely: ${LOCAL_SCRIPT}"
    echo "Nodes: ${NETWORK}.${IP_START_NODE} ... ${NETWORK}.${IP_END_NODE}"
    echo

    printf "Please type the password to ssh access to nodes: "
    read pass
    echo
    export PASSWORD="$pass"

    # The argument $1 is node's IP
    function call_script_at_remote_node
    {
        echo "sshpass -p${PASSWORD} ssh -o StrictHostKeyChecking=no root@${NETWORK}.$1 'bash -s' < \"${LOCAL_SCRIPT}\""
        sshpass -p${PASSWORD} ssh -o StrictHostKeyChecking=no root@${NETWORK}.$1 'bash -s' < "${LOCAL_SCRIPT}"
    }

    for((nodeIP=$IP_START_NODE;nodeIP<=$IP_END_NODE;nodeIP++)); do
        call_script_at_remote_node $nodeIP &
    done
fi
