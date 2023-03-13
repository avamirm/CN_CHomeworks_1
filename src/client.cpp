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
        perror("connect");
        return -1;
    }
    return fd;
}

bool Client::checkDate(std::string date)
{
    std::istringstream ss(date);
    date::year_month_day resultDate;
    ss >> date::parse("%F", resultDate); // %F: %Y-%M-%d
    if (ss.fail())
        return false;
    std::string tempStr;
    if (ss >> tempStr)
        return false;
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
    isRoomCmd_ = false;
    std::string username = tokens[1];
    std::string password = tokens[2];
    message["cmd"] = "signin";
    message["username"] = username;
    message["password"] = password;
    std::string messageStr = message.dump();
    if (send(commandFd_, messageStr.c_str(), messageStr.size(), 0) < 0)
        perror("send");
    if (recv(commandFd_, readBuffer, sizeof(readBuffer), 0) < 0)
        perror("recieve");
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (!recvMessage["isError"])
    {
        hasLoggedIn_ = true;
    }
    std::cout << recvMessage["errorMessage"] << std::endl;
}

bool Client::signUpCommand(std::string &username)
{
    nlohmann::json message;
    isRoomCmd_ = false;
    message["cmd"] = "signup";
    message["username"] = username;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);

    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    else
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        recvMessage["cmd"] = "SuccessSignup";
        recvMessage["username"] = username;
        std::string password, purse, phone, address;
        std::cout << "password: ";
        std::getline(std::cin, password);
        recvMessage["password"] = password;
        std::cout << "money: ";
        std::getline(std::cin, purse);
        if (!checkDigits(purse))
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        recvMessage["money"] = purse;
        std::cout << "phone: ";
        std::getline(std::cin, phone);
        if (!checkDigits(phone) || phone.length() != 11)
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        recvMessage["phoneNumber"] = phone;
        std::cout << "address: ";
        std::getline(std::cin, address);
        recvMessage["address"] = address;
        std::string messageStr = recvMessage.dump();
        if (send(commandFd_, messageStr.c_str(), messageStr.size(), 0) < 0)
            perror("send");
        memset(readBuffer, 0, 1024);
        if (recv(commandFd_, readBuffer, sizeof(readBuffer), 0) < 0)
            perror("recieve");
        nlohmann::json recvMessage = json::parse(readBuffer);
        std::cout << recvMessage["errorMessage"] << std::endl;
    }
    return true;
}

void Client::viewUserInfoCommand()
{
    nlohmann::json message;
    isRoomCmd_ = false;
    message["cmd"] = "View user information";
    std::string messageStr = message.dump();
    if (send(commandFd_, messageStr.c_str(), messageStr.size(), 0) < 0)
        perror("send");
    if (recv(commandFd_, readBuffer, sizeof(readBuffer), 0) < 0)
        perror("recieve");
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << "\u2022 User Info:" << std::endl
              << "  ID:        " << recvMessage["id"] << std::endl
              << "  Username:  " << recvMessage["username"] << std::endl
              << "  Password:  " << recvMessage["password"] << std::endl
              << "  Admin:     " << recvMessage["isAdmin"] << std::endl;
    if (!recvMessage["isAdmin"])
    {
        std::cout << "  Phone:     " << recvMessage["phoneNumber"] << std::endl
                  << "  Money:     " << recvMessage["money"] << std::endl
                  << "  Address:   " << recvMessage["address"] << std::endl;
    }
}

bool Client::viewAllUsers()
{
    nlohmann::json message;
    isRoomCmd_ = false;
    message["cmd"] = "View all users";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    std::cout << "\u2022 Users Info:" << std::endl;
    for (auto user : recvMessage["users"])
    {
        std::cout
            << "  ID:        " << user["id"] << std::endl
            << "  Username:  " << user["username"] << std::endl
            << "  Admin:     " << user["isAdmin"] << std::endl;
        if (!user["isAdmin"])
        {
            std::cout << "  Phone:     " << user["phoneNumber"] << std::endl
                      << "  Money:     " << user["money"] << std::endl
                      << "  Address:   " << user["address"] << std::endl;
        }
        std::cout << std::endl;
    }
    return true;
}

bool Client::isTokenSizeCorrect(int tokenSize, int correctNum)
{
    return tokenSize == correctNum;
}

