#include "reservation.hpp"

Reservation::Reservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds)
{
    roomNo_ = roomNo;
    userId_ = userId;
    reserveDate_ = reserveDate;
    checkInDate_ = checkOutDate;
    numOfBeds_ = numOfBeds;
}

Reservation::~Reservation()
{
}

// int Reservation::getRoomNo()
// {
//     return roomNo_;
// }