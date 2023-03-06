#include "room.hpp"

Room::Room(int roomNo, int maxCapacity, int price)
{
    roomNo = roomNo;
    maxCapacity_ = maxCapacity;
    price_ = price;
    freeCapacity_ = maxCapacity;
    isFull_ = false;
}

int Room::getRoomNo()
{
    return roomNo;
}

int Room::getPrice()
{
    return price_;
}