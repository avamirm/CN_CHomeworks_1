#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <string>

#include "hotel.hpp"
#include "errors.hpp"

#define VIEW_USER_INFORMATION "View user information"
#define VIEW_ALL_USERS "View all users"
#define VIEW_ROOMS_INFORMATION "View rooms information"
#define BOOKING "Booking"
#define CANCELING "Canceling"
#define PASS_DAY "pass day"
#define EDIT_INFORMATION "Edit information"
#define LEAVING_ROOM "Leaving room"
#define ROOMS "Rooms"
#define LOGOUT "Logout"

#define ADD "add"
#define MODIFY "modify"
#define REMOVE "remove"
#define CAPACITY "capacity"

#define SIGN_IN "signin"
#define SIGN_UP "signup"
#define SUCCESS_SIGN_UP "SuccessSignup"

// #define USER_SUCCESSFULLY_SIGN_UP "Error 231: User successfully signed up."
// #define USER_SIGNED_UP "Error 311: User Signed up. Enter your password, purse, phone and address."
// #define INVALID_USERNAME_OR_PASSWORD "Error 430: Invalid username or password."
// #define USER_EXISTED "Error 451: User already existed!"

class CommandHandler
{
public:
    CommandHandler();
    void setRooms(json &rooms);
    void setUsers(json &users);
    json isUserExist(std::string errMessage);
    json runCommand(json command, int userFd);
    void setDate(date::year_month_day date);
    // bool doesUsernameExist

private:
    Hotel hotel_;
};

#endif