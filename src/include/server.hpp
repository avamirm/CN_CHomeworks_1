#ifndef SERVER_HPP
#define SERVER_HPP

#include "commandHandler.hpp"

#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <iostream>
#include "date.hpp"
#include <chrono>
#include <sstream>

#define CONFIG_FILE "jsonFiles/config.json"

#define SET_TIME "setTime"
#define SIGN_IN "signin"
#define SIGN_UP "signup"

class Server
{
public:
    Server();
    void start();
    int acceptClient(int server_fd);
    int setupServer(int port);
    bool checkDate(std::string date);
    void setServerDate(std::string date);
    void readPortHostnameConfig();
    json readRoomsConfig();
    json readUsersConfig();
    std::vector<std::string> tokenizeCommand(std::string command);
    int setupServer();
    void setDate();

private:
    CommandHandler commandHandler_;
    int port_;
    std::string hostname_;
};

#endif
