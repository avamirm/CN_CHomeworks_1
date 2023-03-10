#include "include/client.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>

Client::Client()
{
    hasLoggedIn_ = false;
    isRoomCmd_ = false;
    isLeavingRoomCmd_ = false;
    readConfig();
}

int Client::connectServer(const int port, const std::string host)
{
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host.c_str());

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cout << "Error in connecting to server\n";
        return -1;
    }
    return fd;
}

bool Client::checkDate(std::string date)
{
    // istringstream ss(date);
    date::year_month_day tempServerDate;
    std::cin >> date::parse("%F", tempServerDate);
    if (!std::cin)
    {
        std::cin.clear();
        return false;
    }
    return true;
}

bool Client::checkDigits(std::string digits)
{
    for (int i = 0; i < digits.size(); i++)
    {
        if (!isdigit(digits[i]))
        {
            return false;
        }
    }
    return true;
}

std::vector<std::string> Client::tokenizeCommand(std::string command)
{
    std::vector<std::string> tokens;
    std::stringstream check1(command);
    std::string line;
    while (getline(check1, line, ' '))
    {
        tokens.push_back(line);
    }
    return tokens;
}

void Client::signInCommand(std::vector<std::string> &tokens)
{
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    std::string username = tokens[1];
    std::string password = tokens[2];
    message["cmd"] = "signin";
    message["username"] = username;
    message["password"] = password;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (!recvMessage["isError"])
    {
        hasLoggedIn_ = true;
    }
    std::cout << recvMessage["errorMessage"] << std::endl;
}

bool Client::signUpCommand(std::vector<std::string> &tokens)
{
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    std::string username = tokens[1];
    message["cmd"] = "signup";
    message["username"] = username;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);

    if (recvMessage["isError"])
        std::cout << recvMessage["errorMessage"] << std::endl;
    else 
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        std::cout << USER_SIGNED_UP;
        recvMessage["cmd"] = "SuccessSignup";
        recvMessage["username"] = username;
        std::string password, purse, phone, address;
        std::cin >> password;
        recvMessage["password"] = password;
        std::cin >> purse;
        if (!checkDigits(purse))
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        recvMessage["money"] = purse;
        std::cin >> phone;
        if (!checkDigits(phone))
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        recvMessage["phoneNumber"] = phone;
        std::cin >> address;
        recvMessage["address"] = address;
        std::string messageStr = recvMessage.dump();
        send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
        memset(readBuffer, 0, 1024);
        recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
        nlohmann::json recvMessage = json::parse(readBuffer);
        std::cout << recvMessage["errorMessage"] << std::endl;
    }
    return true;
}

void Client::viewUserInfoCommand()
{
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    message["cmd"] = "View user information";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << "\u2022 User Info:" << std::endl
                << "  ID:        " << recvMessage["id"] << std::endl
                << "  Username:  " << recvMessage["username"] << std::endl
                << "  Password:  " << recvMessage["password"] << std::endl
                << "  Admin:     " << recvMessage["isAdmin"] << std::endl;
    if (recvMessage["isAdmin"])
    {
        std::cout << "  Phone:     " << recvMessage["phoneNumber"] << std::endl
                    << "  Money:     " << recvMessage["money"] << std::endl
                    << "  Address:   " << recvMessage["address"] << std::endl;
    }
}

void Client::viewAllUsers()
{
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    message["cmd"] = "View all users";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
    }
    std::cout << "\u2022 User Info:" << std::endl
                << "  ID:        " << recvMessage["id"] << std::endl
                << "  Username:  " << recvMessage["username"] << std::endl
                << "  Admin:     " << recvMessage["isAdmin"] << std::endl;
    if (recvMessage["isAdmin"])
    {
        std::cout << "  Phone:     " << recvMessage["phoneNumber"] << std::endl
                    << "  Money:     " << recvMessage["money"] << std::endl
                    << "  Address:   " << recvMessage["address"] << std::endl;
    }
}

bool Client::isTokenSizeCorrect(int tokenSize, int correctNum)
{
    return tokenSize == correctNum;
}

void Client::viewRoomsInfo()
{
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    message["cmd"] = "View rooms information";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << "\u2022 Rooms Info: " << std::endl
                << "  Number:         " << recvMessage["roomNo"] << std::endl
                << "  Price:          " << recvMessage["price"] << std::endl
                << "  Max Capacity:   " << recvMessage["maxCapacity"] << std::endl
                << "  Free Capacity:  " << recvMessage["freeCapacity"] << std::endl;
    for (auto &user : recvMessage["users"])
    {
        std::cout << "\u2022 Users: " << std::endl
                    << "  User ID:         " << user["userId"] << std::endl
                    << "  Number of Beds:  " << user["numOfBeds"] << std::endl
                    << "  Reserve Date:    " << user["reserveDate"] << std::endl
                    << "  CheckOut Date:   " << user["checkOutDate"] << std::endl;
    }
}

