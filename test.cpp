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
    loggr.log(moody::Loggr::Level::INFO, "CLIENT", "TESTING", {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::TRACE, "APP",    "TESTING", {__FILE__, __LINE__}, "x", x);
    loggr.log(moody::Loggr::Level::DEBUG, "RENDERER", "TESTING", {__FILE__, __LINE__}, &test.one, test.one, &test.two);
    loggr.log(moody::Loggr::Level::WARN, "GRAPHICS", "TESTING", {__FILE__, __LINE__}, "z", z);
    loggr.log(moody::Loggr::Level::ERROR, "TEXTURE", "TESTING", {__FILE__, __LINE__}, &y);
    loggr.log(moody::Loggr::Level::FATAL, "CONFIG", "TESTING", {__FILE__, __LINE__}, y);

    return 0;
}