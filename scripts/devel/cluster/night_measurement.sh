#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage:"
    echo "  $ bash $0 <ssh_password> <fray1_password>"
else
  export SSH_PASSWORD="$1"
  export FRAY1_PASSWORD="$2"

  cd /root/clondike/scripts/devel/cluster/
  rm -rf measurment_results/
  ./run_parallel_script.sh 1 24 $SSH_PASSWORD set_next_bootstrap_node.sh
  sleep 10

  for((i=0;i<1000;i++)); do
    time timeout -k 130 120 /root/clondike/scripts/devel/cluster/measurment_incremental.sh $SSH_PASSWORD
    cd /root/clondike/scripts/devel/cluster/
    tar -cf archive-$i.tar measurment_results/
    sshpass -p$FRAY1_PASSWORD scp archive-$i.tar tvrdipa1@fray1.fit.cvut.cz:/home/stud/tvrdipa1/archive-$i.tar
    echo ""
    echo "----- $i -----"
    echo ""
  done

  echo "END OF NIGHT MEASURE"
  cd /root/clondike/scripts/devel/cluster/
  ./run_parallel_script.sh 1 24 $SSH_PASSWORD scripts/poweroff.sh
fi
