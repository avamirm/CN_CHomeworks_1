#ifndef HOTEL_HPP
#define HOTEL_HPP

#include "reservation.hpp"
#include "room.hpp"
#include "user.hpp"
#include <vector>
#include <unordered_map>

#define ACCESS_DENIED "Error 403: Access Denied"
#define ROOM_NOT_FOUND "Error 101: The desired room was not found"
#define LOW_BALANCE "Error 108: Your account balance is not enough"
#define ROOM_IS_FULL "Error 109: The room capacity is full"
#define RESERVE_NOT_FOUND "Error 102: Your reservation was not found"

class Hotel 
{
public:
    Hotel();
    ~Hotel();
    void addRoom(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull);
    void addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    // bool isDateValid(std::string& date);
    User* findUser(int userFd);
    json viewAllUsers(User* user);
    json viewRooms(bool isUserAdmin);
    void addRooms(json& rooms);
    void addUsers(json& users);
    void setDate(date::year_month_day date);
private:
    std::unordered_map<int, User> users_;
    std::unordered_map<int, std::vector<Reservation>> reservations_;
    std::unordered_map<int, Room> rooms_;
    date::year_month_day date_;
    int lastAssignedId_;
};
#endif