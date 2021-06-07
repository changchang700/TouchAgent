cmake_minimum_required(VERSION 3.0.0)
project(TouchAgent VERSION 1.0.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")

include_directories("include")

find_library(z-lib z)
find_library(log-lib log)

add_library(local_hv STATIC IMPORTED)
set_target_properties(local_hv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libhv.a)

add_subdirectory(src)
add_subdirectory(tests)
