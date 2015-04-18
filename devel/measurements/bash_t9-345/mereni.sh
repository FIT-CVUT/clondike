#!/bin/bash
# Skript pro mereni systemu Clondike v laboratori
#  Fakulty informacnich technologii CVUT v Praze
#  T9:345.
# Skript distribuuje potrebne skripty a spusti nastroj
#  mpstat, ktery je soucasti balicku sysstat.
# 



[[ $# -ne 1 ]] && {
   echo "Usage: $0 <pocet_vlaken>\nNutne mit na vsech pocitacich nainstalovany balicek sysstat kvuli nastroji mpstat"
   exit 0
}

echo "Budu kompilovat s $1 vlakny"

################################
###### Definice promennych #####

adresarSJadrem="/usr/src/linux-2.6.32.5"
adresar="/root/clondike/mereni"

cekaciDoba=120
spustitV=`date +"%s"`
spustitV=$((spustitV+cekaciDoba))

souborKlientaSablona="$adresar/mereni_klient_sablona.sh"
souborKlienta="$adresar/mereni_klient.sh"
souborSVysledkem="$adresar/mereni_vysledek.txt"
souborSVysledkemCas="$adresar/mereni_vysledek_cas.txt"

hromadnaDistribuce="/root/clondike/devel/hromadnaDistribuce.sh"
hromadnyPrikaz="/root/clondike/devel/hromadnyPrikaz.sh"


################################
######### Inicializace #########
echo "Start: $spustitV" > "$souborSVysledkemCas"

################################

################################
########## Hlavni kod ##########

read -p "Start?"

mkdir -p "$adresar"
cd "$adresar"

cp "$souborKlientaSablona" "$souborKlienta"
sed -i "s/XXX_CAS_SPUSTENI_XXX/$spustitV/" "$souborKlienta"

echo "mkdir -p $adresar"
"$hromadnyPrikaz" "mkdir -p $adresar"
echo "Distribuce souboru $souborKlienta"
"$hromadnaDistribuce" "$souborKlienta"
echo "Spusteni skriptu $souborKlienta"
"$hromadnyPrikaz" "$souborKlienta &"


# Cekani na synchronizaci s jadrem
while [[ ! `date +"%s"` -ge $spustitV ]] ; do
   sleep 1
done

# Vlastni kompilace a mereni na lokalnim uzlu
echo "Start"
mpstat 1 > "$souborSVysledkem" &

cd "$adresarSJadrem"
make -j $1
echo "Konec: `date +'%s'`" >> "$souborSVysledkemCas"


################################
############ Uklid #############

rm "$souborKlienta"

echo "pkill mpstat"
"$hromadnyPrikaz" "pkill mpstat"


echo "rm $souborKlienta"
"$hromadnyPrikaz" "rm $souborKlienta"


################################
############ KONEC #############
################################

exit 0



