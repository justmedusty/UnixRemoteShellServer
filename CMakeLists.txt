cmake_minimum_required(VERSION 3.27)
project(UnixRemoteShellServer C)

set(CMAKE_C_STANDARD 11)

add_executable(UnixRemoteShellServer main.c
        remote_login.h
        remote_login.c
        handle_remote_shell.c
        handle_remote_shell.h
        server_helper_functions.c
        server_helper_functions.h)
