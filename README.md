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

## Server

This code defines a "Server" class and its "start" method.
The "start" method sets up a server socket and initializes a file descriptor set. It enters into a loop for checking and handling client connections.

The loop uses the "select" function to monitor the file descriptor set and wait for an event. If an incoming connection request is detected, a new client connection is accepted and added to the file descriptor set.

If there is no incoming connection request, then the function iterates through the file descriptor set, and if there is any data is available to read, the function reads the data (in JSON format), processes it through a command handler, and sends a response back to the client.

Finally, if the received data is an EOF signal, the connection of the relevant client is closed, and its file descriptor is removed from the file descriptor set. The loop keeps running until the server process is terminated.

We must read Configuration, Users and Rooms information from the json files and save their datas.

```cpp
while (1)
{
    working_set = master_set;
    if (select(max_sd + 1, &working_set, NULL, NULL, NULL) < 0)
        perror("select");

    if (FD_ISSET(server_fd, &working_set))
    { // new clinet
        new_socket = acceptClient(server_fd);
        FD_SET(new_socket, &master_set);
        if (new_socket > max_sd)
            max_sd = new_socket;
        printf("New client connected. fd = %d\n", new_socket);
    }
    else
    {
        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                int bytes_received;
                bytes_received = recv(i, buffer, 1024, 0);
                if (bytes_received == 0)
                { // EOF
                    printf("client fd = %d closed\n", i);
                    close(i);
                    FD_CLR(i, &master_set);
                    continue;
                }
                nlohmann::json message = json::parse(buffer);
                nlohmann::json sendMsg = commandHandler_.runCommand(message, i);
                printf("client %d: %s\n", i, buffer);
                memset(buffer, 0, 1024);
                std::string sendMsgstr = sendMsg.dump();
                if (send(i, sendMsgstr.c_str(), sendMsgstr.size(), 0) < 0)
                    perror("send");
            }
        }
    }
}
```

## Client

This code defines a "Client" class and its "run" method.

In "run" method, the function waits for input from the user and takes commands from standard input with options. It then parses the input and takes actions based on the commands.

The code checks the inputted command's prefix and the condition-based configurations set for executing actions. Based on this information, corresponding actions will be executed.

The actions include sign-in, sign-up, view user information, view all users, view room information, book a room, cancel a room booking, pass a day, edit information, leave a room, free a room, room command, add, modify, remove, and logout. If the input is an invalid command, the function will prompt the user a message.

The function runs continuously while waiting for input from the user, except if the user logs out or an error has occurred.

## Read from files

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

## Set server date

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
    std::istringstream ss(date);
    date::year_month_day resultDate;
    ss >> date::parse("%F", resultDate); // %F: %Y-%M-%d
    if (ss.fail())
        return false;
    std::string tempStr;
    if (ss >> tempStr)
        return false;
    return true;
}
```

## Sign up

Then client has to signin or signup to server by entering the related command. If the client wants to sign up, first it should be checked whether there is a client with this username or not, if there is, it will give an error and if not, it will receive the rest of the information from the client and add it to users. As we can see in this function, server sends informations and errors by JSON, then the client recieved and convert it to string file to undrestand the messages.

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

json Hotel::addUser(json user)
{
    json respond;
    respond["isError"] = false;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    lastAssignedId_ += 1;
    int id = lastAssignedId_;
    std::string username = user["username"].get<std::string>();
    std::string password = user["password"].get<std::string>();
    std::string phoneNumber = user["phoneNumber"].get<std::string>();
    std::string address = user["address"].get<std::string>();
    int money = stoi(user["money"].get<std::string>());
    User newUser = User(id, username, password, false, phoneNumber, address, money);
    users_.insert({id, newUser});
    respond["errorMessage"] = USER_SUCCESSFULLY_SIGN_UP;
    return respond;
}
```

## Sign in

But for sign in, the client must enter the username and password, then check that this information is entered correctly, if it is correct, the user is logged in, otherwise the username or password is entered incorrectly which returns error to the client.

