#include "include/commandHandler.hpp"

CommandHandler::CommandHandler()
{
    Hotel hotel;
    hotel_ = hotel;
}

void CommandHandler::setRooms(json &rooms)
{
    hotel_.addRooms(rooms);
}

void CommandHandler::setUsers(json &users)
{
    hotel_.addUsers(users);
}

void CommandHandler::setDate(date::year_month_day date)
{
    hotel_.setDate(date);
}

json CommandHandler::isUserExist(std::string errMessage)
{
    json respond;
    if (errMessage == INVALID_USERNAME_OR_PASSWORD)
    {
        respond["isError"] = true;
        respond["errorMessage"] = errMessage;
    }
    else if (errMessage == USER_SIGNED_UP)
    {
        respond["isError"] = false;
        respond["errorMessage"] = errMessage;
    }
    return respond;
}

json CommandHandler::runCommand(json command, int userFd)
{
    json respond;
    std::string cmd = command["cmd"].get<std::string>();
    User *user = hotel_.findUser(userFd);

    bool isUserAdmin = false;
    if (user != nullptr)
        isUserAdmin = user->isAdmin();      /////////////////////////////////////////

    if (cmd == SIGN_IN)
        respond = hotel_.findUserByName(command["username"].get<std::string>(), command["password"].get<std::string>(), userFd);

    else if (cmd == SIGN_UP)
        respond = hotel_.checkUsernameExistance(command["username"].get<std::string>());

    else if (cmd == SUCCESS_SIGN_UP)
        respond = hotel_.addUser(command);

    else if (cmd == VIEW_USER_INFORMATION)
        respond = user->viewInfo(true);
    else if (cmd == VIEW_ALL_USERS)
        respond = hotel_.viewAllUsers(user);
    else if (cmd == VIEW_ROOMS_INFORMATION)
        respond = hotel_.viewRooms(isUserAdmin);
    // else if (cmd == BOOKING)
    else if (cmd == EDIT_INFORMATION)
        respond = user->editInformation(command);
    else if (cmd == LEAVING_ROOM)
    {
        std::string value = command["value"];
        int value_ = std::stoi(value);
        respond = hotel_.leavingRoom(user, value_);
    }
    else if (cmd == CAPACITY)
    {
        std::string value = command["value"];
        std::string newCap = command["newCap"];
        int value_ = std::stoi(value);
        int newCap_ = std::stoi(newCap);
        respond = hotel_.changeCapacity(value_, newCap_, isUserAdmin);
    }
    else if (cmd == ROOMS)
        respond = hotel_.editRooms(isUserAdmin);
    else if (cmd == ADD)
        respond = hotel_.addNewRoom(command);
    else if (cmd == MODIFY)
        respond = hotel_.modifyRoom(command);
    else if (cmd == REMOVE)
        respond = hotel_.removeRoom(command);
    return respond;
}
