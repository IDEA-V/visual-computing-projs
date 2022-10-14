cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

project(datraw)

add_library(datraw INTERFACE)
target_include_directories(datraw INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/datraw>)