```cpp
json Hotel::findUserByName(std::string username, std::string password, int userFd)
{
    json response;
    response["isError"] = "true";
    response["errorMessage"] = INVALID_USERNAME_OR_PASSWORD;
    for (auto &user : users_)
    {
        if (user.second.getName() == username && user.second.getPassword() == password)
        {
            setUserFd(&user.second, userFd);
            response["isError"] = false;
            response["errorMessage"] = USER_LOGGED_IN;
        }
    }
    return response;
}
```

## Main Page

After the user has successfully logged in, the main page of the program will be displayed in the console and then the client enters the desired command. In case of any error or mistake in entering the option, the error will be shown to the user and this information will be displayed again. The Commands are:

- View user information
- View all users
- View rooms information
- Booking
- Canceling
- pass day
- Edit information
- Leaving room
- Rooms
- Logout

## View user information

**View user information**

This command is available for both types of users and displays all user informations. The return value is Json, which save the users information and send it to the client.

```cpp
json User::viewInfo(bool isSelfInfo)
{
    json info;
    info["username"] = username_;
    info["id"] = id_;
    info["isAdmin"] = isAdmin_;
    if (isSelfInfo)
        info["password"] = password_;
    if (!isAdmin_)
    {
        info["phoneNumber"] = phoneNumber_;
        info["address"] = address_;
        info["money"] = money_;
    }
    return info;
}
```

## View all users

**View all users**

This command is accessible only to the admin user, and if a normal user enters this command, it will encounter a 403 error and return to the main page. In this section, all user information except the password will be displayed to the admin.  
The code generates a list of users that can be viewed by an admin user. The function takes the user as the parameter, and it first instantiates a JSON object to store user information. If the user is an admin, the function loops through all the registered users and adds each user's information to the JSON object. If the user is not an admin, an error message is added to the JSON object indicating that the user does not have permission to perform this action. Finally, the JSON object with all the user information is returned.

```cpp
json Hotel::viewAllUsers(User *user)
{
    json usersInfo;
    usersInfo["isError"] = false;
    usersInfo["errorMessage"] = "";
    usersInfo["users"] = json::array();
    if (user->isAdmin())
    {
        for (auto user : users_)
            usersInfo["users"].push_back(user.second.viewInfo(false));
    }
    else
    {
        usersInfo["isError"] = true;
        usersInfo["errorMessage"] = ACCESS_DENIED;
    }
    return usersInfo;
}
```

## View rooms information

**View rooms information**

This command is available for both types of users. This section displays all the information of all the rooms, whether they are full or empty.  
The code generates a list of rooms that can be viewed by users. The user is specified in the parameters, and they can either be an admin or not. The isEmptyRoom parameter specifies whether only empty rooms should be displayed. For each room available in the hotel, the function gets information about it such as its room number, price, maximum capacity, and free capacity. If the user is an administrator and the room is not empty, then a list of users who have made reservations and the dates of those reservations are also displayed. Finally, the information is returned as a JSON object.

```cpp
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
```

## Booking

**book _RoomNum_ _NumOfBeds_ _CheckInDate_ _CheckOutDate_**  
This command is used for the normal user. In this section, the user can book a room with a specified capacity and the exact checkIn and checkOut date.  
The code is for booking a hotel room for a user. It first gets the booking details such as the room number, number of beds, check-in date, and check-out date. It then checks if the requested room exists and if the user has enough money to pay for the booking. It then checks if the room is available during the requested dates and if it has enough beds available. If all these checks pass, the user is charged for the booking and the number of beds are marked as filled in the room. Finally, a response is returned indicating whether the booking was successful or if there was an error with an appropriate error message.  
First, it should be checked whether there is a room with this number or not. If not, error number 101 will be displayed. Second, the user money must be checked whether there is enough money for reserving the room or not. If not, error number 108 will be displayed. If the reservation is not possible due to the room being full, error number 109 will be displayed. Before all, if the command structure is incorrect, error number 503 will be displayed.

