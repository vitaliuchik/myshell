cmake_minimum_required(VERSION 3.15)
project(myshell)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Readline REQUIRED)
include_directories(${Readline_INCLUDE_DIRS})

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options filesystem REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

include_directories(inc)

add_executable(myshell main.cpp)
add_executable(mycat src/cat.cpp)

add_dependencies(myshell mycat)

add_library(options SHARED src/options.cpp inc/options/options.h)
add_library(builtIn SHARED inc/builtIn/builtIn.h src/builtIn.cpp)
add_library(ioposix SHARED src/ioposix.cpp inc/ioposix/ioposix.h)

target_link_libraries(myshell ${Readline_LIBRARY} builtIn ${Boost_LIBRARIES})
target_link_libraries(mycat ioposix options)