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
    loggr.log(moody::Loggr::Level::WARN, "GRAPHICS", "TESTING", {__FILE__, __LINE__}, "host", host); 
    
    // Value of varaible: y
    loggr.log(moody::Loggr::Level::ERROR, "TEXTURE", "TESTING", {__FILE__, __LINE__}, "y", y);  
    
    // Adress of variable: y
    loggr.log(moody::Loggr::Level::FATAL, "CONFIG", "TESTING", {__FILE__, __LINE__}, "&y", &y); 

    return 0;
}

