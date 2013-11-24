# Clondike #

Clondike stands for **CL**uster **O**f **N**on-**D**edicated **I**nteroperating **KE**rnels.

## History of the project ##
The initial goal of the Clondike project was to design a new
type of cluster of Linux machines. **It should be capable of utilizing
standard Linux machines as its computational units, while still
maintaining the illusion of a powerful single system image (SSI)**.
The unique feature of the Clondike system is its ability to integrate
the workstations even if they are not fully dedicated to the system,
they could still be used and administered by their users/admins and
offer their **computing power to the cluster only when they become
idle**.

## State of the art ##

Currently, the Clondike system is being extended into a **full peer-to-peer
operating system level cluster**. In the new version, each
participating user can form his own virtual cluster and use the
other workstations’ computing power for his own calculations. The
P2P architecture of the system makes the system fault-tolerant,
since there is **no single point of failure**, all machines are fully
autonomous. In addition, the new architecture is more attractive for
users, because now they do not only contribute computing power of
their machines, but they can also use computing power of the other
Clondike machines for their own computations.

Since every computer in the Clondike system forms its own administrative
domain, the Clondike system goes beyond boundaries
of traditional clustering systems. It is **similar to the architecture of
a P2P-based grid system**.

## How to install ##

Full details about the build process can be found in the [Clondike install instructions](https://github.com/FIT-CVUT/clondike/blob/master/doc/install_manual_clondike-en.desc).

Soon we will have here an automatic generated run-able image of the Clondike for your easy tryout.

