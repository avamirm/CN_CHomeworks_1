#include "include/commandHandler.hpp"

CommandHandler::CommandHandler()
{
    Hotel hotel;
    hotel_ = hotel;
}

void CommandHandler::setRooms(json& rooms)
{
    hotel_.addRooms(rooms);
}

void CommandHandler::setUsers(json& users)
{
    hotel_.addUsers(users);
}

void CommandHandler::setDate(date::year_month_day date)
{
    hotel_.setDate(date);
}

json CommandHandler::runCommand(json command, int userFd)
{
    json respond;
    std::string cmd = command["cmd"];
    User* user = hotel_.findUser(userFd);
    bool isUserAdmin = user->isAdmin();
    if (cmd == VIEW_USER_INFORMATION)
        respond = user->viewInfo(true);
    else if (cmd == VIEW_ALL_USERS)
        respond = hotel_.viewAllUsers(user);
    else if (cmd == VIEW_ROOMS_INFORMATION)
        respond = hotel_.viewRooms(isUserAdmin);
   // else if (cmd == BOOKING)

    



    return respond;
}
