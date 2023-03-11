#include "include/room.hpp"

Room::Room(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull)
{
    roomNo_ = roomNo;
    maxCapacity_ = maxCapacity;
    price_ = price;
    freeCapacity_ = freeCapacity;
    isFull_ = isFull;
}

Room::~Room()
{
}

int Room::getRoomNo()
{
    return roomNo_;
}

int Room::getPrice()
{
    return price_;
}

int Room::getMaxCapacity()
{
    return maxCapacity_;
}

int Room::getFreeCapacity()
{
    return freeCapacity_;
}

bool Room::getIsFull()
{
    return isFull_;
}

void Room::setFreeCapacity(int freeCapacity)
{
    freeCapacity_ = freeCapacity;
}

void Room::setPrice(int price)
{
    price_ = price;
}