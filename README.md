# Moody Loggr

MIT License

> **Note**
> Moody Loggr is currently under construction.

---

# Description

Moody Loggr is a lightweight C++ logging library designed for quick integration into modern C++ projects.

The goal is to provide a lightweight asynchronous and thread-safe logger focused on:

* readable structured logging
* file + console output
* ANSI colored logs
* real-time-friendly timestamps
* simple integration into existing projects

All logging is performed asynchronously through a single internal worker thread. Producer threads enqueue log events and never write directly to sinks.

The formatter is designed to remain human-readable while also preparing the logger for future structured formats such as JSON.

Example output:

```text
2026-05-28 09:00:06.381 [DEBUG] [PARSER] [Parser.cpp:24] msg:{Handling system real-time byte} <byte=248>
```

---

# Requirements

* C++17 or newer
* Standard library threading support
* CMake 3.10 or newer

---

# Current Project Structure

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

## Option 1: Manual Integration

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

## Option 2: Git Submodule

Add Moody Loggr as a Git submodule:

```bash
git submodule add <repository-url> external/moody-loggr
git submodule update --init --recursive
```

Add the project to your CMake build:

```cmake
add_subdirectory(external/moody-loggr/moody)

target_link_libraries(your_target PRIVATE
    moody::loggr
)
```

---

# Building Moody Loggr

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

# File Output Modes

Moody Loggr supports multiple file output strategies.

```cpp
logger.set_file_mode(moody::Loggr::FileMode::SingleFile);
logger.set_file_mode(moody::Loggr::FileMode::PerModule);
logger.set_file_mode(moody::Loggr::FileMode::SingleFileAndPerModule);
```

## SingleFile

Writes all log events to one chronological file.

```text
logs/music_engine/engine.log
```

This is useful for seeing the full application timeline.

## PerModule

Writes log events only to module-specific files.

```text
logs/music_engine/modules/PARSER.log
logs/music_engine/modules/ANALYZER.log
logs/music_engine/modules/ARRANGER.log
logs/music_engine/modules/ENGINE.log
```

This is useful when debugging one subsystem at a time.

## SingleFileAndPerModule

Writes each log event to both:

```text
logs/music_engine/engine.log
```

and:

```text
logs/music_engine/modules/PARSER.log
logs/music_engine/modules/ANALYZER.log
logs/music_engine/modules/ARRANGER.log
logs/music_engine/modules/ENGINE.log
```

This mode is useful for pipelines such as:

```text
MIDI Parser -> MIDI Analyzer -> MIDI Arranger -> MusicEngineCore
```

It gives both:

* a full chronological event history
* module-specific log files

Recommended for multi-stage systems:

```cpp
logger.set_file_mode(
    moody::Loggr::FileMode::SingleFileAndPerModule
);
```

---

# Root CMake Configuration

```cmake
cmake_minimum_required(VERSION 3.10)

project(MoodyWorkspace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(moody)

add_executable(loggr_test
    test.cpp
)

target_link_libraries(loggr_test PRIVATE
    moody::loggr
)
```

---

# Moody Library CMake

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
```

---

# Quick Start

A complete usage example can be found in:

```text
test.cpp
```

The example demonstrates:

* logger creation
* log levels
* structured logging
* optional variables
* file output modes
* colored console output
* asynchronous logging
* flushing sinks

Build and run the example:

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
LOG_INFO(logger, "ANALYZER", "Detected musical key");
LOG_INFO(logger, "ARRANGER", "Generated arrangement section");
LOG_INFO(logger, "ENGINE", "Scheduled MIDI event");
```

---

# Logging Macros

Moody Loggr provides the following macros:

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

All logging macros are thread-safe and may be used from multiple threads simultaneously.

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

If an odd number of optional values is passed:

```text
<key=<missing>>
```

is printed automatically.

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

# Supported Features

## Core Features

* [x] Lightweight C++ logging library
* [x] Separate public header and implementation file
* [x] Asynchronous logging
* [x] Thread-safe producer/consumer architecture
* [x] Internal background worker thread
* [x] File logging
* [x] Per-module file logging
* [x] Combined global + per-module file logging
* [x] Console logging
* [x] Optional ANSI colored console output
* [x] Automatic log directory creation
* [x] Append or overwrite file output
* [x] Configurable minimum log level
* [x] Manual `flush()` support
* [x] Optional millisecond timestamps
* [x] Filename-only source location formatting
* [x] Automatic `__FILE__` and `__LINE__` capture through macros
* [x] Structured readable log format
* [x] Optional key/value variables
* [x] Doxygen-style documentation comments

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

---

# Planned Extensions

* [ ] JSON output format
* [ ] Multiple sinks
* [ ] Rotating file sink
* [ ] Pattern-based formatting
* [ ] Compile-time log level filtering
* [ ] Sensitive data filtering
* [ ] Rate limiting / throttling
* [ ] Thread-local logging context
* [ ] Extensible sink API
* [ ] Network/syslog/database sinks

---