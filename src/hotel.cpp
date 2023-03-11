#include "include/hotel.hpp"

Hotel::Hotel()
{
    lastAssignedId_ = -1;
}

Hotel::~Hotel()
{
}

void Hotel::addRooms(json &rooms)
{
    for (auto room : rooms["rooms"])
    {
        int roomNo = room["roomNo"].get<int>();
        int maxCapacity = room["maxCapacity"].get<int>();
        int price = room["price"].get<int>();
        bool isFull = room["isFull"].get<bool>();
        int freeCapacity = room["freeCapacity"].get<int>();
        addRoom(roomNo, maxCapacity, freeCapacity, price, isFull);
        if (freeCapacity != maxCapacity)
        {
            for (auto &user : room["users"])
            {
                int userId = user["id"].get<int>();
                std::string reserveDate = user["reserveDate"].get<std::string>();
                std::string checkOutDate = user["checkOutDate"].get<std::string>();
                int numOfBeds = user["numOfBeds"].get<int>();
                addReservation(roomNo, userId, reserveDate, checkOutDate, numOfBeds);
            }
        }
    }
}

void Hotel::addUsers(json &users)
{
    std::string phoneNumber_;
    std::string address_;
    int money_;
    for (auto user : users["users"])
    {
        int id = user["id"].get<int>();
        std::string username = user["username"].get<std::string>();
        std::string password = user["password"].get<std::string>();
        bool isAdmin = user["isAdmin"].get<bool>();
        lastAssignedId_ = std::max(lastAssignedId_, id);
        if (!isAdmin)
        {
            std::string phoneNumber = user["phoneNumber"].get<std::string>();
            std::string address = user["address"].get<std::string>();
            int money = user["money"].get<int>();
            User newUser = User(id, username, password, isAdmin, phoneNumber, address, money);
            users_.insert({id, newUser});
        }
        else
        {
            User newUser = User(id, username, password, isAdmin, "", "", -1);
            users_.insert({id, newUser});
        }
    }
}

json Hotel::addUser(json user)
{
    json respond;
    respond["isError"] = false;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    lastAssignedId_ += 1;
    int id = lastAssignedId_;
    std::string username = user["username"].get<std::string>();
    std::string password = user["password"].get<std::string>();
    // lastAssignedId_ = std::max(lastAssignedId_, id);
    std::string phoneNumber = user["phoneNumber"].get<std::string>();
    std::string address = user["address"].get<std::string>();
    int money = stoi(user["money"].get<std::string>());
    User newUser = User(id, username, password, false, phoneNumber, address, money);
    users_.insert({id, newUser});
    respond["errorMessage"] = USER_SUCCESSFULLY_SIGN_UP;
    return respond;
}

void Hotel::addRoom(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull)
{
    Room room = Room(roomNo, maxCapacity, freeCapacity, price, isFull);
    rooms_.insert({roomNo, room});
}

void Hotel::addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds)
{
    Reservation reservation = Reservation(roomNo, userId, reserveDate, checkOutDate, numOfBeds);
    reservations_[roomNo].push_back(reservation);
}

// bool Hotel::isDateValid(std::string& date)
// {
//     // for(auto& reservation : reservations_)
//     // {
//     //     if(reservation.second.getCheckIn() == date)
//     //     {
//     //         return false;
//     //     }
//     // }
// }

User *Hotel::findUser(int userFd)
{
    for (auto &user : users_)
    {
        if (user.second.getFd() == userFd)
            return &user.second;
    }
    return nullptr;
}

json Hotel::findUserByName(std::string username, std::string password, int userFd)
{
    json response;
    response["isError"] = "true";
    response["errorMessage"] = INVALID_USERNAME_OR_PASSWORD;
    for (auto &user : users_)
    {
        if (user.second.getName() == username && user.second.getPassword() == password)
        {
            setUserFd(&user.second, userFd);
            response["isError"] = false;
            response["errorMessage"] = USER_LOGGED_IN;
        }
    }
    return response;
}

json Hotel::viewAllUsers(User *user)
{
    json usersInfo;
    usersInfo["isError"] = false;
    usersInfo["errorMessage"] = "";
    usersInfo["users"] = json::array();
    if (user->isAdmin())
    {
        for (auto user : users_)
            usersInfo["users"].push_back(user.second.viewInfo(false));
    }
    else
    {
        usersInfo["isError"] = true;
        usersInfo["errorMessage"] = ACCESS_DENIED;
    }
    return usersInfo;
}

