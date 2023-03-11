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
    // I guess this will always be empty
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

void Client::run()
{
    // Configuration config(configFilePath);
    std::string command;
    int commandFd;
    std::ifstream fin(CONFIG_FILE);
    json j;
    fin >> j;
    int port = j["commandChannelPort"].get<int>();
    std::string hostname = j["hostName"].get<std::string>();
    commandFd = connectServer(port, hostname);
    char readBuffer[1024];
    std::vector<std::string> tokens;
    bool isRoom = false, isLeavingRoom = false, isLogin = false;
    while (std::getline(std::cin, command))
    {
        tokens = tokenizeCommand(command);
        std::string cmd = tokens[0];
        memset(readBuffer, 0, 1024);
        nlohmann::json message;
        if (cmd == SIGN_IN)
        {
            if (tokens.size() != 3)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            std::string username = tokens[1];
            std::string password = tokens[2];
            message["cmd"] = "signin";
            message["username"] = username;
            message["password"] = password;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
            if (!recvMessage["isError"])
            {
                isLogin = true;
            }
            std::cout << recvMessage["errorMessage"] << std::endl;
        }

        else if (cmd == SIGN_UP)
        {
            if (tokens.size() != 2)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            std::string username = tokens[1];
            message["cmd"] = "signup";
            message["username"] = username;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);

            if (isLogin)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            }

            else if (recvMessage["isError"])
            {
                std::cout << recvMessage["errorMessage"] << std::endl;
            }

            else if (!recvMessage["isError"])
            {
                std::cout << recvMessage["errorMessage"] << std::endl;
                std::cout << USER_SIGNED_UP;
                recvMessage["cmd"] = "SuccessSignup";
                std::string password, purse, phone, address;
                std::cin >> password;
                recvMessage["password"] = password;
                std::cin >> purse;
                if (!checkDigits(purse))
                {
                    std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                    continue;
                }
                recvMessage["purse"] = purse;
                std::cin >> phone;
                if (!checkDigits(phone))
                {
                    std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                    continue;
                }
                recvMessage["phone"] = phone;
                std::cin >> address;
                recvMessage["address"] = address;
                std::string messageStr = recvMessage.dump();
                send(commandFd, messageStr.c_str(), messageStr.size(), 0);
                memset(readBuffer, 0, 1024);
                recv(commandFd, readBuffer, sizeof(readBuffer), 0);
                nlohmann::json recvMessage = json::parse(readBuffer);
                std::cout << recvMessage["errorMessage"] << std::endl;
            }
        }

        else if (cmd == VIEW_USER_INFORMATION)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            message["cmd"] = "View user information";
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
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

        else if (cmd == VIEW_ALL_USERS)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            message["cmd"] = "View all users";
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
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

        else if (cmd == VIEW_ROOMS_INFORMATION)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            message["cmd"] = "View rooms information";
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
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

        else if (cmd == BOOKING)
        {
            std::getline(std::cin, command);
            tokens = tokenizeCommand(command);
            if (tokens.size() != 5)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            std::string roomNo = tokens[1];
            std::string numOfBeds = tokens[2];
            std::string checkInDate = tokens[3];
            std::string checkOutDate = tokens[4];

            if (!checkDigits(roomNo) || !checkDigits(numOfBeds))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }

            if (!checkDate(checkInDate) || !checkDate(checkOutDate))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            message["cmd"] = "Booking";
            message["roomNum"] = roomNo;
            message["numOfBeds"] = numOfBeds;
            message["checkInDate"] = checkInDate;
            message["checkOutDate"] = checkOutDate;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == CANCELING)
        {
            std::getline(std::cin, command);
            tokens = tokenizeCommand(command);
            if (tokens.size() != 3)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            std::string roomNo = tokens[1];
            std::string reserveNum = tokens[2];
            if (!checkDigits(roomNo) || !checkDigits(reserveNum))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            message["cmd"] = "Canceling";
            message["roomNum"] = roomNo;
            message["Num"] = reserveNum;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == PASS_DAY)
        {
            std::getline(std::cin, command);
            tokens = tokenizeCommand(command);
            if (tokens.size() != 2)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string value = tokens[1];
            if (!checkDigits(value))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            message["cmd"] = "pass day";
            message["value"] = value;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == EDIT_INFORMATION)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string newPassword, phone, address;
            std::cin >> newPassword;
            std::cin >> phone;
            std::cin >> address;
            if (!checkDigits(phone))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = false;
            message["cmd"] = "Edit information";
            message["newPassWord"] = newPassword;
            message["phone"] = phone;
            message["address"] = address;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == LEAVING_ROOM)
        {
            std::getline(std::cin, command);
            tokens = tokenizeCommand(command);
            if (tokens.size() != 2)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string value = tokens[1];
            if (!checkDigits(value))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::cout << "You can change the Capacity by entering the corresponding command." << std::endl;
            isRoom = false, isLeavingRoom = true;
            message["cmd"] = "Leaving room";
            message["value"] = value;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        ////////////////////////////////////////////////////////
        else if (cmd == CAPACITY)
        {
            if (tokens.size() != 2 || !isLeavingRoom)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string newCap = tokens[1];
            if (!checkDigits(newCap))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = false, isLeavingRoom = true;
            message["cmd"] = CAPACITY;
            message["newCap"] = newCap;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }
        //////////////////////////////////////////////////////////

        else if (cmd == ROOMS)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::cout << "You can Add / Modify / Remove rooms by entering the corresponding command." << std::endl;
            message["cmd"] = "Rooms";
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
            if (!recvMessage["isError"])
            {
                isRoom = true, isLeavingRoom = false;
            }
            else if (recvMessage["isError"])
            {
                std::cout << recvMessage["errorMessage"] << std::endl;
                isRoom = false, isLeavingRoom = false;
            }
        }

        ///////////////////////////////////////////////////////
        else if (cmd == ADD)
        {
            if (tokens.size() != 4 || !isRoom)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string roomNo = tokens[1];
            std::string maxCap = tokens[2];
            std::string price = tokens[3];
            if (!checkDigits(roomNo) || !checkDigits(maxCap) || !checkDigits(price))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = true, isLeavingRoom = false;
            message["cmd"] = ADD;
            message["roomNum"] = roomNo;
            message["maxCap"] = maxCap;
            message["price"] = price;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == MODIFY)
        {
            if (tokens.size() != 4 || !isRoom)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string roomNo = tokens[1];
            std::string newMaxCap = tokens[2];
            std::string price = tokens[3];
            if (!checkDigits(roomNo) || !checkDigits(newMaxCap) || !checkDigits(price))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = true, isLeavingRoom = false;
            message["cmd"] = MODIFY;
            message["roomNum"] = roomNo;
            message["newMaxCap"] = newMaxCap;
            message["price"] = price;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }

        else if (cmd == REMOVE)
        {
            if (tokens.size() != 2 || !isRoom)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            std::string roomNo = tokens[1];
            if (!checkDigits(roomNo))
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            isRoom = true, isLeavingRoom = false;
            message["cmd"] = REMOVE;
            message["roomNum"] = roomNo;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }
        //////////////////////////////////////////////////////////////
        else if (cmd == LOGOUT)
        {
            if (tokens.size() != 1)
            {
                std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
                continue;
            }
            message["cmd"] = "Logout";
            isRoom = false, isLeavingRoom = false;
            std::string messageStr = message.dump();
            send(commandFd, messageStr.c_str(), messageStr.size(), 0);
            recv(commandFd, readBuffer, sizeof(readBuffer), 0);
            nlohmann::json recvMessage = json::parse(readBuffer);
        }
    }
}

int main()
{
    Client client;
    client.run();
}