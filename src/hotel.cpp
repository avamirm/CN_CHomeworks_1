#include "include/hotel.hpp"

Hotel::Hotel()
{
    lastAssignedId_ = -1;
}

Hotel::~Hotel()
{
}

void Hotel::addRooms(json &rooms)
{
    for (auto room : rooms["rooms"])
    {
        int roomNo = room["roomNo"].get<int>();
        int maxCapacity = room["maxCapacity"].get<int>();
        int price = room["price"].get<int>();
        bool isFull = room["isFull"].get<bool>();
        int freeCapacity = room["freeCapacity"].get<int>();
        addRoom(roomNo, maxCapacity, freeCapacity, price, isFull);
        if (freeCapacity != maxCapacity)
        {
            for (auto &user : room["users"])
            {
                int userId = user["id"].get<int>();
                std::string reserveDate = user["reserveDate"].get<std::string>();
                std::string checkOutDate = user["checkOutDate"].get<std::string>();
                int numOfBeds = user["numOfBeds"].get<int>();
                addReservation(roomNo, userId, reserveDate, checkOutDate, numOfBeds);
            }
        }
    }
}

void Hotel::addUsers(json &users)
{
    std::string phoneNumber_;
    std::string address_;
    int money_;
    for (auto user : users["users"])
    {
        int id = user["id"].get<int>();
        std::string username = user["username"].get<std::string>();
        std::string password = user["password"].get<std::string>();
        bool isAdmin = user["isAdmin"].get<bool>();
        lastAssignedId_ = std::max(lastAssignedId_, id);
        if (!isAdmin)
        {
            std::string phoneNumber = user["phoneNumber"].get<std::string>();
            std::string address = user["address"].get<std::string>();
            int money = user["money"].get<int>();
            User newUser = User(id, username, password, isAdmin, phoneNumber, address, money);
            users_.insert({id, newUser});
        }
        else
        {
            User newUser = User(id, username, password, isAdmin, "", "", -1);
            users_.insert({id, newUser});
        }
    }
}

json Hotel::addUser(json user)
{
    json respond;
    respond[IS_ERROR] = false;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    lastAssignedId_ += 1;
    int id = lastAssignedId_;
    std::string username = user["username"].get<std::string>();
    std::string password = user["password"].get<std::string>();
    // lastAssignedId_ = std::max(lastAssignedId_, id);
    std::string phoneNumber = user["phoneNumber"].get<std::string>();
    std::string address = user["address"].get<std::string>();
    int money = stoi(user["money"].get<std::string>());
    User newUser = User(id, username, password, false, phoneNumber, address, money);
    users_.insert({id, newUser});
    respond[ERROR_MSG] = USER_SUCCESSFULLY_SIGN_UP;
    return respond;
}

void Hotel::addRoom(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull)
{
    Room room = Room(roomNo, maxCapacity, freeCapacity, price, isFull);
    rooms_.insert({roomNo, room});
}

void Hotel::addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds)
{
    Reservation reservation = Reservation(roomNo, userId, reserveDate, checkOutDate, numOfBeds);
    reservations_[roomNo].push_back(reservation);
}

User *Hotel::findUser(int userFd)
{
    for (auto &user : users_)
    {
        if (user.second.getFd() == userFd)
            return &user.second;
    }
    return nullptr;
}

json Hotel::findUserByName(std::string username, std::string password, int userFd)
{
    json response;
    response[IS_ERROR] = true;
    response[ERROR_MSG] = INVALID_USERNAME_OR_PASSWORD;
    for (auto &user : users_)
    {
        if (user.second.getName() == username && user.second.getPassword() == password)
        {
            setUserFd(&user.second, userFd);
            response[IS_ERROR] = false;
            response[ERROR_MSG] = USER_LOGGED_IN;
        }
    }
    return response;
}

json Hotel::viewAllUsers(User *user)
{
    json usersInfo;
    usersInfo[IS_ERROR] = false;
    usersInfo[ERROR_MSG] = "";
    usersInfo["users"] = json::array();
    if (user->isAdmin())
    {
        for (auto user : users_)
            usersInfo["users"].push_back(user.second.viewInfo(false));
    }
    else
    {
        usersInfo[IS_ERROR] = true;
        usersInfo[ERROR_MSG] = ACCESS_DENIED;
    }
    return usersInfo;
}

