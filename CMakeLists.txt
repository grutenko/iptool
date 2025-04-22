cmake_minimum_required(VERSION 3.28)

project(iap)

add_executable(iap iap.c ip.c ip-parse.c ip-range.c inflate.c invert.c filter.c)