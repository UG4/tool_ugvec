################################################
# created by Sebastian Reiter
# s.b.reiter@gmail.com
################################################

cmake_minimum_required(VERSION 2.6)
project(vecutil)

set(sources	vecutil_main.cpp)

option(ParallelLoadSpeedup "Build with speedup for parallel input vectors" OFF)
message(STATUS "ParallelLoadSpeedup: " ${ParallelLoadSpeedup} " (options are: ON, OFF)")
if (${ParallelLoadSpeedup} STREQUAL "ON")
	add_definitions(-DPARALLEL_LOAD_SPEEDUP)
endif()	

add_executable(vecutil ${sources})
install(TARGETS vecutil RUNTIME DESTINATION "bin")
