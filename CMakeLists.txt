cmake_minimum_required(VERSION 3.7)
project(lps_sdr)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(lps_sdr ${SOURCE_FILES})
target_link_libraries(lps_sdr -lfftw3f)