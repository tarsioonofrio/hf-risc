cmake_minimum_required(VERSION 3.10)
project (scheduler)
#Bring the headers into the project
#set(ROOT ./tp1)

#Bring the headers into the project
include_directories(tp1/x86/include)
#However, the file(GLOB...) allows for wildcard additions:
#file(GLOB SOURCES  "${ROOT}/x86/lib/*.c")
#Can manually add the sources using the set command as follows:
set(SOURCES tp1/app/scheduler.c tp1/x86/include/list.h tp1/x86/lib/list.c)# tp1/x86/include/rt.h tp1/x86/lib/rt.c)
#set(SOURCES tp1/app/scheduler.c tp1/x86/lib/list.c tp1/lib/rt.c)
add_executable(scheduler ${SOURCES})