bool Client::viewRoomsInfo()
{
    nlohmann::json message;
    isRoomCmd_ = false;
    std::cout << "which rooms do you want to be shown? (choose the number)" << std::endl
              << "  \u2022 1: Only empty rooms" << std::endl
              << "  \u2022 2: All rooms" << std::endl;
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1")
        message["emptyRooms"] = true;
    else if (choice == "2")
        message["emptyRooms"] = false;
    else
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }

    message["cmd"] = "View rooms information";

    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << "\u2022 Rooms Info: " << std::endl;
    for (auto room : recvMessage["rooms"])
    {
        std::cout << "  Number:         " << room["roomNo"] << std::endl
                  << "  Price:          " << room["price"] << std::endl
                  << "  Max Capacity:   " << room["maxCapacity"] << std::endl
                  << "  Free Capacity:  " << room["freeCapacity"] << std::endl;
        if (room["isAdmin"] && choice == "2")
            for (auto &user : room["users"])
            {
                std::cout << "      \u2022 User: " << std::endl
                          << "      User ID:         " << user["userId"] << std::endl
                          << "      Number of Beds:  " << user["numOfBeds"] << std::endl
                          << "      Reserve Date:    " << user["reserveDate"] << std::endl
                          << "      CheckOut Date:   " << user["checkOutDate"] << std::endl;
            }
        std::cout << std::endl;
    }
    return true;
}

bool Client::booking()
{
    std::cout << "Please book your favorite room by entering command: book <roomNo> <numOfBeds> <checkInDate> <checkOutDate>" << std::endl;
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (!isTokenSizeCorrect(tokens.size(), 5) || tokens[0] != "book")
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false;
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
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    return true;
}

bool Client::canceling()
{
    nlohmann::json sendMsg;
    sendMsg["cmd"] = "show user reserves";
    std::string sendMsgStr = sendMsg.dump();
    send(commandFd_, sendMsgStr.c_str(), sendMsgStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMsg = json::parse(readBuffer);
    if (!recvMsg["isEmpty"])
    {
        std::cout << "\u2022 Reservations: " << std::endl;
        for (auto reserve : recvMsg["user"])
        {
            std::cout << "  RoomNo:  " << reserve["roomNo"] << std::endl
                      << "  ReserveDate:  " << reserve["reserveDate"] << std::endl
                      << "  CheckOutDate:  " << reserve["checkOutDate"] << std::endl
                      << "  NumOfBeds:  " << reserve["numOfBeds"] << std::endl;
            std::cout << std::endl;
        }
        std::cout << "Please enter the roomNo and numOfBeds of your reservation using command: cancel <roomNo> <numOfBeds>" << std::endl;
        std::string command;
        std::getline(std::cin, command);
        std::vector<std::string> tokens = tokenizeCommand(command);
        nlohmann::json message;
        if (!isTokenSizeCorrect(tokens.size(), 3) || tokens[0] != "cancel")
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        isRoomCmd_ = false;
        std::string roomNo = tokens[1];
        std::string numOfBeds = tokens[2];
        if (!checkDigits(roomNo) || !checkDigits(numOfBeds))
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            return false;
        }
        message["cmd"] = "Canceling";
        message["roomNo"] = roomNo;
        message["numOfBeds"] = numOfBeds;
        std::string messageStr = message.dump();
        send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
        memset(readBuffer, 0, 1024);
        recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
        nlohmann::json recvMessage = json::parse(readBuffer);
        if (recvMessage["isError"])
        {
            std::cout << recvMessage["errorMessage"] << std::endl;
            return false;
        }
        std::cout << recvMessage["errorMessage"] << std::endl;
    }

    return true;
}

bool Client::passDay()
{
    std::cout << "Please enter the number of days to pass by enterin command: passDay <numberOfDays>" << std::endl;
    std::string command;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    if (!isTokenSizeCorrect(tokens.size(), 2) || tokens[0] != "passDay")
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
    isRoomCmd_ = false;
    message["cmd"] = "pass day";
    message["daysNo"] = stoi(value);
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    return true;
}

bool Client::editInfo()
{
    nlohmann::json message;
    std::string newPassword, newPhone, newAddress;
    std::cout << "newPassword: ";
    std::getline(std::cin, newPassword);
    std::cout << "newPhone: ";
    std::getline(std::cin, newPhone);
    std::cout << "newAddress: ";
    std::getline(std::cin, newAddress);
    if (!checkDigits(newPhone) || newPhone.length() != 11)
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false;
    message["cmd"] = "Edit information";
    message["newPassWord"] = newPassword;
    message["newPhone"] = newPhone;
    message["newAddress"] = newAddress;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << recvMessage["errorMessage"] << std::endl;
    return true;
}

