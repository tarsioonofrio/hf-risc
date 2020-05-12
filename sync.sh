#!/usr/bin/env bash
rsync -avz -e 'ssh' ./tools user@192.168.15.210:/home/user/tp
rsync -avz -e 'ssh' ./tp1 user@192.168.15.210:/home/user/tp

