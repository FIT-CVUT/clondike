#!/bin/bash

function stop_all_nodes {
  printf "stopping all nodes..."
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PATH_PREFIX/scripts/ruby_stop.sh &
  is_ruby_running="asdasdsad"
  while [ "$is_ruby_running" != "" ]; do
    sleep 1
    printf "a "
    is_ruby_running="`echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PATH_PREFIX/scripts/is_director_running.sh`"
    echo "is_ruby_running='$is_ruby_running'"
  done
  echo "END of stop waiting"
}

# $1 is number of used nodes
function make_measure_with {
  node=$1
  echo "START measuring with $node nodes"
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/start.sh &
  bootstrap_line=""
  while [ "`echo \"$bootstrap_line\" | wc -l`" != "$node" ]; do
    sleep 1
    printf "b "
    bootstrap_line="`echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/pick_data.sh | grep -i Bootstrap`"
    echo "bootstrap_line=$bootstrap_line"
  done
  echo
  echo "At node $node ended the bootstrap process"
  echo "bootstrapline=$bootstrap_line"
  echo "$bootstrap_line" >> $MEASURMENT_DIR/$node
  echo "" >> $MEASURMENT_DIR/$node
  echo

  stop_all_nodes
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/clear.sh &
  director_cat="I am messy"
  while [ "$director_cat" != "" ]; do
    sleep 1
    printf "c "
    director_cat="`echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $node $PATH_PREFIX/scripts/cat_director.log.sh`"
    echo "director_cat='$director_cat'"
  done
}

if [ $# -ne 1  ]; then
    echo "Usage:"
    echo "  $ bash $0 <ssh_password>"
else
  export PATH_PREFIX="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
  export PASSWORD="$1"
  export NODES_NUM=23
  export MEASURMENT_DIR="$PATH_PREFIX/measurment_results"

  mkdir -p $MEASURMENT_DIR
  stop_all_nodes
  echo $PASSWORD | $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PATH_PREFIX/scripts/clear.sh &

  for((node=2; node<=$NODES_NUM; node++)); do
    make_measure_with $node
  done

  stop_all_nodes
  echo
  echo "---- THE END OF MEASURING ---"
  echo
  echo "tar -cf archive.tar measurment_results/"
fi