```cpp
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

    respond["isError"] = true;
    if (!doesRoomExist(roomNo))
    {
        respond["errorMessage"] = ROOM_NOT_FOUND;
        return respond;
    }

    Room foundRoom = rooms_.find(roomNo)->second;
    int roomPrice = foundRoom.getPrice();

    if (user->getMoney() < roomPrice * numOfBeds)
    {
        respond["errorMessage"] = LOW_BALANCE;
        return respond;
    }

    date::year_month_day tempDay = checkIn;

    while (tempDay <= checkOut)
    {
        int numOfFullBeds = 0;
        for (auto reserve : reservations_[roomNo])
        {
            date::year_month_day resCheckIn = convertDate(reserve.getReserveDate());
            date::year_month_day resCheckOut = convertDate(reserve.getCheckOutDate());
            if (tempDay >= resCheckIn && tempDay < resCheckOut)
                numOfFullBeds += reserve.getNumOfBeds();
        }
        if (foundRoom.getMaxCapacity() - numOfFullBeds < numOfBeds)
        {
            respond["errorMessage"] = ROOM_IS_FULL;
            return respond;
        }
        tempDay = date::sys_days(tempDay) + date::days(1);
    }

    user->pay(numOfBeds * roomPrice);
    foundRoom.fill(numOfBeds);
    respond["isError"] = false;
    return respond;
}
```

## Canceling

**cancel _RoomNum_ _Num_**  
This command is used when the user wants to cancel his reservation earlier than Checkout, in this case the user's account will be deleted and half of the money will be returned to the user.  
The code defines a method in a Hotel class that cancels a reservation for a specific user. It takes a User and a json object as inputs, containing the room number and number of beds requested to cancel. It first sets a response object with isError set to true. If the room number is not found in the hotel's records, the response object is immediately returned with an error message.
Otherwise, the method searches for a reservation matching the user and requested number of beds, erasing it from the hotel's records and increasing the room's available space. If a matching reservation is not found, the response object is updated with a corresponding error message and returned.  
After entering the fifth number, a list of user reservations should be displayed. If the desired room is not found, error number 101 will be displayed, and if there is no reservation with the desired user with that number or more in the desired room, error number 102 will be displayed, and if the command format is incorrect, error 401 will be displayed.  
_Note that the system must delete the reservation when the system calendar equals the Checkout of any reservation._

```cpp
json Hotel::canceling(User *user, json command)
{
    int roomNo = stoi(command["roomNo"].get<std::string>());
    int numOfBeds = stoi(command["numOfBeds"].get<std::string>());
    json response;
    response["isError"] = true;
    if (!doesRoomExist(roomNo))
    {
        response["errorMessage"] = ROOM_NOT_FOUND;
        return response;
    }
    std::vector<Reservation> resCpy(reservations_[roomNo].begin(), reservations_[roomNo].end());
    for (int i = resCpy.size() - 1; i >= 0; i--)
    {
        if (reservations_[roomNo][i].getUserId() == user->getId() && reservations_[roomNo][i].getNumOfBeds() >= numOfBeds)
        {
            response["isError"] = false;
            response["errorMessage"] = SUCCESSFULLY_DONE;
            Room room = rooms_.find(roomNo)->second;
            room.increaseSpace(numOfBeds);
            reservations_[roomNo].erase(reservations_[roomNo].begin() + i);
            return response;
        }
    }
    response["errorMessage"] = RESERVE_NOT_FOUND;
    return response;
}
```

## Pass day

**passday _Value_**  
This command is accessible only to the admin user, and if a normal user enters this command, he will encounter a 403 error and return to the main page of the system. In this section, the admin can enter the number of days that have passed.  
The code is for moving ahead in time by a specified number of days (daysNo). It first checks if the user is an admin, and if not, an error message is returned. If the user is an admin, the date of the hotel is updated to reflect the new date. Then for each room in the hotel, the function checks each reservation and determines if it has passed its checkout date. If so, the function sends notification to the specified room and the reservation record for that booking is deleted. Finally, a response JSON object is returned indicating whether there was an error and whether the date was successfully updated.

```cpp
json Hotel::passDay(int daysNo, bool isUserAdmin)
{
    json response;
    response["isError"] = false;
    if (!isUserAdmin)
    {
        response["isError"] = true;
        response["errorMessage"] = ACCESS_DENIED;
        return response;
    }
    date_ = date::sys_days(date_) + date::days(daysNo);
    for (auto room : rooms_)
    {
        std::vector<Reservation> resCpy(reservations_[room.first].begin(), reservations_[room.first].end());
        for (int i = resCpy.size() - 1; i >= 0; i--)
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
    return response;
}
```

