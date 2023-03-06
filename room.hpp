#ifndef __ROOM_H__
#define __ROOM_H__

#include <string>
#include <vector>
#include "User.hpp"

class Room
{
public:
    Room(int roomNo, int maxCapacity, int price);
    ~Room();
    int getRoomNo();
    // int getMaxUser();
    int getPrice();

private:
    int roomNo;
    int maxCapacity_;
    int price_;
    int freeCapacity_;
    bool isFull_;
};
#endif