cmake_minimum_required(VERSION 2.8.9)
project (tasker)
#Can manually add the sources using the set command as follows:
set(SOURCES  tp1/app/main.c)
add_executable(tasker ${SOURCES})