bool Client::leaveRoom()
{
    std::string command;
    std::cout << "You can leave the room by entering command: room <roomNo>" << std::endl;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (!isTokenSizeCorrect(tokens.size(), 2))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    std::string value = tokens[1];
    if (!checkDigits(value) || tokens[0] != "room")
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false;
    message["cmd"] = "Leaving room";
    message["value"] = value;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    else
        std::cout << recvMessage["errorMessage"] << std::endl;
    return true;
}

bool Client::freeRoom()
{
    std::string command;
    std::cout << "You can make the room empty by entering command: room <roomNo>" << std::endl;
    std::getline(std::cin, command);
    std::vector<std::string> tokens = tokenizeCommand(command);
    nlohmann::json message;
    if (!isTokenSizeCorrect(tokens.size(), 2))
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    std::string value = tokens[1];
    if (!checkDigits(value) || tokens[0] != "room")
    {
        std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
        return false;
    }
    isRoomCmd_ = false;
    message["cmd"] = "free room";
    message["roomNo"] = value;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    else
        std::cout << recvMessage["errorMessage"] << std::endl;
    return true;
}

void Client::roomCommand()
{
    nlohmann::json message;
    std::cout << ROOM_HELP << std::endl;
    message["cmd"] = "Rooms";
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (!recvMessage["isError"])
    {
        isRoomCmd_ = true;
    }
    else if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        isRoomCmd_ = false;
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
    isRoomCmd_ = true;
    message["cmd"] = ADD;
    message["roomNo"] = roomNo;
    message["maxCap"] = maxCap;
    message["price"] = price;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    std::cout << recvMessage["errorMessage"] << std::endl;
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
    isRoomCmd_ = true;
    message["cmd"] = MODIFY;
    message["roomNo"] = roomNo;
    message["newMaxCap"] = newMaxCap;
    message["newPrice"] = price;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    std::cout << recvMessage["errorMessage"] << std::endl;
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
    isRoomCmd_ = true;
    message["cmd"] = REMOVE;
    message["roomNo"] = roomNo;
    std::string messageStr = message.dump();
    send(commandFd_, messageStr.c_str(), messageStr.size(), 0);
    recv(commandFd_, readBuffer, sizeof(readBuffer), 0);
    nlohmann::json recvMessage = json::parse(readBuffer);
    if (recvMessage["isError"])
    {
        std::cout << recvMessage["errorMessage"] << std::endl;
        return false;
    }
    std::cout << recvMessage["errorMessage"] << std::endl;
    return true;
}

void Client::logout()
{
    nlohmann::json message;
    message["cmd"] = "Logout";
    isRoomCmd_ = false;
    hasLoggedIn_ = false;
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
    while (true)
    {
        if (hasLoggedIn_)
            std::cout << HELP << std::endl;
        else
            std::cout << SIGNUP_IN_HELP << std::endl;
        std::getline(std::cin, command);
        tokens = tokenizeCommand(command);
        std::string cmd = tokens[0];
        memset(readBuffer, 0, 1024);
        if (cmd == SIGN_IN && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 3))
            signInCommand(tokens);

        else if (cmd == SIGN_UP && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 2))
        {
            if (!signUpCommand(tokens[1]))
                continue;
        }

        else if (cmd == VIEW_USER_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            viewUserInfoCommand();

        else if (cmd == VIEW_ALL_USERS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
        {
            if (!viewAllUsers())
                continue;
        }

        else if (cmd == VIEW_ROOMS_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
        {
            if (!viewRoomsInfo())
                continue;
        }

        else if (cmd == BOOKING && hasLoggedIn_)
        {
            if (!booking())
                continue;
        }

        else if (cmd == CANCELING && hasLoggedIn_)
        {
            if (!canceling())
                continue;
        }

        else if (cmd == PASS_DAY && hasLoggedIn_)
        {
            if (!passDay())
                continue;
        }

        else if (cmd == EDIT_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
        {
            if (!editInfo())
                continue;
        }

        else if (cmd == LEAVING_ROOM && hasLoggedIn_)
        {
            if (!leaveRoom())
                continue;
        }

        else if (cmd == FREE_ROOM)
        {
            if (!freeRoom())
                continue;
        }

        else if (cmd == ROOMS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
            roomCommand();

        else if (cmd == ADD && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
        {
            if (!addRoom(tokens))
                continue;
        }

        else if (cmd == MODIFY && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
        {
            if (!modifyRoom(tokens))
                continue;
        }

        else if (cmd == REMOVE && isTokenSizeCorrect(tokens.size(), 2) && isRoomCmd_ && hasLoggedIn_)
        {
            if (!removeRoom(tokens[1]))
                continue;
        }

        else if (cmd == LOGOUT && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            logout();

        else
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
    }
}

int main()
{
    Client client;
    client.run();
}
