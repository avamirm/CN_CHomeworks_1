#include "Hotel.hpp"

Hotel::Hotel()
{

}

Hotel::~Hotel()
{

}

void Hotel::addRoom(int roomNo, int maxCapacity, int price)
{
    Room room(roomNo, maxCapacity, price);
    rooms_[roomNo] = room;
}

void Hotel::setDate(std::string& date)
{
    date_ = date;
}

void Hotel::addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds)
{
    Reservation reservation(roomNo, userId, reserveDate, checkOutDate, numOfBeds);
    reservations_[roomNo] = reservation;
}

bool Hotel::isDateValid(std::string& date)
{
    // for(auto& reservation : reservations_)
    // {
    //     if(reservation.second.getCheckIn() == date)
    //     {
    //         return false;
    //     }
    // }
}

User* Hotel::findUser(int userSocket)
{
    for (auto& user: users_)
    {
        if (user.getSocket() == userSocket)
            return &user;
    }
    return nullptr;
}

