cmake_minimum_required(VERSION 3.28)

project(iap)

add_executable(iap iap.c ip.c proc.c)

add_custom_target(test
  COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cmake
  DEPENDS iap
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
