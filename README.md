# Moody Loggr

> [!NOTES]
> UNDER CONSTRUCTION

## Description

Moody Loggr is a lightweight, header-only C++ logging library designed for quick integration into any project. The aim is to create a thread safe logging library with both file and console output.

## Features (to be)

Logging levels: Supports DEBUG, INFO, WARNING, ERROR, and FATAL.

* - [x] Header-only: Simply include `moody/loggr.hpp`.
* - [x] File and console logging: Write logs to a file, the console, or both.
* - [x] Configurable output: Supports appending to existing files or overwriting them.
* - [x] Timestamped messages: Logs are automatically prepended with the current date and time.
* - [] Async logging: Logs are enqueued and sent in batches every ish seconds.
