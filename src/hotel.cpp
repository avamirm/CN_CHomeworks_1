#include "include/hotel.hpp"

Hotel::Hotel()
{
    lastAssignedId_ = -1;
}

Hotel::~Hotel()
{

}

void Hotel::addRooms(json& rooms)
{
    for (auto& room : rooms)
    {
        int roomNo = room["roomNo"];
        int maxCapacity = room["maxCapacity"];
        int price = room["price"];
        bool isFull = room["isFull"];
        int freeCapacity = room["freeCapacity"];
        addRoom(roomNo, maxCapacity, freeCapacity, price, isFull);
        if (freeCapacity != maxCapacity)
        {
            for (auto& user : room["users"])
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

void Hotel::addUsers(json& users)
{
    std::string phoneNumber_;
    std::string address_;
    int money_;
    for (auto& user : users)
    {
        int id = user["id"];
        std::string username = user["username"];
        std::string password = user["password"];
        bool isAdmin = user["isAdmin"];
        lastAssignedId_ = std::max(lastAssignedId_, id);
        if (!isAdmin)
        {
            std::string phoneNumber = user["phoneNumber"];
            std::string  address = user["address"];
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

User* Hotel::findUser(int userFd)
{
    for (auto &user: users_)
    {
        if (user.second.getFd() == userFd)
            return &user.second;
    }
    return nullptr;
}

json Hotel::viewAllUsers(User* user)
{
    json usersInfo;
    usersInfo["isError"] = false;
    usersInfo["errorMessage"] = "";
    usersInfo["users"] = json::array();
    if (user->isAdmin())
    {
        for (auto user: users_)
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
    
    for (auto room: rooms_)
    {
        json roomInfo;
        roomInfo["roomNo"] = room.first;
        roomInfo["price"] = room.second.getPrice();
        roomInfo["maxCapacity"] = room.second.getMaxCapacity();
        roomInfo["freeCapacity"] = room.second.getFreeCapacity();
        roomInfo["users"] = json::array();
        if (isUserAdmin && room.second.getIsFull() == false)
        {
            for (auto& reserve: reservations_[room.first])
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