json Hotel::checkUsernameExistance(std::string username)
{
    json response;
    response[IS_ERROR] = false;
    response[ERROR_MSG] = USER_SIGNED_UP;
    for (auto user: users_)
    {
        if (user.second.getName() == username)
        {
            response[IS_ERROR] = true;
            response[ERROR_MSG] = USER_EXISTED;
        }
    }
    return response;
}

json Hotel::viewRooms(bool isUserAdmin, bool isEmptyRoom)
{
    json roomsInfo;

    roomsInfo["rooms"] = json::array();
    for (auto room : rooms_)
    {
        if (!isEmptyRoom || (isEmptyRoom && !room.second.getIsFull()))
        {
            json roomInfo;
            roomInfo["roomNo"] = room.first;
            roomInfo["price"] = room.second.getPrice();
            roomInfo["maxCapacity"] = room.second.getMaxCapacity();
            roomInfo["freeCapacity"] = room.second.getFreeCapacity();
            roomInfo["users"] = json::array();
            roomInfo["isAdmin"] = isUserAdmin;
            if (isUserAdmin && room.second.getFreeCapacity() != room.second.getMaxCapacity())
            {
                for (auto &reserve : reservations_[room.first])
                {
                    json usersInfo;
                    usersInfo["userId"] = reserve.getUserId();
                    usersInfo["username"] = users_.find(usersInfo["userId"])->second.getName();
                    usersInfo["numOfBeds"] = reserve.getNumOfBeds();
                    usersInfo["reserveDate"] = reserve.getReserveDate();
                    usersInfo["checkOutDate"] = reserve.getCheckOutDate();
                    roomInfo["users"].push_back(usersInfo);
                }
            }
            roomsInfo["rooms"].push_back(roomInfo);
        }
    }
    return roomsInfo;
}

void Hotel::setDate(date::year_month_day date)
{
    date_ = date;
}

bool Hotel::doesRoomExist(int roomNo)
{
    for (auto room: rooms_)
        if (room.first == roomNo)
            return true;
    return false;
}

json Hotel::leavingRoom(User *user, int roomNo)
{
    json response;
    response[IS_ERROR] = true;
    if (!doesRoomExist(roomNo))
    {
        response[ERROR_MSG] = BAD_SEQUENCE_OF_COMMANDS;
        return response;
    }
    auto itr = reservations_.find(roomNo);
    if (itr != reservations_.end())
    {
        std::vector < Reservation> resCpy (reservations_[roomNo].begin(), reservations_[roomNo].end());
        for (int i=resCpy.size()-1; i >= 0; i--)
        {
            date::year_month_day checkOutDate = convertDate(resCpy[i].getCheckOutDate());
            if (resCpy[i].getUserId() == user->getId() && checkOutDate >= date_) 
            {
                reservations_[roomNo].erase(reservations_[roomNo].begin() + i);
                response[IS_ERROR] = false;
                response[ERROR_MSG] = SUCCESSFULLY_LEAVING;
                updateRoomsFile();
                return response;
            }
        }
    }
    response[ERROR_MSG] = RESERVE_NOT_FOUND;
    return response;
}
date::year_month_day Hotel::convertDate(std::string date)
{
    std::istringstream ss(date);
    date::year_month_day resultDate;
    ss >> date::parse("%F", resultDate);
    return resultDate;
}

