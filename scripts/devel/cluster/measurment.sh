#!/bin/bash

function stop_all_nodes {
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PATH_PREFIX/scripts/stop.sh &
  echo "START waiting for stopping all nodes"
  sleep 30
  echo "END   waiting for stopping all nodes"
}

# $1 is number of used nodes
function make_measure_with {
  node=$1
  echo "START measuring with $node nodes"
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/start.sh &
  sleep 60
  echo "END   measuring with $node nodes"
  echo "START pick data"
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/pick_data.sh | grep -i Bootstrap >> $MEASURMENT_DIR/$node &
  sleep 20
  echo "END   pick data"
  stop_all_nodes
  echo "" >> $MEASURMENT_DIR/$node
}

if [ $# -ne 1  ]; then
    echo "Usage:"
    echo "  $ bash $0 <ssh_password>"
else
  export PATH_PREFIX="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
  export PASSWORD="$1"
  export NODES_NUM=24
  export MEASURMENT_DIR="$PATH_PREFIX/measurment_results"

  mkdir -p $MEASURMENT_DIR
  stop_all_nodes

  for((node=2; node<=$NODES_NUM; node++)); do
    make_measure_with $node
  done

  echo
  echo "---- THE END OF MEASURING ---"
  echo
  echo "tar -cf archive.tar measurment_results/"
fi
