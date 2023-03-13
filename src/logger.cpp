#include "include/logger.hpp"

Logger::Logger()
{

}

void Logger::setPath(std::string path)
{
    path_ = path;
}

void Logger::log(std::string msg, bool isError)
{
    std::fstream logFile;
    logFile.open(path_, std::fstream::in | std::fstream::out | std::fstream::app);
    nlohmann::json j;
    auto current = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(current);
    std::string time = std::ctime(&currentTime);
    time.erase(time.end()-1);
    j["time"] = time;
    j["isError"] = isError;
    j["message"] = msg;
    logFile << j.dump() << std::endl;
    logFile.close();
}

void Logger::log(nlohmann::json &j, int clientFd)
{
    std::fstream logFile;
    logFile.open(path_, std::fstream::in | std::fstream::out | std::fstream::app);
    auto current = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(current);
    std::string time = std::ctime(&currentTime);
    time.erase(time.end()-1);
    j["time"] = time;
    j["clientFd"] = clientFd;
    logFile << j.dump() << std::endl;
    logFile.close();
}