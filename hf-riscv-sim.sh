#!/usr/bin/env bash
make clean      -C ./tp1
make ${1:-scheduler} -C ./tp1
./tools/sim/hf_riscv_sim/hf_riscv_sim ./tp1/code.bin
