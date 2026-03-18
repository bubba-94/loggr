# Moody Loggr

> [!NOTE]
> UNDER CONSTRUCTION

## Description

Moody Loggr is a lightweight, header-only C++ logging library designed for quick integration into any project. The aim is to create a thread safe logging library with both file and console output.

## Core features

Logging levels: Supports DEBUG, INFO, WARNING, ERROR, and FATAL.

- [x] Header-only: Simply include `moody/loggr.hpp`.
- [x] File and console logging: Write logs to a file, the console, or both.
- [x] Configurable output: Supports appending to existing files or overwriting them.
- [x] Timestamped messages: Logs are automatically prepended with the current date and time.
- [x] Async logging: Logs are enqueued and processed accordingly.
- [-] Creation of just one log message
- [-] Integrate into existing application


## Planned extension

- [-] Structured logging: Support for key-value pairs and JSON output for easier parsing.
- [-] Scoped / contextual logging: Automatically include request IDs, user IDs, or other metadata.
- [-] Thread-local context propagation: Each thread can maintain its own logging context
- [-] Multiple sinks: Support for sending logs to multiple destinations simultaneously (console, file, network, database).
- [-] Rotating file sink: Automatic log rotation based on size or date.
- [-] Log formatting options: Pattern-based formatting (custom templates) and colored console output.
- [-] Compile-time log level filtering: Optionally remove DEBUG/TRACE logs in release builds for performance.
- [-] Error handling & safety: Fail-safe logging so that logging itself never crashes the program.
- [-] Sensitive data filtering: Redact passwords, tokens, or PII.
- [-] Rate limiting / throttling: Prevent log spam in high-frequency code paths.
- [-] Extensible API for new sinks: Network logging (HTTP, syslog), database, or in-memory.
- [-] Distributed tracing support: Optional integration with Trace ID / Span ID for distributed systems.