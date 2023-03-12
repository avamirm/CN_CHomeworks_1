# CN_CHomeworks_1

To start the program, we define 7 classes:
- Server  
        In the Server class, reading from files, setting date and accepting clients are done.
- Client  
        In the Client class, defining the command's functions which getting the information from the client and send it to server.
- Command Handler  
        In the Command Handler class, running commands which sent from client to server and call the related function in hotel or user class.
- User  
        In the User class, set the user's information.
- Hotel  
        In the Hotel class, defining the command's functions and which sending the information from server to client.
- Room  
        In the Room class, set the room's information.


Then we should read Configuration, Users and Rooms information from the json files and save their datas.
- Read Configuration
```cpp
    std::ifstream fin(CONFIG_FILE);
    json j;
    fin >> j;
    port_ = j["commandChannelPort"].get<int>();
    hostname_ = j["hostName"].get<std::string>();
```

- Save Users Informations
```cpp
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
```

- Save Rooms Informations
```cpp
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
```

At first we have to set the server date and check that it is enter correctly.

```cpp
void Server::setServerDate(std::string date)
{
    std::istringstream ss(date);
    date::year_month_day serverDate;
    ss >> date::parse("%F", serverDate);
    commandHandler_.setDate(serverDate);
}

bool Server::checkDate(std::string date)
{
    // istringstream ss(date);
    // date::year_month_day tempServerDate;
    // std::cin >> date::parse("%F", tempServerDate);
    // if (!std::cin)
    // {
    //     std::cin.clear();
    //     return false;
    // }
    // return true;
    std::istringstream ss(date);
    date::year_month_day tempServerDate;
    ss >> date::parse("%F", tempServerDate);
    if (!ss)
        return false;
    return true;
}
```

Then client has to signin or signup to server by entering the related command and server checking if the user exist or not to send the correct message to client.  
```cpp
json Hotel::checkUsernameExistance(std::string username)
{
    json response;
    response["isError"] = false;
    response["errorMessage"] = USER_SIGNED_UP;
    for (auto user : users_)
    {
        if (user.second.getName() == username)
        {
            response["isError"] = true;
            response["errorMessage"] = USER_EXISTED;
        }
    }
    return response;
}
```

## nes2

sala, salaskn jfd dkfjdkf `func()` kjkj **bold** *italic* ~~strikve~~ ***gf***

- a
- b
- c

[man kossher](google.com)

![man kossher](fuck.png)


| sdddf | sdf |
| :--- | ---- |
| sdf | sdf|


skdfjs kdjfdskjf skdjf
<br/>
<br/>
<br/>
skdfksjdfksdjf kdjf