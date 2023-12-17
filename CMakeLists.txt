cmake_minimum_required(VERSION 3.26)
project(miio_cpp_rewrite)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
find_package(RapidJSON)

add_library(miio_cpp_rewrite SHARED
        Device.cpp
        Device.h
        aes.c
        aes.h
        md5.cpp
        md5.h
        socket_class.cpp
        socket_class.h
        type_tools.cpp
        type_tools.h
        )

add_executable(test test.cpp
        Device.cpp
        Device.h
        aes.c
        aes.h
        md5.cpp
        md5.h
        socket_class.cpp
        socket_class.h
        type_tools.cpp
        type_tools.h
)
