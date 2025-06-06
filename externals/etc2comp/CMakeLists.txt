# Copyright 2015 Etc2Comp Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0048 NEW)
project(EtcTest VERSION 0.1)

set (CMAKE_CXX_STANDARD 11)
IF (APPLE)
	set (CMAKE_CXX_FLAGS "-I/usr/include/i386-linux-gnu/c++/4.8 -I/usr/include/c++/4.8 -std=c++11 -g3 -Wall -O3")
ELSE ()
	IF (WIN32)
		set (CMAKE_CXX_FLAGS "-I/usr/include/i386-linux-gnu/c++/4.8 -I/usr/include/c++/4.8 -W4 /EHsc")
	ELSE()
		set (CMAKE_CXX_FLAGS "-I/usr/include/i386-linux-gnu/c++/4.8 -I/usr/include/c++/4.8 -std=c++11 -pthread -g3 -Wall -O2")
	ENDIF()
ENDIF ()
ADD_SUBDIRECTORY(EtcLib)
ADD_SUBDIRECTORY(EtcTool)
