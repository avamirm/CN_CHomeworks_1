#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "json.hpp"
#include "errors.hpp"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "date.hpp"
#include <chrono>

#define SET_TIME "setTime"
#define SIGN_IN "signin"
#define SIGN_UP "signup"
#define CONFIG_FILE "config.json"

// #define VIEW_USER_INFORMATION "View user information"
// #define VIEW_ALL_USERS "View all users"
// #define VIEW_ROOMS_INFORMATION "View rooms information"
// #define BOOKING "Booking"
// #define CANCELING "Canceling"
// #define PASS_DAY "pass day"
// #define EDIT_INFORMATION "Edit information"
// #define LEAVING_ROOM "Leaving room"
// #define ROOMS "Rooms"
// #define LOGOUT "Logout"
using json = nlohmann::json;

#define VIEW_USER_INFORMATION "1"
#define VIEW_ALL_USERS "2"
#define VIEW_ROOMS_INFORMATION "3"
#define BOOKING "4"
#define CANCELING "5"
#define PASS_DAY "6"
#define EDIT_INFORMATION "7"
#define LEAVING_ROOM "8"
#define ROOMS "9"
#define LOGOUT "0"

#define ADD "add"
#define MODIFY "modify"
#define REMOVE "remove"
#define CAPACITY "capacity"

// #define USER_LOGGED_IN "230: User logged in."
// #define USER_SIGNED_UP "311: User Signed up. Enter your password, purse, phone and address."
// #define INVALID_USERNAME_OR_PASSWORD "430: Invalid username or password."
// #define BAD_SEQUENCE_OF_COMMANDS "503: Bad sequence of commands."

class Client
{
public:
    Client();
    void run();
    int connectServer(const int port, const std::string host);
    bool checkDate(std::string date);
    bool checkDigits(std::string digits);
    std::vector<std::string> tokenizeCommand(std::string command);
    void signInCommand(std::vector<std::string> &tokens);
    bool signUpCommand(std::vector<std::string> &tokens);
    bool isTokenSizeCorrect(int tokenSize, int correctNum);
    void viewUserInfoCommand();
    void viewAllUsers();
    void viewRoomsInfo();
    bool booking();
    bool canceling();
    bool passDay();
    bool editInfo();
    bool leaveRoom();
    bool changeCapacity(std::string newCap);
    void roomCommand();
    bool addRoom(std::vector<std::string> &tokens);
    bool modifyRoom(std::vector<std::string> &tokens);
    bool removeRoom(std::string &roomNo);
    void logout();
    void readConfig();

private:
    bool hasLoggedIn_;
    bool isRoomCmd_;
    bool isLeavingRoomCmd_;
    int commandFd_;
    char readBuffer[1024];
    int port_;
    std::string hostname_;
};

#endif
