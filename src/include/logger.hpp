#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <memory>

/**
 * Simple logger class which logs messages to files.
 * Not thread-safe!
*/
class Logger {
    public:
        Logger(std::string filePath);

        void log(std::string msg);

    private:
        std::unique_ptr<std::ofstream> stream;
};