bool Client::booking()
{
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (!isTokenSizeCorrect(tokens.size(), 5))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    std::string roomNo = tokens[1];
    std::string numOfBeds = tokens[2];
    std::string checkInDate = tokens[3];
    std::string checkOutDate = tokens[4];

    if (!checkDigits(roomNo) || !checkDigits(numOfBeds))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }

    if (!checkDate(checkInDate) || !checkDate(checkOutDate))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    message["cmd"] = "Booking";
    message["roomNum"] = roomNo;
    message["numOfBeds"] = numOfBeds;
    message["checkInDate"] = checkInDate;
    message["checkOutDate"] = checkOutDate;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::canceling()
{
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (!isTokenSizeCorrect(tokens.size(), 3))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    std::string roomNo = tokens[1];
    std::string reserveNum = tokens[2];
    if (!checkDigits(roomNo) || !checkDigits(reserveNum))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    message["cmd"] = "Canceling";
    message["roomNum"] = roomNo;
    message["Num"] = reserveNum;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::passDay()
{
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    if (!isTokenSizeCorrect(tokens.size(), 2))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    nlohmann::json message;
    std::string value = tokens[1];
    if (!checkDigits(value))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    message["cmd"] = "pass day";
    message["value"] = value;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::editInfo()
{
    nlohmann::json message;
     std::string newPassword, phone, address;
    std::cin >> newPassword;
    std::cin >> phone;
    std::cin >> address;
    if (!checkDigits(phone))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    message["cmd"] = "Edit information";
    message["newPassWord"] = newPassword;
    message["phone"] = phone;
    message["address"] = address;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::leaveRoom()
{
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (isTokenSizeCorrect(tokens.size(), 2))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    std::string value = tokens[1];
    if (!checkDigits(value))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    std::cout << "You can change the Capacity by entering the corresponding command." << std::endl;
    isRoomCmd_ = false, isLeavingRoomCmd_ = true;
    message["cmd"] = "Leaving room";
    message["value"] = value;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::changeCapacity(std::string newCap)
{
    if (!checkDigits(newCap))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    nlohmann::json message;
    isRoomCmd_ = false, isLeavingRoomCmd_ = true;
    message["cmd"] = CAPACITY;
    message["newCap"] = newCap;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

void Client::roomCommand()
{
    nlohmann::json message;
    std::cout << "You can Add / Modify / Remove rooms by entering the corresponding command." << std::endl;
    message["cmd"] = "Rooms";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (!recvMessage["isError"])
    {
        isRoomCmd_ = true, isLeavingRoomCmd_ = false;
    }
    else if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    }
}

bool Client::addRoom(std::vector<std::string> &tokens)
{
    nlohmann::json message;
    std::string roomNo = tokens[1];
    std::string maxCap = tokens[2];
    std::string price = tokens[3];
    if (!checkDigits(roomNo) || !checkDigits(maxCap) || !checkDigits(price))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = true, isLeavingRoomCmd_ = false;
    message["cmd"] = ADD;
    message["roomNum"] = roomNo;
    message["maxCap"] = maxCap;
    message["price"] = price;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::modifyRoom(std::vector<std::string> &tokens)
{
    nlohmann::json message;
    std::string roomNo = tokens[1];
    std::string newMaxCap = tokens[2];
    std::string price = tokens[3];
    if (!checkDigits(roomNo) || !checkDigits(newMaxCap) || !checkDigits(price))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = true, isLeavingRoomCmd_ = false;
    message["cmd"] = MODIFY;
    message["roomNum"] = roomNo;
    message["newMaxCap"] = newMaxCap;
    message["newPrice"] = price;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

bool Client::removeRoom(std::string &roomNo)
{
    nlohmann::json message;
    if (!checkDigits(roomNo))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = true, isLeavingRoomCmd_ = false;
    message["cmd"] = REMOVE;
    message["roomNum"] = roomNo;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    return true;
}

void Client::logout()
{
    nlohmann::json message;
    message["cmd"] = "Logout";
    isRoomCmd_ = false, isLeavingRoomCmd_ = false;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
}

void Client::readConfig()
{
    std::ifstream fin(CONFIG_FILE);
    json j;
    fin >> j;
    port_ = j["commandChannelPort"].get<int>();
    hostname_ = j["hostName"].get<std::string>();
}

void Client::run()
{
    std::string command;
    commandFd_ = connectServer(port_, hostname_);
    std::vector<std::string> tokens;
    while (std::getline(std::cin, command))
    {
        tokens = tokenizeCommand(command);
        std::string cmd = tokens[0];
        memset(readBuffer, 0, 1024);
        if (cmd == SIGN_IN && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 3))
            signInCommand(tokens);
        
        else if (cmd == SIGN_UP && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 2))
            if (!signUpCommand(tokens))
                continue;

        else if (cmd == VIEW_USER_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            viewUserInfoCommand();

        else if (cmd == VIEW_ALL_USERS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
            viewAllUsers();
        
        else if (cmd == VIEW_ROOMS_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            viewRoomsInfo();

        else if (cmd == BOOKING && hasLoggedIn_)
            if (!booking())
                continue;    

        else if (cmd == CANCELING && hasLoggedIn_)
            if (!canceling())
                continue;

        else if (cmd == PASS_DAY && hasLoggedIn_)
            if (!passDay())
                continue;

        else if (cmd == EDIT_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            if (!editInfo())
                continue;

        else if (cmd == LEAVING_ROOM && hasLoggedIn_)
            if (!leaveRoom())
                continue;

        ////////////////////////////////////////////////////////
        else if (cmd == CAPACITY && isTokenSizeCorrect(tokens.size(), 2) && isLeavingRoomCmd_)
            if(!changeCapacity(tokens[1]))
                continue;
        //////////////////////////////////////////////////////////

        else if (cmd == ROOMS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
            roomCommand();
        
        ///////////////////////////////////////////////////////
        else if (cmd == ADD && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
            if (!addRoom(tokens))
                continue;

        else if (cmd == MODIFY && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
            if (!modifyRoom(tokens))
                continue;

        else if (cmd == REMOVE && isTokenSizeCorrect(tokens.size(), 2) && isRoomCmd_ && hasLoggedIn_)
            if (!removeRoom(tokens[1]))
                continue;
                
        //////////////////////////////////////////////////////////////
        else if (cmd == LOGOUT && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            logout();
    }
}

int main()
{
    Client client;
    client.run();
}

