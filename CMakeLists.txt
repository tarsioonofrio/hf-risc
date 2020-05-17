cmake_minimum_required(VERSION 3.10)
project (scheduler)
#set(ROOT ./tp1)
#Bring the headers into the project
include_directories(tp1/local)
#However, the file(GLOB...) allows for wildcard additions:
#file(GLOB SOURCES  "${ROOT}/local/lib/*.c")
#Can manually add the sources using the set command as follows:
set(SOURCES tp1/app/scheduler.c tp1/local/list.c)# tp1/local/include/rt.h tp1/local/lib/rt.c)
add_executable(scheduler ${SOURCES})