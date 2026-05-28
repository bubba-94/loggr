#include "moody/Loggr.hpp"

#include <string>

using namespace moody;

/*  
    Expected output: 
    
    logs/test/output.txt
    logs/test/modules/CLIENT.log
    logs/test/modules/APP.log
    logs/test/modules/RENDERER.log
    logs/test/modules/GRAPHICS.log
    logs/test/modules/TEXTURE.log
    logs/test/modules/CONFIG.log
    logs/test/modules/PARSER.log
    logs/test/modules/ENGINE.log
*/

int main()
{
    struct Test {
        double two;
        int one;
    };

    Test test{13.3, 7};

    int x = 1337;
    double y = 9.4;
    std::string host = "bubba94";

    Loggr loggr(
        "logs",
        "test",
        "output.txt",
        true,   // console output
        true,   // append
        true,   // colored console
        true    // millisecond timestamps
    );

    loggr.set_level(Loggr::TRACE);

    // Test global + per-module logging
    loggr.set_file_mode(Loggr::FileMode::SingleFileAndPerModule);

    LOG_INFO(loggr, "CLIENT", "Testing info log");

    LOG_TRACE(loggr, "APP", "Testing trace log", "x", x);

    LOG_DEBUG(loggr, "RENDERER", "Testing debug log", "test.two", test.two, "test.one", test.one);

    LOG_WARN(loggr, "GRAPHICS", "Testing warn log", "host", host);

    LOG_ERROR(loggr, "TEXTURE", "Testing error log", "y", y);

    LOG_FATAL(loggr, "CONFIG", "Testing fatal log", "&y", &y);

    // Test odd optional argument count
    LOG_DEBUG(loggr, "PARSER", "Testing missing optional value", "orphan_key");

    // Test changing timestamp precision at runtime
    loggr.set_milliseconds(false);
    LOG_INFO(loggr, "ENGINE", "Testing timestamp without milliseconds");

    loggr.set_milliseconds(true);
    LOG_INFO(loggr, "ENGINE", "Testing timestamp with milliseconds");

    loggr.flush();

    return 0;
}