## Edit information

**_new password_**  
**_phone_**  
**_address_**

This command is accessible to both types of users. In this section, the user can change the entered information. In case of any error or wrong data entry, error 503 will be displayed to the user and he will be redirected to the main page, and if successful, message 312 will be displayed.  
The code is for updating and editing user information. The function takes a JSON object as input containing the new information to be updated, including the new password, phone number, and address. If the user is an admin, only the password is updated. If the user is not an admin, the password, phone number, and address fields are all updated. Finally, a response JSON object is returned indicating that the information was changed without any errors.

```cpp
json User::editInformation(json command)
{
    json response;
    if (isAdmin_)
    {
        password_ = command["newPassWord"];
    }
    else
    {
        password_ = command["newPassWord"];
        phoneNumber_ = command["newPhone"];
        address_ = command["newAddress"];
    }
    response["isError"] = false;
    response["errorMessage"] = INFORMATION_CHANGED;
    return response;
}
```

## Leaving room

**room _Value_**  
**capacity _new capacity_**  
This command is available for both types of users. A normal user can deliver the room ahead of time and leave the room.  
The code is for a user leaving a hotel room. It checks whether the room exists and whether the user has a reservation for the specified room. If the room does not exist or the user does not have a reservation, an error message is returned as a JSON object. If the user does have a reservation, the reservation record for that room and user is deleted, and the response JSON object indicates a successful checkout.

```cpp
json Hotel::leavingRoom(User *user, int roomNo)
{
    json response;
    response["isError"] = true;
    if (!doesRoomExist(roomNo))
    {
        response["errorMessage"] = BAD_SEQUENCE_OF_COMMANDS;
        return response;
    }
    std::vector<Reservation> resCpy(reservations_[roomNo].begin(), reservations_[roomNo].end());
    for (int i = resCpy.size() - 1; i >= 0; i--)
    {
        if (resCpy[i].getUserId() == user->getId())
        {
            reservations_[roomNo].erase(reservations_[roomNo].begin() + i);
            response["isError"] = false;
            response["errorMessage"] = SUCCESSFULLY_LEAVING;
            return response;
        }
    }
    response["errorMessage"] = RESERVE_NOT_FOUND;
    return response;
}
```

The admin can empty any room and change the status of the room.  
The code is for emptying a hotel room. It first checks if the user is an admin, and if not, an error message is returned. If the user is an admin, the function checks if the specified room exists. If the room is found, the function updates the room to be empty. Next, it checks each reservation for that room, and if the reservation's check-out date has passed the current date or is equal to the current date, the reservation record for that booking is deleted. Finally, a response JSON object is returned indicating whether there was an error and whether the room was successfully emptied.

```cpp
json Hotel::emptyRoom(int roomNo, bool isUserAdmin)
{
    json response;

    response["isError"] = true;
    if (!isUserAdmin)
    {
        response["errorMessage"] = ACCESS_DENIED;
        return response;
    }
    if (!doesRoomExist(roomNo))
    {
        response["errorMessage"] = ROOM_NOT_FOUND;
        return response;
    }
    for (auto room : rooms_)
    {
        if (room.first == roomNo)
            room.second.makeEmpty();
    }
    std::vector<Reservation> resCpy(reservations_[roomNo].begin(), reservations_[roomNo].end());
    for (int i = resCpy.size() - 1; i >= 0; i--)
    {
        date::year_month_day reserveDate = convertDate(reservations_[roomNo][i].getReserveDate());
        date::year_month_day checkOutDate = convertDate(reservations_[roomNo][i].getCheckOutDate());
        if (checkOutDate <= date_)
            reservations_[roomNo].erase(reservations_[roomNo].begin() + i);
    }

    response["isError"] = false;
    response["errorMessage"] = SUCCESSFULLY_LEAVING;
    return response;
}
```

## Rooms

