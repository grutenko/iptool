cmake_minimum_required(VERSION 3.28)

project(iap LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(iap src/iap.c src/core.c src/cmd.c src/arg.c)
target_include_directories(iap PRIVATE include)

add_custom_target(test
  COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cmake
  DEPENDS iap
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
