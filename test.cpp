#include "moody/Loggr.hpp"

int main (){
    
    moody::Loggr::LevelSeverity sev = moody::Loggr::LevelSeverity::DEBUG;

    moody::Loggr loggr("logs", "ppw", "output.txt", true, false);

    loggr.msg("Hello", sev);
    sev = moody::Loggr::LevelSeverity::INFO;
    loggr.msg("My", sev);
    sev = moody::Loggr::LevelSeverity::WARNING;
    loggr.msg("Name", sev);
    sev = moody::Loggr::LevelSeverity::ERROR;
    loggr.msg("Is", sev);
    sev = moody::Loggr::LevelSeverity::FATAL;
    loggr.msg("Johan", sev);

    loggr.write();

    return 0;
}