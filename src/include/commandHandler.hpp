#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

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
#define Booking "Booking"
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
    void setRooms(json& rooms);
    void setUsers(json& users);

    json runCommand(json command, int userFd);
    // bool doesUsernameExist

private:
    Hotel hotel_;
};

#endif