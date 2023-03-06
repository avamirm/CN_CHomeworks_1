#ifndef __USER_H__
#define __USER_H__

#include <string>

class User 
{
public:
    User(std::string& name, int socket);
    ~User();
    std::string getName();
    std::string getPassword();
    int getSocket();
    void setInfo(std::string& password, int id, std::string& phoneNumber, std::string& address, int money);

private:
    std::string username_;
    std::string password_;
    int id_;
    bool isAdmin_;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    int socket_;
};

#endif