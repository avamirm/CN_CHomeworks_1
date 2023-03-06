#ifndef __RESERVATION_H__
#define __RESERVATION_H__

#include <string>
class Reservation
{
public:
    Reservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    ~Reservation();
    // int getRoomNo();
    // int getUserNo();
    // int getPrice();
    // int getCheckIn();
    // int getCheckOut();
private:
    int roomNo_;
    int userId_;
    std::string reserveDate_;
    std::string checkInDate_;
    int numOfBeds_;
};

#endif