# This file is part of ugvec, a program for analysing and comparing vectors
#
# Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 2.6)
project(ugvec)

set(sources	ugvec_main.cpp)

option(ParallelLoadSpeedup "Build with speedup for parallel input vectors" OFF)
message(STATUS "      ParallelLoadSpeedup: " ${ParallelLoadSpeedup} " (options are: ON, OFF)")
if (${ParallelLoadSpeedup})
	add_definitions(-DPARALLEL_LOAD_SPEEDUP)
endif()	

add_executable(ugvec ${sources})
install(TARGETS ugvec RUNTIME DESTINATION "bin")
