# Moody Loggr

MIT License

> **Note**
> Moody Loggr is currently under construction.

---

# Table of Contents

* [Description](#description)
* [Features](#features)
* [Requirements](#requirements)
* [Project Structure](#project-structure)
* [Installation](#installation)

  * [Manual Integration](#manual-integration)
  * [Git Submodule](#git-submodule)
* [Building](#building)
* [Quick Start](#quick-start)
* [Logging Macros](#logging-macros)
* [Compile-Time Logging Guard](#compile-time-logging-guard)
* [File Output Modes](#file-output-modes)
* [Structured Log Format](#structured-log-format)
* [Optional Variables](#optional-variables)
* [Log Levels](#log-levels)
* [CMake Configuration](#cmake-configuration)

  * [Root Workspace CMake](#root-workspace-cmake)
  * [Moody Library CMake](#moody-library-cmake)
* [Supported Features](#supported-features)
* [Planned Extensions](#planned-extensions)
* [Current Status](#current-status)

---

# Description

Moody Loggr is a lightweight asynchronous C++ logging library designed for quick integration into modern C++ projects.

The library focuses on:

* asynchronous logging
* thread safety
* structured formatting
* readable output
* modular logging
* real-time-friendly logging architecture
* simple integration into existing codebases

Log events are pushed into a thread-safe queue and processed independently by an internal worker thread.

This minimizes blocking on producer threads and makes the logger suitable for:

* MIDI systems
* game engines
* audio software
* parsers
* simulations
* modular real-time systems

Example output:

```text
2026-05-28 09:00:06.381 [DEBUG] [PARSER] [Parser.cpp:24] msg:{Handling system real-time byte} <byte=248>
```

---

# Features

## Core Features

* asynchronous producer/consumer logging
* thread-safe queue architecture
* internal worker thread
* file logging
* console logging
* ANSI colored console output
* configurable minimum log level
* compile-time logging guard
* structured readable formatting
* module-aware logging
* per-module log files
* combined global + per-module logs
* millisecond timestamp support
* automatic source location capture
* optional key/value variables
* manual sink flushing
* automatic directory creation
* reusable CMake target
* Doxygen documentation comments

---

# Requirements

* C++17 or newer
* Standard library threading support
* CMake 3.10 or newer

---

# Project Structure

```text
moody-loggr/
├── build/
├── logs/
├── moody/
│   ├── include/
│   │   └── moody/
│   │       └── Loggr.hpp
│   ├── src/
│   │   └── Loggr.cpp
│   └── CMakeLists.txt
├── CMakeLists.txt
├── test.cpp
├── README.md
└── LICENSE
```

---

# Installation

Moody Loggr can be integrated manually or as a Git submodule.

---

## Manual Integration

Copy the following files into your project:

```text
moody/include/moody/Loggr.hpp
moody/src/Loggr.cpp
```

Then include the logger:

```cpp
#include "moody/Loggr.hpp"
```

Make sure `Loggr.cpp` is compiled together with your project.

---

## Git Submodule

Add Moody Loggr as a Git submodule:

```bash
git submodule add <repository-url> external/moody-loggr
git submodule update --init --recursive
```

Add the logger to your project:

```cmake
add_subdirectory(external/moody-loggr/moody)

target_link_libraries(your_target PRIVATE
    moody::loggr
)
```

---

# Building

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Run the test executable from the repository root:

```bash
./build/loggr_test
```

Running from the repository root ensures logs are written to:

```text
logs/
```

instead of:

```text
build/logs/
```

because log paths are resolved relative to the current working directory.

---

# Quick Start

A complete usage example can be found in:

```text
test.cpp
```

Build and run:

```bash
cmake -S . -B build
cmake --build build

./build/loggr_test
```

Example:

```cpp
moody::Loggr logger(
    "logs",
    "music_engine",
    "engine.log",
    true,
    false,
    true,
    true
);

logger.set_level(moody::Loggr::DEBUG);

logger.set_file_mode(
    moody::Loggr::FileMode::SingleFileAndPerModule
);

LOG_INFO(logger, "PARSER", "Parsed MIDI message");

LOG_DEBUG(
    logger,
    "ARRANGER",
    "Generated phrase",
    "notes", 42,
    "velocity", 96
);
```

---

# Logging Macros

Moody Loggr provides:

```cpp
LOG_TRACE(...)
LOG_DEBUG(...)
LOG_INFO(...)
LOG_WARN(...)
LOG_ERROR(...)
LOG_FATAL(...)
```

The macros automatically capture:

```cpp
__FILE__
__LINE__
```

All logging macros are thread-safe.

---

# Compile-Time Logging Guard

Moody Loggr supports compile-time macro disabling through:

```cpp
MOODY_LOGGR_ENABLE_LOGGING
```

When disabled, logging macros compile to:

```cpp
((void)0)
```

This prevents:

* log calls
* formatting
* argument evaluation

Example:

```cpp
LOG_DEBUG(
    logger,
    "PARSER",
    "Parsed byte",
    "byte",
    expensiveFunction()
);
```

When logging is disabled:

```cpp
expensiveFunction()
```

is never executed.

---

## Enable Logging

```bash
cmake -S . -B build -DMOODY_LOGGR_ENABLE_LOGGING=ON
cmake --build build
```

---

## Disable Logging

```bash
cmake -S . -B build -DMOODY_LOGGR_ENABLE_LOGGING=OFF
cmake --build build
```

---

## Header Macro Guard

`Loggr.hpp` uses:

```cpp
#ifndef MOODY_LOGGR_ENABLE_LOGGING
#define MOODY_LOGGR_ENABLE_LOGGING 1
#endif

#if MOODY_LOGGR_ENABLE_LOGGING

// logging macros

#else

// no-op macros

#endif
```

---

## Runtime vs Compile-Time Filtering

### Runtime filtering

```cpp
logger.set_level(moody::Loggr::WARN);
```

Ignores lower-severity messages while keeping logging active.

### Compile-time disabling

```bash
-DMOODY_LOGGR_ENABLE_LOGGING=OFF
```

Completely removes logging macros from the build.

---

# File Output Modes

Moody Loggr supports:

```cpp
logger.set_file_mode(moody::Loggr::FileMode::SingleFile);

logger.set_file_mode(moody::Loggr::FileMode::PerModule);

logger.set_file_mode(
    moody::Loggr::FileMode::SingleFileAndPerModule
);
```

---

## SingleFile

Writes all events to one chronological log:

```text
logs/music_engine/engine.log
```

---

## PerModule

Writes events only to module-specific logs:

```text
logs/music_engine/modules/PARSER.log
logs/music_engine/modules/ANALYZER.log
logs/music_engine/modules/ARRANGER.log
logs/music_engine/modules/ENGINE.log
```

---

## SingleFileAndPerModule

Writes events to both:

* global log
* module logs

Useful for multi-stage pipelines:

```text
MIDI Parser
    ↓
MIDI Analyzer
    ↓
MIDI Arranger
    ↓
MusicEngineCore
```

Recommended for modular systems.

---

# Structured Log Format

Current format:

```text
YYYY-MM-DD HH:MM:SS.mmm [LEVEL] [MODULE] [File.cpp:line] msg:{Message} <key=value>
```

Example:

```text
2026-05-28 09:00:06.381 [DEBUG] [RENDERER] [Renderer.cpp:24] msg:{Render picture} <testVar=234>
```

---

# Optional Variables

Optional variables are passed in key/value pairs:

```cpp
"name", value
```

Example:

```cpp
LOG_DEBUG(
    logger,
    "ARRANGER",
    "Generated note",
    "note", 64,
    "velocity", 127
);
```

Output:

```text
<note=64, velocity=127>
```

Odd argument counts are handled automatically:

```text
<key=<missing>>
```

---

# Log Levels

```text
TRACE
DEBUG
INFO
WARN
ERROR
FATAL
OFF
```

Example:

```cpp
logger.set_level(moody::Loggr::WARN);
```

---

# CMake Configuration

---

## Root Workspace CMake

```cmake
cmake_minimum_required(VERSION 3.10)

project(MoodyWorkspace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MOODY_LOGGR_ENABLE_LOGGING
    "Enable Moody Loggr logging macros"
    ON
)

add_subdirectory(moody)

add_executable(loggr_test
    test.cpp
)

target_link_libraries(loggr_test PRIVATE
    moody::loggr
)
```

---

## Moody Library CMake

```cmake
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
```

---

# Supported Features

* [x] Asynchronous logging
* [x] Thread-safe producer/consumer architecture
* [x] Internal worker thread
* [x] File logging
* [x] Per-module logging
* [x] Combined global + module logging
* [x] ANSI colored console output
* [x] Structured formatting
* [x] Millisecond timestamps
* [x] Automatic source capture
* [x] Compile-time logging guards
* [x] Runtime log filtering
* [x] Doxygen documentation comments
* [x] Reusable CMake target
* [x] Submodule integration support

---

# Planned Extensions

* [ ] JSON output
* [ ] Multiple sinks
* [ ] Rotating file sink
* [ ] Pattern-based formatting
* [ ] Thread-local context propagation
* [ ] Sensitive data filtering
* [ ] Rate limiting
* [ ] Network/syslog/database sinks

---
