# Měřící framework v Clondike #

TODO: Translate to english

## Použití ##

Zkontrolujeme případně upravíme soubor ~/.migration.conf

    FULL_PATH_TO_EXEC_PROG - POVOLENE_UZLY_KAM_SE_MUZE_MIGROVAT
    ...

například:

    /usr/lib64/gcc - Any
    /usr/lib/gcc - Any
    /usr/bin/gcc - Any
    /usr/lib/gcc/x86_64-linux-gnu/4.4.4/cc1 - Any
    /usr/lib/gcc/x86_64-linux-gnu/4.4.5/cc1 - Any
    /usr/lib/gcc/x86_64-linux-gnu/4.7/cc1 - Any

Spustíme ruby director s EMIG 1:

    # EMIG=1 /root/clondike/scripts/restart-director.sh

Spustíme clienta pro CliServer (který se pouští v director.rb)

    # cd /root/clondike/userspace/simple-ruby-director
    # ruby1.8 cli/CliClient.rb

Pustíme měření nějakého plánu, například single-kernel:

    Director> measure resultFilename /tmp/singleKernelLog planFilename /root/clondike/userspace/simple-ruby-director/plans/single-kernel

V tomto případě se výstup z naplánovaných úloh zapíše do souborů /tmp/*execCommand*-*PID*

Celkové statistiky o měření se zapíší do souboru /tmp/singleKernelLog

Další log o migraci se zapisuje do /var/log/director/LoadBalancer.log

## Plány ##

Jak vytvářet plány se asi nejrychleji pochopí z ukázkového plánu [sample-lsplan](https://github.com/FIT-CVUT/clondike/blob/master/userspace/simple-ruby-director/plans/sample-lsplan)
