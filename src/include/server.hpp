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

#define ROOMS_FILE "RoomsInfo.json"
#define USERS_FILE "UsersInfo.json"
#define CONFIG_FILE "config.json"

#define SET_TIME "setTime"
#define SIGN_IN "signin"
#define SIGN_UP "signup"

#define INVALID_VALUE "401: Invalid value!"
#define BAD_SEQUENCE_OF_COMMANDS "503: Bad sequence of commands."

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

private:
    CommandHandler commandHandler_;
    int port_;
    std::string hostname_;
    date::year_month_day serverDate;
};

#endif
