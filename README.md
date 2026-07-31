PPCSIM
======
ppcsim is an instruction set simulator for classic PowerPC processors.
It was licensed by Motorola using the language shown in LICENSE, 
specifically in support of using Motorola (later, Freescale and NXP)
processors.

This incarnation of PPCSIM is surrounded by code allowing it to run
on Linux, as well as more dynamic setup of system architecure.

It emulates instructions well enough to boot DINK, MDINK, SPINIT
(a Sandpoint PMC8240-only boot image) and a few others.

PROCESSORS
==========
* MPC60x:  MPC601, MPC603, MPC603e, MPC603ev, MPC604, MPC604e
* MPC7xx:  MPC750, MPC74, MPC755, MPC745
* MPC7xxx: MPC7400, MPC7410, MPC7450, MPC7447, MPC7448A
* MPC824x: MPC8240, MPC8245

NORTHBRIDGE
===========
* MPC106, MPC107, MPC8240

IO
==
* UART:  PC1655x
* NVRAM: 4K generic
* RTC:   read time (not set)

All other IO, including PCI config cycles, are silently ignored.

History note
============
ppcsim was originally built into DINK. Now, why on Earth would you add
a virtual PowerPC emulator in code that is already running on PowerPC?

The reason was that the RISC Applications group developed hardware and
software that needed to be ready on day 1 of new silicon.  We could
run new DINK images before silicon.  The boards supported "interposers"
to make current silicon look like new silicon, but that's a story for a
different time.
