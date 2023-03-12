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

const std::string HELP =  "Please enter the number of your choice.\n"
                "\u2022 1) View User Information \n" 
                "\u2022 2) View All Users \n" 
                "\u2022 3) View Rooms Information \n" 
                "\u2022 4) Booking \n" 
                "\u2022 5) Canceling \n" 
                "\u2022 6) Pass Dayn \n" 
                "\u2022 7) Edit Information \n" 
                "\u2022 8) Leaving Room \n" 
                "\u2022 9) Rooms \n" 
                "\u2022 10) Make The Room Empty \n" 
                "\u2022 0) Logout \n" ;

const std::string ROOM_HELP = "\u2022 Add -> add <roomNo> <maxCapacity> <price>\n" 
                              "\u2022 Modify -> modify <roomNo> <newMaxCapacity> <newPrice>\n" 
                              "\u2022 Remove -> remove <roomNo> \n" ;
const std::string SIGNUP_IN_HELP = "Welcome to Nesava Hotel. You can signIn or signUp using commands below:\n"
                                   "\u2022 signIn -> signin <username> <password>\n" 
                                   "\u2022 signUp -> signup <username> \n" ;

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
#define FREE_ROOM "10"

#define ADD "add"
#define MODIFY "modify"
#define REMOVE "remove"


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
    bool signUpCommand(std::string &username);
    bool isTokenSizeCorrect(int tokenSize, int correctNum);
    void viewUserInfoCommand();
    bool viewAllUsers();
    bool viewRoomsInfo();
    bool booking();
    bool canceling();
    bool passDay();
    bool editInfo();
    bool leaveRoom();
    bool freeRoom();
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
