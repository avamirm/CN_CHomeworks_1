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
    respond["isError"] = true;
    respond["errorMessage"] = errMessage;
    return respond;
}

json CommandHandler::runCommand(json command, int userFd)
{
    json respond;
    std::string cmd = command["cmd"];
    User *user = hotel_.findUser(userFd);
    User *userByName = hotel_.findUSer(command["username"], command["password"]);

    if (userByName == nullptr)
    {
        if (cmd == SIGN_IN)
            return isUserExist(INVALID_USERNAME_OR_PASSWORD);
        else if (cmd == SIGN_UP)
            return isUserExist(USER_SIGNED_UP)
    }

    bool isUserAdmin = user->isAdmin();

    if (cmd == SIGN_IN)
        respond["errorMessage"] = USER_LOGGED_IN;

    else if (cmd == SIGN_UP)
        respond['errorMessage'] = USER_EXISTED;

    else if (cmd == SUCCESS_SIGN_UP)
    {
        respond = hotel_.addUSer(command);
        respond['errorMessage'] = USER_SUCCESSFULLY_SIGN_UP;
    }

    else if (cmd == VIEW_USER_INFORMATION)
        respond = user->viewInfo(true);
    else if (cmd == VIEW_ALL_USERS)
        respond = hotel_.viewAllUsers(user);
    else if (cmd == VIEW_ROOMS_INFORMATION)
        respond = hotel_.viewRooms(isUserAdmin);
    // else if (cmd == BOOKING)
    else if (cmd == EDIT_INFORMATION)
        response = user->editInformation(command);
    else if (cmd == LEAVING_ROOM)
        respond = hotel_.leavingRoom(user, stoi(command["value"]));
    else if (cmd == CAPACITY)
        respond = hotel_.changeCapacity(stoi(command["value"]), stoi(command["newCap"]), isUserAdmin);
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
