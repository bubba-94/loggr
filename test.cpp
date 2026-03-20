#include "moody/Loggr.hpp"

using namespace moody;

int main (){

    struct Test{
        double two;
        int one;
    };

    Test test {2.0, 2};
    int x = 1337;
    double y = 0.0;
    std::string z = "JOHAN";
    
    Loggr loggr("logs", "ppw", "output.txt", true, false, true);

    // Last argument is an array of optional values defined in LogOptions
    LOG_INFO(loggr, "CLIENT", "TESTING");
    LOG_TRACE(loggr, "APP", "TESTING", &z);
    LOG_DEBUG(loggr, "RENDERER", "TESTING", &x);
    LOG_WARN(loggr, "GRAPHICS", "TESTING", &test.two, test.one);
    LOG_ERROR(loggr, "TEXTURE", "TESTING", &test);
    LOG_FATAL(loggr, "CLIENT", "TESTING", &x);

    return 0;
}