FROM debian:8

RUN apt-get update && apt-get install -y gcc g++ make initramfs-tools git ncurses-dev libnl-3-200 libnl-3-dev libnl-genl-3-200 libnl-genl-3-dev libnl-utils ruby-dev ruby git vim-nox pkg-config

ENV PATH_SIMULATOR /root/clondike/kernel_simulator

ENV PATH_USERSPACE /root/clondike/userspace

ADD kernel_simulator $PATH_SIMULATOR

ADD userspace $PATH_USERSPACE

ADD scripts /root/clondike/scripts

ADD root/.migration.conf /root

RUN echo 'PROMPT_COMMAND="source /root/clondike/scripts/bash_prompt.sh"' >> /root/.bashrc && mkdir /clondike 

WORKDIR $PATH_SIMULATOR

RUN make && /root/clondike/scripts/build-userspace.sh

CMD ["$PATH_SIMULATOR/clondike_kernel_simulator"]