json Hotel::checkUsernameExistance(std::string username)
{
    json response;
    response["isError"] = false;
    response["errorMessage"] = USER_SIGNED_UP;
    for (auto user: users_)
    {
        if (user.second.getName() == username)
        {
            response["isError"] = true;
            response["errorMessage"] = USER_EXISTED;
        }
    }
    return response;
}

json Hotel::viewRooms(bool isUserAdmin)
{
    json roomsInfo;

    roomsInfo["rooms"] = json::array();

    for (auto room : rooms_)
    {
        json roomInfo;
        roomInfo["roomNo"] = room.first;
        roomInfo["price"] = room.second.getPrice();
        roomInfo["maxCapacity"] = room.second.getMaxCapacity();
        roomInfo["freeCapacity"] = room.second.getFreeCapacity();
        roomInfo["users"] = json::array();
        roomInfo["isAdmin"] = isUserAdmin;
        if (isUserAdmin && room.second.getIsFull() == false)
        {
            for (auto &reserve : reservations_[room.first])
            {
                json usersInfo;
                usersInfo["userId"] = reserve.getUserId();
                usersInfo["username"] = users_.find(usersInfo["userId"])->second.getName();
                usersInfo["numOfBeds"] = reserve.getNumOfBeds();
                usersInfo["reserveDate"] = reserve.getReserveDate();
                usersInfo["checkOutDate"] = reserve.getCheckOutDate();
                roomInfo["users"].push_back(usersInfo);
            }
        }
        roomsInfo["rooms"].push_back(roomInfo);
    }
    return roomsInfo;
}

void Hotel::setDate(date::year_month_day date)
{
    date_ = date;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
json Hotel::leavingRoom(User *user, int value)
{
    json respond;
    respond["isError"] = true;
    for (auto reserve : reservations_)
    {
        for (auto roomUser : reserve.second)
        {
            if (roomUser.getUserId() == user->getId())
            {
                if (roomUser.getRoomNo() == value)
                {
                    respond["isError"] = false;
                    respond["errorMessage"] = SUCCESSFULLY_LEAVING;
                }
                else
                    respond["errorMessage"] = RESERVE_NOT_FOUND;
                return respond;
            }
        }
    }
    respond["errorMessage"] = RESERVE_NOT_FOUND;
    return respond;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

json Hotel::changeCapacity(int value, int capacity, bool isUserAdmin)
{
    json response;
    response["isError"] = true;
    if (!isUserAdmin)
    {
        response["errorMessage"] = ACCESS_DENIED;
        return response;
    }
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == value)
        {
            if (room.second.getMaxCapacity() < capacity)
            {
                response["errorMessage"] = INVALID_CAPACITY;
                return response;
            }
            else
            {
                response["isError"] = false;
                room.second.setMaxCapacity(capacity);
                response["errorMessage"] = SUCCESSFULLY_LEAVING;
                return response;
            }
        }
    }
    response["errorMessage"] = ROOM_NOT_FOUND;
    return response;
}

json Hotel::editRooms(bool isUserAdmin)
{
    json editedRooms;
    if (!isUserAdmin)
    {
        editedRooms["isError"] = true;
        editedRooms["errorMessage"] = ACCESS_DENIED;
    }
    else
    {
        editedRooms["isError"] = false;
    }
    return editedRooms;
}

json Hotel::addNewRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNum"];
    std::string maxCapacity = command["maxCap"];
    std::string price = command["price"];
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == std::stoi(roomNo))
        {
            respond["errorMessage"] = ROOM_EXIST;
            return respond;
        }
    }
    addRoom(std::stoi(roomNo), std::stoi(maxCapacity), std::stoi(maxCapacity), std::stoi(price), false);
    respond["isError"] = false;
    respond["errorMessage"] = SUCCESSFULLY_ADDED;
    return respond;
}

json Hotel::modifyRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNum"];
    std::string maxCapacity = command["newMaxCap"];
    std::string price = command["newPrice"];
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == std::stoi(roomNo))
        {
            if (room.second.getIsFull() == true && room.second.getMaxCapacity() < std::stoi(maxCapacity))
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            room.second.setMaxCapacity(std::stoi(maxCapacity));
            room.second.setPrice(std::stoi(price));
            respond["isError"] = false;
            respond["errorMessage"] = SUCCESSFULLY_MODIFIED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
    return respond;
}

void Hotel::setUserFd(User* user, int userFd)
{
    user->setFd(userFd);
}

json Hotel::removeRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNum"];
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == std::stoi(roomNo))
        {
            if (room.second.getIsFull() == true)
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            rooms_.erase(std::stoi(roomNo));
            respond["isError"] = false;
            respond["errorMessage"] = SUCCESSFULLY_DELETED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
    return respond;
}