json Hotel::emptyRoom(int roomNo, bool isUserAdmin)
{
    json response;
    
    response[IS_ERROR] = true;
    if (!isUserAdmin)
    {
        response[ERROR_MSG] = ACCESS_DENIED;
        return response;
    }
    if (!doesRoomExist(roomNo))
    {
        response[ERROR_MSG] = ROOM_NOT_FOUND;
        return response;
    }
    for (auto room : rooms_)
    {
        if (room.first == roomNo)
            room.second.makeEmpty();
    }
    auto itr = reservations_.find(roomNo);
    if (itr != reservations_.end())
    {
        std::vector < Reservation> resCpy (reservations_[roomNo].begin(), reservations_[roomNo].end());
        for (int i=resCpy.size()-1; i >= 0; i--)    
        {
            date::year_month_day reserveDate = convertDate(reservations_[roomNo][i].getReserveDate());
            date::year_month_day checkOutDate = convertDate(reservations_[roomNo][i].getCheckOutDate());
            if (checkOutDate >= date_ && reserveDate <= date_)
                reservations_[roomNo].erase(reservations_[roomNo].begin() + i);
        }
    }

    response[IS_ERROR] = false;
    response[ERROR_MSG] = SUCCESSFULLY_LEAVING;
    updateRoomsFile();
    return response;
}

json Hotel::editRooms(bool isUserAdmin)
{
    json editedRooms;
    if (!isUserAdmin)
    {
        editedRooms[IS_ERROR] = true;
        editedRooms[ERROR_MSG] = ACCESS_DENIED;
    }
    else
    {
        editedRooms[IS_ERROR] = false;
    }
    return editedRooms;
}

json Hotel::addNewRoom(json command)
{
    json respond;
    respond[IS_ERROR] = true;
    std::string roomNo = command["roomNo"];
    std::string maxCapacity = command["maxCap"];
    std::string price = command["price"];
    if (doesRoomExist(std::stoi(roomNo)))
    {
        respond[ERROR_MSG] = ROOM_EXIST;
        return respond;
    }
    addRoom(std::stoi(roomNo), std::stoi(maxCapacity), std::stoi(maxCapacity), std::stoi(price), false);
    respond[IS_ERROR] = false;
    respond[ERROR_MSG] = SUCCESSFULLY_ADDED;
    updateRoomsFile();
    return respond;
}

json Hotel::modifyRoom(json command)
{
    json respond;
    respond[IS_ERROR] = true;
    int roomNo = std::stoi(command["roomNo"].get<std::string>());
    int maxCapacity = std::stoi(command["newMaxCap"].get<std::string>());
    int price = std::stoi(command["newPrice"].get<std::string>());
    auto room = rooms_.find(roomNo);
    if (room == rooms_.end())
    {
        respond[ERROR_MSG] = ROOM_NOT_FOUND;
        return respond;
    }
    date:: year_month_day lasCheckOut = findLastCheckOut(roomNo);
    date:: year_month_day tempDate = date_;
    while (tempDate < lasCheckOut)
    {
        int numOfFull = 0;
        auto itr = reservations_.find(roomNo);
        if (itr != reservations_.end())
        {
            for (auto reserve: reservations_[roomNo])
            {
                date:: year_month_day checkOut = convertDate(reserve.getCheckOutDate());
                date:: year_month_day checkIn = convertDate(reserve.getReserveDate());
                if (tempDate >= checkIn && tempDate < checkOut)
                    numOfFull += reserve.getNumOfBeds();
            }
        }
        if (numOfFull > maxCapacity)
        {
            respond[ERROR_MSG] = ROOM_IS_FULL;
            return respond;
        }
        tempDate = date::sys_days(tempDate) + date::days(1);
    }

    room->second.setMaxCapacity(maxCapacity);
    room->second.setPrice(price);
    respond[IS_ERROR] = false;
    respond[ERROR_MSG] = SUCCESSFULLY_MODIFIED;
    updateRoomsFile();
    return respond;
}

void Hotel::setUserFd(User* user, int userFd)
{
    user->setFd(userFd);
}

date::year_month_day Hotel::findLastCheckOut(int roomNo)
{
    date::year_month_day lastCheckOut = date_;
    auto itr = reservations_.find(roomNo);
    if (itr != reservations_.end())
    {
        for (auto reserve: reservations_[roomNo])
        {
            date::year_month_day checkOutDate = convertDate(reserve.getCheckOutDate());
            if (checkOutDate > lastCheckOut)
                lastCheckOut = checkOutDate;
        }
    }
    return lastCheckOut;
}

