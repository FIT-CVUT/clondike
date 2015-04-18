#!/bin/bash


spustitV="XXX_CAS_SPUSTENI_XXX"

adresar="/root/clondike/mereni/"
souborSVysledkem="$adresar/mereni_vysledek.txt"

while [[ `date +"%s"` -ge $spustitV ]] ; do
   sleep 1
done

echo "Start"
mpstat 1 > "$souborSVysledkem" &







