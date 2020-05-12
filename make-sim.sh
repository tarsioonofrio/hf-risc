#!/usr/bin/env bash
ssh user@192.168.15.210 << EOF
    make clean -C /home/user/tp/tools/sim/hf_riscv_sim
    make -C /home/user/tp/tools/sim/hf_riscv_sim
EOF