json Hotel::removeRoom(json command)
{
    json respond;
    respond[IS_ERROR] = true;
    int roomNo = std::stoi(command["roomNo"].get<std::string>());
    if (!doesRoomExist(roomNo))
    {
        respond[ERROR_MSG] = ROOM_NOT_FOUND;
        return respond;
    }
    auto itr = reservations_.find(roomNo);
    if (itr != reservations_.end())
    {
        respond[ERROR_MSG] = ROOM_IS_FULL;
        return respond;
    } 
    rooms_.erase(roomNo);
    respond[IS_ERROR] = false;
    respond[ERROR_MSG] = SUCCESSFULLY_DELETED;
    updateRoomsFile();
    return respond;
}

json Hotel::logoutUser(User* user)
{
    user->logout();
    json response;
    response[ERROR_MSG] = LOGOUT_SUCCESSFULLY;
    return response;
}

json Hotel::passDay(int daysNo, bool isUserAdmin)
{
    json response;
    response[IS_ERROR] = false;
    if (!isUserAdmin)
    {
        response[IS_ERROR] = true;
        response[ERROR_MSG] = ACCESS_DENIED;
        return response;
    }
    date_ = date::sys_days(date_) + date::days(daysNo);
    for (auto room: rooms_)
    {
        std::vector < Reservation> resCpy (reservations_[room.first].begin(), reservations_[room.first].end());
        for (int i = resCpy.size() -1; i >= 0; i--)
        {
            std::istringstream ss(resCpy[i].getCheckOutDate());
            date::year_month_day chechOutDate;
            ss >> date::parse("%F", chechOutDate);
            if (chechOutDate < date_)
            {
                room.second.increaseSpace(reservations_[room.first][i].getNumOfBeds());
                reservations_[room.first].erase(reservations_[room.first].begin() + i);
            }
        }
    }
    updateRoomsFile();
    return response;
}

bool Hotel::doesHaveTimeConflict(date::year_month_day reserveDate1, date::year_month_day checkOutDate1, 
                                 date::year_month_day reserveDate2, date::year_month_day checkOutDate2)
{
    return (reserveDate1 <= reserveDate2 && checkOutDate1 <= checkOutDate2) ||
           (reserveDate1 >= reserveDate2 && checkOutDate1 <= checkOutDate2) ||
           (reserveDate1 <= reserveDate2 && checkOutDate1 >= checkOutDate2) ||
           (reserveDate1 >= reserveDate2 && checkOutDate1 >= checkOutDate2);
}

json Hotel::booking(User *user, json command)
{
    json respond;
    bool isReserved = false;
    int roomNo = std::stoi(command["roomNum"].get<std::string>());
    int numOfBeds = std::stoi(command["numOfBeds"].get<std::string>());
    std::string checkInDate = command["checkInDate"];
    std::string checkOutDate = command["checkOutDate"];
    date::year_month_day checkIn = convertDate(checkInDate);
    date::year_month_day checkOut = convertDate(checkOutDate);

    respond[IS_ERROR] = true;
    if (!doesRoomExist(roomNo))
    {
        respond[ERROR_MSG] = ROOM_NOT_FOUND;
        return respond;
    }

    Room foundRoom = rooms_.find(roomNo)->second;
    int roomPrice = foundRoom.getPrice();

    if (user->getMoney() < roomPrice * numOfBeds)
    {
        respond[ERROR_MSG] = LOW_BALANCE;
        return respond;
    }

    date::year_month_day tempDay = checkIn;
    

    while (tempDay <= checkOut)
    {
        int numOfFullBeds = 0;
        auto itr = reservations_.find(roomNo);
        if (itr != reservations_.end())
        {
            for (auto reserve : reservations_[roomNo])
            {
                date::year_month_day resCheckIn = convertDate(reserve.getReserveDate());
                date::year_month_day resCheckOut = convertDate(reserve.getCheckOutDate());
                if (tempDay >= resCheckIn && tempDay < resCheckOut)
                    numOfFullBeds += reserve.getNumOfBeds();   
            }
        }
        if (foundRoom.getMaxCapacity() - numOfFullBeds < numOfBeds)
        {
            respond[ERROR_MSG] = ROOM_IS_FULL;
            return respond;
        }
        tempDay = date::sys_days(tempDay) + date::days(1);
    }
   
    user->pay(numOfBeds * roomPrice);
    foundRoom.fill(numOfBeds);
    respond[IS_ERROR] = false;
    updateRoomsFile();
    return respond;
}

