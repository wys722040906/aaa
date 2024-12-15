#include "logger.h"
#include <iomanip>

std::ofstream Logger::logFile;

void Logger::init(const std::string& filename) {
    logFile.open(filename, std::ios::app);
}

void Logger::log(const std::string& message) {
    if (!logFile.is_open()) return;
    
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    
    logFile << std::put_time(tm, "[%Y-%m-%d %H:%M:%S] ")
            << message << std::endl;
}

void Logger::close() {
    if (logFile.is_open()) {
        logFile.close();
    }
} 