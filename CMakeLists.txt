cmake_minimum_required(VERSION 3.27)
project(UnixRemoteShellServer C)

set(CMAKE_C_STANDARD 11)

add_executable(UnixRemoteShellServer main.c
        remote_login.h
        remote_login.c
        HandleClientShellSession.c
        HandleClientShellSession.h
        ServerHelperFunctions.c
        ServerHelperFunctions.h)
