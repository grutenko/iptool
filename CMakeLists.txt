cmake_minimum_required(VERSION 3.28)

project(iap)

add_executable(iap src/iap.c src/ip.c src/proc.c)
target_include_directories(iap PRIVATE include)