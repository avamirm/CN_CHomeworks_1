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
        respond = hotel_.viewRooms(isUserAdmin, command["emptyRooms"].get<bool>());
    else if (cmd == BOOKING)
        respond = hotel_.booking(user, command);
    else if (cmd == CANCELING)
        respond = hotel_.canceling(user, command);
    else if (cmd == EDIT_INFORMATION)
        respond = user->editInformation(command);
    else if (cmd == LEAVING_ROOM)
    {
        std::string value = command["value"];
        int value_ = std::stoi(value);
        respond = hotel_.leavingRoom(user, value_);
    }
    else if (cmd == FREE_ROOM)
    {
        std::string roomNo = command["roomNo"];
        int roomN = std::stoi(roomNo);
        respond = hotel_.emptyRoom(roomN, isUserAdmin);
    }
    else if (cmd == PASS_DAY)
        respond = hotel_.passDay(command["daysNo"].get<int>(), isUserAdmin);
    else if (cmd == ROOMS)
        respond = hotel_.editRooms(isUserAdmin);
    else if (cmd == ADD)
        respond = hotel_.addNewRoom(command);
    else if (cmd == MODIFY)
        respond = hotel_.modifyRoom(command);
    else if (cmd == REMOVE)
        respond = hotel_.removeRoom(command);
    else if (cmd == LOGOUT)
        respond = hotel_.logoutUser(user);
    else if (cmd == SHOW_USER_RESERVES)
        respond = hotel_.showUserReserves(user);
    return respond;
}
