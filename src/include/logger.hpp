#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "json.hpp"
#include <string>
#include <fstream>
#include <ctime>
#include <chrono>

class Logger
{
public:
    Logger();
    void log(std::string msg, bool isError);
    void log(nlohmann::json &msg, int clientFd);
    void setPath(std::string path);
private:
    std::string path_;
};
#endif