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

    loggr.log(moody::Loggr::Level::INFO, "UI", "TESTING", {});
    loggr.log(moody::Loggr::Level::TRACE, "APP", "TRACE", &test.one, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::TRACE, "APP", "TRACE", &test.two, {__FILE__, __LINE__});
    loggr.log(moody::Loggr::Level::TRACE, "APP", "TRACE", &x, {__FILE__, __LINE__});

    return 0;
}