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
    loggr.log(moody::Loggr::Level::INFO,    "RENDERER", "TESTING", {});
    loggr.log(moody::Loggr::Level::INFO,    "RENDERER", "TESTING", {__FILE__});
    loggr.log(moody::Loggr::Level::TRACE,   "APP",      "TESTING", &test.one, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::DEBUG,   "WINDOW",   "TESTING", &test.two, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::WARN,    "GRAPHICS", "TESTING", &x, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::ERROR,   "TEXTURE",  "TESTING", &y, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::FATAL,   "CLIENT",   "TESTING", &z, {__FILE__, __LINE__});

    return 0;
}