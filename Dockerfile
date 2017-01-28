#FROM debian:8
FROM python:3.5

RUN apt-get update && apt-get install -y gcc g++ make initramfs-tools bc libssl-dev git ncurses-dev libnl-3-200 libnl-3-dev libnl-genl-3-200 libnl-genl-3-dev libnl-utils ruby-dev ruby git vim-nox pkg-config wget xz-utils valgrind gem && gem install cassandra-driver -v 3.0.0.rc.2 --no-document

ENV PATH_SIMULATOR /root/clondike/kernel_simulator

ENV PATH_USERSPACE /root/clondike/userspace

ADD kernel_simulator $PATH_SIMULATOR

ADD userspace $PATH_USERSPACE

ADD scripts /root/clondike/scripts

ADD root/.migration.conf /root

ADD requirements.txt $PATH_SIMULATOR

RUN echo 'PROMPT_COMMAND="source /root/clondike/scripts/bash_prompt.sh"' >> /root/.bashrc && mkdir /clondike 

WORKDIR $PATH_SIMULATOR

RUN pip install -r requirements.txt

RUN cp -r fake_bin /usr/ && ln -s $PATH_SIMULATOR/clondike /usr/bin/clondike && make && make client && /root/clondike/scripts/build-userspace.sh

WORKDIR /root

ENTRYPOINT ["/root/clondike/scripts/run_clondike.sh"]

CMD ["172.18.0.2", "172.18.0.3:54321"]



