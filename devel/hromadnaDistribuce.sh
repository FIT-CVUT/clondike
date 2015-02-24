#!/bin/bash
# hromadna distribuce pro bouraci ucebnu pro projekt Clondike
# autor: Vojtech Myslivec, FIT CVUT
#  gnu/gpl licence s laskou udelena Jirkovi Rakosnikovi a Zdenkovi Novemu
#

passSoubor=./pass
interface=eth1


ip4byteOd=100
ip4byteDo=215
ip4byteKrok=5
pocetPingu=1

[ $# -eq 0 ] && {
   echo "USAGE:
   $0 file...

   Zkopiruje file na cilove stroje se stejnou cestou"
   exit 1
}



[ -x "$passSoubor" ] || {
   echo "soubor s heslem '$passSoubor' neni spustitelny nebo neexistuje"
   exit 2
}

adrMoje=`ip addr show "$interface" | sed -n 's/.*\<inet\> *\([0-9.]*\).*/\1/p'`
adrPrefix="${adrMoje%.*}"

[ -n "$adrMoje" ] || {
   echo "problem s adresou, adresa na '$interface' je '$adrMoje'"
   exit 3
}


export DISPLAY=:0
export SSH_ASKPASS="$passSoubor"

citacNO=0
citacOK=0
citacFAIL=0

for ((i=$ip4byteOd ; i<=$ip4byteDo ; i+=$ip4byteKrok )); do 
   adr=${adrPrefix}.$i

   echo -n "$adr: "
   [ "$adr" == "$adrMoje" ] && {
      echo "preskakuji (moje adresa)"
      continue;
   }


   if ping -c "$pocetPingu" "$adr" > /dev/null 2>&1; then 
      if setsid scp -o StrictHostKeyChecking=no "$@" "$adr:$PWD"; then
         echo "ok!"
         ((citacOK++))
      else
         echo "FAIL!"
         ((citacFAIL++))
      fi 
   else
      echo "bez odpovedi"
      ((citacNO++))
   fi

done


echo "Hotovo
   uspesnych	$citacOK
   neuspesnych	$citacFAIL
   bez odpovedi	$citacNO"

