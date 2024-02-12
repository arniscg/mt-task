#include "include/logger.hpp"
#include <iomanip>
#include <ctime>

/**
 * Simple logger class which logs messages to files.
 * Not thread-safe!
*/
Logger::Logger(std::string filePath) {
    this->stream = std::make_unique<std::ofstream>(filePath);
}

void Logger::log(std::string msg) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    *(this->stream.get())
        << std::put_time(&tm, "[%d-%m-%Y %H:%M:%S] ")
        << msg
        << std::endl;

    this->stream->flush();
}