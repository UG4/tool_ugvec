################################################
# created by Sebastian Reiter
# s.b.reiter@gmail.com
################################################

cmake_minimum_required(VERSION 2.6)
project(vecutil)

set(sources	vecutil_main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "RELEASE" FORCE)

add_executable(vecutil ${sources})
install(TARGETS vecutil RUNTIME DESTINATION "bin")