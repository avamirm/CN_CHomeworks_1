#include "user.hpp"

User::User(std::string& username, int socket)
{
    username_ = username;
    socket_ = socket;
}

User::~User()
{
}

void User::setInfo(std::string& password, int id, std::string& phoneNumber, std::string& address, int money)
{
    password_ = password;
    id_ = id;
    phoneNumber_ = phoneNumber;
    address_ = address;
    money_ = money;
}

std::string User::getName()
{
    return username_;
}

std::string User::getPassword()
{
    return password_;
}

int getSocket()
{
    return socket_;
}