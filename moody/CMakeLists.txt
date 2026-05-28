cmake_minimum_required(VERSION 3.10)

project(moody_loggr
    VERSION 0.3.0
    LANGUAGES CXX
)

add_library(moody_loggr
    src/Loggr.cpp
)

add_library(moody::loggr ALIAS moody_loggr)

target_include_directories(moody_loggr
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(moody_loggr
    PUBLIC
        cxx_std_17
)

target_compile_definitions(moody_loggr
    PUBLIC
        MOODY_LOGGR_ENABLE_LOGGING=$<BOOL:${MOODY_LOGGR_ENABLE_LOGGING}>
)