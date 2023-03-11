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
    for (auto &room : rooms)
    {
        int roomNo = room["roomNo"];
        int maxCapacity = room["maxCapacity"];
        int price = room["price"];
        bool isFull = room["isFull"];
        int freeCapacity = room["freeCapacity"];
        addRoom(roomNo, maxCapacity, freeCapacity, price, isFull);
        if (freeCapacity != maxCapacity)
        {
            for (auto &user : room["users"])
            {
                int userId = user["id"];
                std::string reserveDate = user["reserveDate"];
                std::string checkOutDate = user["checkOutDate"];
                int numOfBeds = user["numOfBeds"];
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
    for (auto &user : users)
    {
        int id = user["id"];
        std::string username = user["username"];
        std::string password = user["password"];
        bool isAdmin = user["isAdmin"];
        lastAssignedId_ = std::max(lastAssignedId_, id);
        if (!isAdmin)
        {
            std::string phoneNumber = user["phoneNumber"];
            std::string address = user["address"];
            int money = user["money"];
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

User *Hotel::findUser(std::string username, std::string password)
{
    for (auto user : users_)
    {
        if (user.second.getName() == username && user.second.getPassword() == password)
            return &user.second;
    }
    return nullptr;
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

json Hotel::viewRooms(bool isUserAdmin)
{
    json roomsInfo;

    roomsInfo = json::array();

    for (auto room : rooms_)
    {
        json roomInfo;
        roomInfo["roomNo"] = room.first;
        roomInfo["price"] = room.second.getPrice();
        roomInfo["maxCapacity"] = room.second.getMaxCapacity();
        roomInfo["freeCapacity"] = room.second.getFreeCapacity();
        roomInfo["users"] = json::array();
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
        roomsInfo.push_back(roomInfo);
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
                if (roomUSer.getRoomNo() == value)
                    respond["errorMessage"] = SUCCESSFULLY_LEAVING;
                else
                    respond["errorMessage"] = RESERVE_NOT_FOUND;
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

json Hotel::changeCapcity(int value, int capacity, bool isUserAdmin)
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
    int roomNo = stoi(command["roomNum"]);
    int maxCapacity = stoi(command["maxCap"]);
    int price = stoi(command["price"]);
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == roomNo)
        {
            respond["errorMessage"] = ROOM_EXIST;
            return respond
        }
    }
    addRoom(stoi(roomNo), stoi(maxCapacity), stoi(maxCapacity), stoi(price), false);
    respond["errorMessage"] = SUCCESSFULLY_ADDED;
    return respond;
}

json Hotel::modifyRoom(json command)
{
    json respond;
    respond["isError"] = true;
    int roomNo = stoi(command["roomNum"]);
    int maxCapacity = stoi(command["newMaxCap"]);
    int price = stoi(command["newPrice"]);
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == roomNo)
        {
            if (room.second.getIsFull() == true && room.second.getMaxCapacity() < stoi(maxCapacity))
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            room.second.setMaxCapacity(stoi(maxCapacity));
            room.second.setPrice(stoi(price));
            respond["errorMessage"] = SUCCESSFULLY_MODIFIED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
}

json Hotel::removeRoom(json command)
{
    json respond;
    respond["isError"] = true;
    int roomNo = stoi(command["roomNum"]);
    for (auto room : rooms_)
    {
        if (room.second.getRoomNo() == roomNo)
        {
            if (room.second.getIsFull() == true)
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            rooms_.erase(roomNo);
            respond["errorMessage"] = SUCCESSFULLY_REMOVED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
}