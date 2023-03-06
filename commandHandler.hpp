#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__

#include "hotel.hpp"

#define VIEW_USER_INFORMATION "1"
#define VIEW_ALL_USERS "2"
#define VIEW_ROOMS_INFORMATION "3"
#define BOOKING "4"
#define CANCELING "5"
#define PASS_DAY "6"
#define EDIT_INFORMATION "7"
#define LEAVING_ROOM "8"
#define ROOMS "9"
#define LOGOUT "0"
// enum class Command
// {
//     VIEW_USER_INFORMATION,
//     VIEW_ALL_USERS,
//     VIEW_ROOMS_INFORMATION,
//     BOOKING,
//     CANCELING,
//     PASS_DAY,
//     EDIT_INFORMATION,
//     LEAVING_ROOM,
//     ROOMS,
//     LOGOUT,
// };
class CommandHandler
{
public:
    CommandHandler();
    ~CommandHandler();
    std::string runCommand(std::string& command);
    // bool doesUsernameExist

private:
    Hotel hotel;
}

#endif