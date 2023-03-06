#ifndef __HOTEL_H__
#define __HOTEL_H__

#include "Reservation.hpp"
#include "Room.hpp"
#include "User.hpp"
#include <vector>
#include <unordered_map>

class Hotel 
{
public:
    Hotel();
    ~Hotel();
    void addRoom(int roomNo, int maxCapacity, int price);
    void setDate(std::string& date);
    void addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    bool isDateValid(std::string& date);
    User* findUser(int userSocket);
private:
    std::vector<User> users_;
    std::unordered_map<int, Reservation> reservations_;
    std::unordered_map<int, Room> rooms_;
    std::string date_;
};
#endif