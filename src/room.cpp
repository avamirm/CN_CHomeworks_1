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
    if (maxCapacity_ > freeCapacity)
        freeCapacity_ -= maxCapacity_ - freeCapacity;
    else
        freeCapacity_ += freeCapacity - maxCapacity_;
}

void Room::setPrice(int price)
{
    price_ = price;
}

void Room::setMaxCapacity(int maxCapacity)
{
    maxCapacity_ = maxCapacity;
}

void Room::makeEmpty()
{
    isFull_ = false;
    freeCapacity_ = maxCapacity_;
}

void Room::increaseSpace(int numOfGonePeople)
{
    freeCapacity_ += numOfGonePeople;
    isFull_ = false;
}

void Room::fill(int numOfBeds)
{
    freeCapacity_ -= numOfBeds;
    if (freeCapacity_ == 0)
        isFull_ = true;
}