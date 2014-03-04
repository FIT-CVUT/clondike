#!/bin/bash

function clear_all_nodes {
  $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PASSWORD $PATH_PREFIX/scripts/clear.sh &
  director_cat="I am messy"
  while [ "$director_cat" != "" ]; do
    sleep 1
    printf "clearingAllNodes "
    director_cat="`$PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PASSWORD $PATH_PREFIX/scripts/cat_director.log.sh`"
    echo "director_cat='$director_cat'"
  done
}

function stop_all_nodes {
  printf "stopping all nodes..."
  $PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PASSWORD $PATH_PREFIX/scripts/ruby_stop.sh &
  is_ruby_running="asdasdsad"
  while [ "$is_ruby_running" != "" ]; do
    sleep 1
    printf "stoppingAllNodes "
    is_ruby_running="`$PATH_PREFIX/run_parallel_script.sh 1 $NODES_NUM $PASSWORD $PATH_PREFIX/scripts/is_director_running.sh`"
    echo "is_ruby_running='$is_ruby_running'"
  done
  echo "END of stop waiting"
}

