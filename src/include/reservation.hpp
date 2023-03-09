#ifndef RESERVATION_HPP
#define RESERVATION_HPP

#include <string>
#include "date.hpp"
class Reservation
{
public:
    Reservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    ~Reservation();
    // int getRoomNo();
    int getUserId();
    std::string getReserveDate();
    std::string getCheckOutDate();
    int getNumOfBeds();
    // int getCheckIn();
    // int getCheckOut();
private:
    int roomNo_;
    int userId_;
    std::string reserveDate_;
    std::string checkOutDate_;
    int numOfBeds_;
};

#endif