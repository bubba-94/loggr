# Moody Loggr

> [!NOTE]
> UNDER CONSTRUCTION

## Description

Moody Loggr is a lightweight, header-only C++ logging library designed for quick integration into any project. The aim is to create a simple thread safe logging library with both file and console output.

## Usage

Clone or copy Loggr.hpp and test with the program provided below.

## Testing

```cpp
#include "moody/Loggr.hpp"

using namespace moody;

int main (){

    struct Test{
        double two;
        int one;
    };

    Test test {13.3, 7};
    int x = 1337;
    double y = 9.4;
    std::string host = "bubba94";
    
    Loggr loggr("logs", "test", "output.txt", true, false, true);

    // No variables provided is used for outputting info.
    loggr.log(moody::Loggr::Level::INFO, "CLIENT", "TESTING", {__FILE__, __LINE__});

    // Variable arguments are sent in pairs
    // Value of varaible: x
    loggr.log(moody::Loggr::Level::TRACE, "APP",    "TESTING", {__FILE__, __LINE__}, "x", x); 

    // Values of variables: test.one, test.two
    loggr.log(moody::Loggr::Level::DEBUG, "RENDERER", "TESTING", {__FILE__, __LINE__}, "test.two", test.two, "test.one", test.one);

    // Value of variable: name
    loggr.log(moody::Loggr::Level::WARN, "GRAPHICS", "TESTING", {__FILE__, __LINE__}, "host", z); 
    
    // Value of varaible: y
    loggr.log(moody::Loggr::Level::ERROR, "TEXTURE", "TESTING", {__FILE__, __LINE__}, "y", y);  
    
    // Adress of variable: y
    loggr.log(moody::Loggr::Level::FATAL, "CONFIG", "TESTING", {__FILE__, __LINE__}, "&y", &y); 

    return 0;
}
```

### Output from above program
```bash
# The terminal window will add matching coloring
<timestamp> [ INFO] [  CLIENT  ] in:test.cpp(20) msg:{TESTING}
<timestamp> [TRACE] [   APP    ] in:test.cpp(21) var:{x=1337} msg:{TESTING}
<timestamp> [DEBUG] [ RENDERER ] in:test.cpp(22) vars:{test.one=13.3:test.two=7} msg:{TESTING}
<timestamp> [ WARN] [ GRAPHICS ] in:test.cpp(23) var:{name=bubba94} msg:{TESTING}
<timestamp> [ERROR] [ TEXTURE  ] in:test.cpp(24) var:{y=9.4} msg:{TESTING}
<timestamp> [FATAL] [  CONFIG  ] in:test.cpp(25) var:{&y=<address>} msg:{TESTING}
```

## Core features

Logging levels: Supports INFO, TRACE, DEBUG, WARN, ERROR, and FATAL.

- [x] Header-only: Simply include `moody/loggr.hpp`.
- [x] File and console logging: Write logs to a file, the console, or both.
- [x] Configurable output: Supports appending to existing files or overwriting them.
- [x] Timestamped messages: Logs are automatically prepended with the current date and time.
- [x] Async logging: Logs are enqueued and processed accordingly.
- [x] Creation of just one log message.
- [x] Integrate into existing application.
- [x] Optimize the optional parameter to prepare for scaling and performance.


## Planned extension

- [ ] Structured logging: Support for key-value pairs and JSON output for easier parsing.
- [ ] Scoped / contextual logging: Automatically include request IDs, user IDs, or other metadata.
- [ ] Thread-local context propagation: Each thread can maintain its own logging context
- [ ] Multiple sinks: Support for sending logs to multiple destinations simultaneously (console, file, network, database).
- [ ] Rotating file sink: Automatic log rotation based on size or date.
- [ ] Log formatting options: Pattern-based formatting (custom templates) and colored console output.
- [ ] Compile-time log level filtering: Optionally remove DEBUG/TRACE logs in release builds for performance.
- [ ] Error handling & safety: Fail-safe logging so that logging itself never crashes the program.
- [ ] Sensitive data filtering: Redact passwords, tokens, or PII.
- [ ] Rate limiting / throttling: Prevent log spam in high-frequency code paths.
- [ ] Extensible API for new sinks: Network logging (HTTP, syslog), database, or in-memory.
- [ ] Distributed tracing support: Optional integration with Trace ID / Span ID for distributed systems.