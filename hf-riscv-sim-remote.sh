#!/usr/bin/env bash
rsync -avz -e 'ssh' ./tools user@192.168.15.210:/home/user/tp
rsync -avz -e 'ssh' ./tp1 user@192.168.15.210:/home/user/tp
ssh -tt user@192.168.15.210 << EOF
    PATH=$PATH:/usr/local/mips-elf/gcc/bin:/usr/local/riscv32-unknown-elf/gcc/bin
    make clean      -C /home/user/tp/tp1
    make ${1:-scheduler} -C /home/user/tp/tp1
    /home/user/tp/tools/sim/hf_riscv_sim/hf_riscv_sim /home/user/tp/tp1/code.bin
EOF
