cmake_minimum_required(VERSION 3.10)

project(MoodyWorkspace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MOODY_LOGGR_ENABLE_LOGGING "Enable Moody Loggr logging macros" ON)

add_subdirectory(moody)

add_executable(loggr_test
    test.cpp
)

target_link_libraries(loggr_test PRIVATE
    moody::loggr
)