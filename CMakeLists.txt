cmake_minimum_required(VERSION 3.7.2)
project(BSPA23 C)

set(CMAKE_C_STANDARD 99)

add_executable(BSPA23 main.c main.h keyValStore.c keyValStore.h sub.c sub.h socket.c socket.h semaphore.c semaphore.h)
