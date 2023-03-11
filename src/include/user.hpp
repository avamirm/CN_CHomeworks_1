#ifndef USER_HPP
#define USER_HPP

#include <string>
#include "json.hpp"

#define USER_LOGGED_IN "Error 230: User logged in"
#define INFORMATION_CHANGED "Error 312: Information was changed successfully."
#define INVALID_USERNAME_OR_PASSWORD "Error 430: Invalid username or password"

using json = nlohmann::json;

class User
{
public:
    User(int id, std::string username, std::string password, bool isAdmin, std::string phoneNumber, std::string address, int money);
    ~User();
    std::string getName();
    std::string getPassword();
    int getFd();
    void setInfo(std::string &password, int id, std::string &phoneNumber, std::string &address, int money);
    json viewInfo(bool isSelfInfo);
    json editInformation(json command);
    bool isAdmin();

private:
    std::string username_;
    std::string password_;
    int id_;
    bool isAdmin_;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    int fd_;
};

#endif
