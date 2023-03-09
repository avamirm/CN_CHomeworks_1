#include "include/reservation.hpp"

Reservation::Reservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds)
{
    roomNo_ = roomNo;
    userId_ = userId;
    reserveDate_ = reserveDate;
    checkOutDate_ = checkOutDate;
    numOfBeds_ = numOfBeds;
}

Reservation::~Reservation()
{
}

int Reservation::getNumOfBeds()
{
    return numOfBeds_;
}

std::string Reservation::getCheckOutDate()
{
    return checkOutDate_;
}

std::string Reservation::getReserveDate()
{
    return reserveDate_;
}
// int Reservation::getRoomNo()
// {
//     return roomNo_;
// }

int Reservation::getUserId()
{
    return userId_;
}