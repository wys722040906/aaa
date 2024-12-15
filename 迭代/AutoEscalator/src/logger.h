#pragma once
#include <string>
#include <fstream>
#include <ctime>

class Logger {
private:
    static std::ofstream logFile;
    
public:
    static void init(const std::string& filename = "elevator.log");
    static void log(const std::string& message);
    static void close();
}; 