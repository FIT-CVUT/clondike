#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/measurment_lib.sh

# $1 is number of used nodes
function make_measure_with {
  node=$1
  echo "START measuring with $node nodes"
  $PATH_PREFIX/run_parallel_script.sh $node $node tvrdik $PATH_PREFIX/scripts/start.sh &
  bootstrap_line="`$PATH_PREFIX/run_parallel_script.sh $node $node tvrdik $PATH_PREFIX/scripts/pick_data.sh | grep -i Bootstrap`"
  while [ -z "$bootstrap_line" ]; do
    sleep 1
    printf "."
    bootstrap_line="`$PATH_PREFIX/run_parallel_script.sh $node $node tvrdik $PATH_PREFIX/scripts/pick_data.sh | grep -i Bootstrap`"
  done
  echo
  echo "At node $node ended the bootstrap process"
  echo "bootstrapline=$bootstrap_line"
  echo "$bootstrap_line" >> $MEASURMENT_DIR/$node
  echo
}

if [ $# -ne 1  ]; then
    echo "Usage:"
    echo "  $ bash $0 <ssh_password>"
else
  export PATH_PREFIX="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
  export PASSWORD="$1"
  export NODES_NUM=7
  export MEASURMENT_DIR="$PATH_PREFIX/measurment_results"

  mkdir -p $MEASURMENT_DIR
  stop_all_nodes
  clear_all_nodes

  # start local node
  /root/clondike/scripts/clondike-init &
  sleep 2

  for((node=2; node<=$NODES_NUM; node++)); do
    make_measure_with $node
  done

  stop_all_nodes
  echo
  echo "---- THE END OF MEASURING ---"
  echo
  echo "tar -cf archive.tar measurment_results/"
fi
