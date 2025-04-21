cmake_minimum_required(VERSION 3.28)

project(iptool)

add_executable(iptool iptool.c ip.c ip-parse.c invert.c filter.c)