**add _RoomNum_ _Max Capacity_ _Price_**  
**modify _RoomNum_ _New Max Capacity_ _New Price_**  
**remove _RoomNum_**  
This command is only accessible to the admin user, and if a normal user enters this command, he will encounter a 403 error and return to the main page of the system. On this page, the admin can add or change rooms using three commands. If there is no room number, error number 101 will be displayed. If there is a room number in the Add command, error number 111 will be displayed.

### **ADD**

The method then checks if a room with the given roomNo already exists or not. If a room with the given roomNo exists, it returns an error message in the JSON response. Else, it calls the addRoom method of the Hotel class with the extracted values.

```cpp
json Hotel::addNewRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNo"];
    std::string maxCapacity = command["maxCap"];
    std::string price = command["price"];
    if (doesRoomExist(std::stoi(roomNo)))
    {
        respond["errorMessage"] = ROOM_EXIST;
        return respond;
    }
    addRoom(std::stoi(roomNo), std::stoi(maxCapacity), std::stoi(maxCapacity), std::stoi(price), false);
    respond["isError"] = false;
    respond["errorMessage"] = SUCCESSFULLY_ADDED;
    return respond;
}
```

### **MODIFY**

The function loops through each room in the collection of rooms\_ and checks if the room number matches the value that was included in the input JSON object.  
If a matching room is found, the function checks if the new maximum capacity is valid and updates the relevant attributes of the room object with the new values that were provided in the input JSON object. The function then sets the value of the "isError" key in the respond JSON object to false and the value of the "errorMessage" key to "SUCCESSFULLY_MODIFIED", before returning the object.
If no matching room is found in the room collection, the function sets the value of the "errorMessage" key in the respond JSON object to "ROOM_NOT_FOUND" and returns the object.

```cpp
json Hotel::modifyRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNo"];
    std::string maxCapacity = command["newMaxCap"];
    std::string price = command["newPrice"];
    for (auto room : rooms_)
    {
        if (room.first == std::stoi(roomNo))
        {
            if (room.second.getFreeCapacity() != room.second.getMaxCapacity() && room.second.getMaxCapacity() > std::stoi(maxCapacity))
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            room.second.setMaxCapacity(std::stoi(maxCapacity));
            room.second.setPrice(std::stoi(price));
            respond["isError"] = false;
            respond["errorMessage"] = SUCCESSFULLY_MODIFIED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
    return respond;
}
```

### **REMOVE**

The function checks if a room with the given roomNo exists in the rooms\_ collection.

If a matching room is found, it checks if the room is unoccupied. If the room is unoccupied, it removes the room from the rooms\* collection, sets the value of the "isError" key in the respond JSON object to false, sets the value of the "errorMessage" key to "SUCCESSFULLY_DELETED", and returns the object.  
If the room is currently occupied, it sets the value of the "errorMessage" key in the respond JSON object to "ROOM_IS_FULL", indicating that the room cannot be removed because it is currently occupied, and returns the object.
If no matching room is found in rooms, the function sets the value of the "errorMessage" key in the respond JSON object to "ROOM_NOT_FOUND" and returns the object with the "isError" key set to true.

```cpp
json Hotel::removeRoom(json command)
{
    json respond;
    respond["isError"] = true;
    std::string roomNo = command["roomNo"];
    for (auto room : rooms_)
    {
        if (room.first == std::stoi(roomNo))
        {
            if (room.second.getFreeCapacity() != room.second.getMaxCapacity())
            {
                respond["errorMessage"] = ROOM_IS_FULL;
                return respond;
            }
            rooms_.erase(std::stoi(roomNo));
            respond["isError"] = false;
            respond["errorMessage"] = SUCCESSFULLY_DELETED;
            return respond;
        }
    }
    respond["errorMessage"] = ROOM_NOT_FOUND;
    return respond;
}
```

## Logout

**Logout**  
This command is available for both types of users. By entering this command, the user will be exited from the program and the connection of the same client with the server will be terminated. If the user logout is successful, message 201 will be displayed.

```cpp
json Hotel::logoutUser(User *user)
{
    user->logout();
    json response;
    response["errorMessage"] = LOGOUT_SUCCESSFULLY;
    return response;
}
```
