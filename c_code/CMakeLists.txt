cmake_minimum_required (VERSION 2.8.2)
project (DYCOLOA CXX)
SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fPIC -pedantic -O3 -fopenmp -mtune=native")

INCLUDE_DIRECTORIES("./include" "./dbscan")
#INCLUDE_DIRECTORIES(./lib)
FILE(GLOB COMMON_SOURCE_FILES "source/*.cpp")
FILE(GLOB WALKSCAN_SOURCE_FILES "source/walkscan/*.cpp")
FILE(GLOB BENCHMARKS_SOURCE_FILES "source/benchmarks/*.cpp")

find_package(Boost REQUIRED COMPONENTS program_options) # python)

if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    message(STATUS "Using boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Using boost library dirs: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Using boost libraries: ${Boost_LIBRARIES}")
    message(STATUS "Using boost thread library: ${Boost_THREAD_LIBRARY}")
    link_directories(${Boost_LIBRARY_DIRS})
endif (Boost_FOUND)

add_executable(walkscan ${WALKSCAN_SOURCE_FILES} ${COMMON_SOURCE_FILES} "./dbscan/dbscan.cpp")
add_executable(benchmarks ${BENCHMARKS_SOURCE_FILES} ${COMMON_SOURCE_FILES} "./dbscan/dbscan.cpp")

#FILE(GLOB_RECURSE DBSCAN ./lib/*.a)
#TARGET_LINK_LIBRARIES(walkscan ${DBSCAN})