json Hotel::showUserReserves(User* user)
{
    json reservations;
    reservations["user"] = json::array();
    reservations["isEmpty"] = true;
    for (auto room: rooms_)
    {
        for (auto reserve: reservations_[room.first])
        {
            if (reserve.getUserId() == user->getId())
            {
                json info;
                info["reserveDate"] = reserve.getReserveDate();
                info["checkOutDate"] = reserve.getCheckOutDate();
                info["roomNo"] = room.first;
                info["numOfBeds"] = reserve.getNumOfBeds();
                reservations["user"].push_back(info);
                reservations["isEmpty"] = false;
            }
        }
    }
    return reservations;
}

json Hotel::canceling(User* user, json command)
{
   int roomNo = stoi(command["roomNo"].get<std::string>());
   int numOfBeds = stoi(command["numOfBeds"].get<std::string>());
   json response;
   response[IS_ERROR] = true;
   if (!doesRoomExist(roomNo))
   {
        response[ERROR_MSG] = ROOM_NOT_FOUND;
        return response;
   }
    auto itr = reservations_.find(roomNo);
    if (itr != reservations_.end())
    {
        std::vector < Reservation> resCpy (reservations_[roomNo].begin(), reservations_[roomNo].end());
        for (int i = resCpy.size()-1; i >=0; i--)
        {
            date::year_month_day reserveDate = convertDate(reservations_[roomNo][i].getReserveDate());
            if (reservations_[roomNo][i].getUserId() == user->getId() && reservations_[roomNo][i].getNumOfBeds() >= numOfBeds && date_ < reserveDate)
            {
                response[IS_ERROR] = false;
                response[ERROR_MSG] = SUCCESSFULLY_DONE;
                Room room = rooms_.find(roomNo)->second;
                room.increaseSpace(numOfBeds);
                user->getBackMoney(numOfBeds*room.getPrice());
                reservations_[roomNo].erase(reservations_[roomNo].begin()+i);
                updateRoomsFile();
                return response;
            }
        }
    }
   
   response[ERROR_MSG] = RESERVE_NOT_FOUND;
   return response;
}

void Hotel::updateRoomsFile()
{
    std::ofstream file(ROOMS_FILE); 
    json rooms;
    rooms["rooms"] = json::array();
    for (auto room: rooms_)
    {
        json r;
        r["roomNo"] = room.first;
        r["isFull"] = room.second.getIsFull();
        r["price"] = room.second.getPrice();
        r["maxCapacity"] = room.second.getMaxCapacity();
        r["freeCapacity"] = room.second.getFreeCapacity();
        r["users"] = json::array();
        for (auto reserve: reservations_[room.first])
        {
            json user;
            user["id"] = reserve.getUserId();
            user["numOfBeds"] = reserve.getNumOfBeds();
            user["reserveDate"] = reserve.getReserveDate();
            user["checkOutDate"] = reserve.getCheckOutDate();
            r["users"].push_back(user);
        }
        rooms["rooms"].push_back(r);
    }
    file << rooms.dump(4);
}

void Hotel::updateUsersFile()
{
    std::ofstream file(USERS_FILE);
    json users;
    users["users"] = json::array();
    for (auto user: users_)
    {
        json u;
        u["id"] = user.first;
        u["username"] = user.second.getName();
        u["password"] = user.second.getPassword();
        u["isAdmin"] = user.second.getIsAdmin();
        if (!user.second.getIsAdmin())
        {
            u["money"] = user.second.getMoney();
            u["phoneNumber"] = user.second.getPhoneNumber();
            u["address"] = user.second.getAddress();
        }
        users["users"].push_back(u);
    }
    file << users.dump(4);
}