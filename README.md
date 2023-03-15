# CN_CHomeworks_1
## How to run?
using the terminal cd to CN_CHomeworks_1 directory and then:
```cpp
$ make
```
Then cd to `bin/` directory and run:
```cpp
$ ./server
$ ./client
```
To start the program, we define 8 classes:

- Server  
   In the Server class, reading from files, setting date and accepting clients are done.
- Client  
   In the Client class, defining the command's functions which getting the information from the client and send it to server is done.
- Command Handler  
   In the Command Handler class, running commands which sent from client to server and call the related function in hotel or user class are done.
- User  
   The User class, sets the user's information.
- Hotel  
   In the Hotel class, defining the command's functions which sends the information from server to client are done.
- Room  
   The Room class, sets the room's information.
- Reservation
   The Reservation class consists of each reservation information.
- Logger
   The Logger class is for writing logs to json files.

## Server

This is the declaration of Server class. As you can see we read the port and hostname from the config.json file and save them in port_ and hostname_ attributes. This class also contains an instance of commandHandler for handeling client's commands. The logger_ is for saving logs of different clients connecting to server.
```cpp
class Server
{
public:
    Server();
    void start();
    int acceptClient(int server_fd);
    int setupServer();
    bool checkDate(std::string date);
    void setServerDate(std::string date);
    void readPortHostnameConfig();
    json readRoomsConfig();
    json readUsersConfig();
    std::vector<std::string> tokenizeCommand(std::string command);
    int setupServer();
    void setDate();

private:
    CommandHandler commandHandler_;
    int port_;
    std::string hostname_;
    Logger logger_;
};
```
This code defines a "Server" class and its "start" method.
The "start" method sets up a server socket and initializes a file descriptor set. It enters into a loop for checking and handling client connections.

The loop uses the "select" function to monitor the file descriptor set and wait for an event. If an incoming connection request is detected, a new client connection is accepted and added to the file descriptor set. Also at the begining we add server's fd and STDIN_FILENO to the dile descriptor set.

If there is no incoming connection request, then the function iterates through the file descriptor set, and if there is any data is available to read, the function reads the data (in JSON format), processes it through a command handler, and sends a response back to the client.

Finally, if the received data is an EOF signal (the size of recieved data is 0), the connection of the relevant client is closed, and its file descriptor is removed from the file descriptor set. The loop keeps running until the server process is terminated. Also if we enter "exit" command in std input, the server closes all clients' fds which are is master set.

We must read Configuration, Users and Rooms information from the json files and save their datas.

```cpp
    while (1)
    {
        working_set = master_set;
        if (select(max_sd + 1, &working_set, NULL, NULL, NULL)<0)
            perror("select");
        if (FD_ISSET(STDIN_FILENO, &working_set))
        {
            memset(buffer, 0, 1024);
            read(STDIN_FILENO, buffer, 1024);
            if (std::string(buffer) == "exit\n")
            {
                for (int j = 1; j < max_sd + 1; j++)
                    if (FD_ISSET(j, &master_set))   
                        close(j);
                return;
            }
        }
        else if (FD_ISSET(server_fd, &working_set))
        { // new client
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
                    memset(buffer, 0, 1024);
                    bytes_received = recv(i, buffer, 1024, 0);
                    if (bytes_received == 0)
                    { 
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    nlohmann::json message = json::parse(buffer);
                    nlohmann::json sendMsg = commandHandler_.runCommand(message, i);
                    logger_.log(message, i);
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
This is the declaration of Client class. We save the port and hostname. readBuffer is a char* for saving what the client has recieved. hasLoggedIn_ shows the login status of the client. isRoomCmd_ shows if the client enteres the room command recently. logger_ is for saving logs of each client in json files.

```cpp
class Client
{
public:
    Client();
    void run();
    int connectServer(const int port, const std::string host);
    bool checkDate(std::string date);
    bool checkDigits(std::string digits);
    std::vector<std::string> tokenizeCommand(std::string command);
    void signInCommand(std::vector<std::string> &tokens);
    bool signUpCommand(std::string &username);
    bool isTokenSizeCorrect(int tokenSize, int correctNum);
    void viewUserInfoCommand();
    bool viewAllUsers();
    bool viewRoomsInfo();
    bool booking();
    bool canceling();
    bool passDay();
    bool editInfo();
    bool leaveRoom();
    bool freeRoom();
    void roomCommand();
    bool addRoom(std::vector<std::string> &tokens);
    bool modifyRoom(std::vector<std::string> &tokens);
    bool removeRoom(std::string &roomNo);
    void logout();
    void readConfig();
    void setupLogger(std::string username);

private:
    bool hasLoggedIn_;
    bool isRoomCmd_;
    int commandFd_;
    char readBuffer[1024];
    int port_;
    std::string hostname_;
    Logger logger_;
};
```
This code defines a "Client" class and its "run" method.

In "run" method, the function waits for input from the user and takes commands from standard input with options. It then parses the input and takes actions based on the commands.

The code checks the inputted command's prefix and the condition-based configurations set for executing actions. Based on this information, corresponding actions will be executed.

The actions include sign-in, sign-up, view user information, view all users, view room information, book a room, cancel a room booking, pass a day, edit information, leave a room, free a room, room command, add, modify, remove, logout, and exit. If the input is an invalid command, the function will prompt the user a message.

The function runs continuously while waiting for input from the user, except if the user exits or an error has occurred. The user can only exits after logging out or before logging in; after logging out, the client fd in client class is set to -1 (which is the deafult value for not having logged in). By entering exit command, the client's fd closes.

```cpp
void Client::run()
{
    std::string command;
    commandFd_ = connectServer(port_, hostname_);
    std::vector<std::string> tokens;
    while (true)
    {
        if (hasLoggedIn_ && !isRoomCmd_)
            std::cout << HELP << std::endl;
        else if (!hasLoggedIn_)
            std::cout << SIGNUP_IN_HELP << std::endl;
        std::getline(std::cin, command);
        tokens = tokenizeCommand(command);
        std::string cmd = tokens[0];
        memset(readBuffer, 0, 1024);
        if (cmd == SIGN_IN && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 3))
            signInCommand(tokens);

        else if (cmd == SIGN_UP && !hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 2))
        {
            if (!signUpCommand(tokens[1]))
                continue;
        }

        else if (cmd == VIEW_USER_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            viewUserInfoCommand();

        else if (cmd == VIEW_ALL_USERS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
        {
           if (!viewAllUsers())
                continue;
        }

        else if (cmd == VIEW_ROOMS_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
        {
            if (!viewRoomsInfo())
                continue;
        }    

        else if (cmd == BOOKING && hasLoggedIn_)
        {
            if (!booking())
                continue;
        }

        else if (cmd == CANCELING && hasLoggedIn_)
        {
            if (!canceling())
                continue;
        }

        else if (cmd == PASS_DAY && hasLoggedIn_)
        {
            if (!passDay())
                continue;
        }   

        else if (cmd == EDIT_INFORMATION && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
        {
            if (!editInfo())
                continue;
        }

        else if (cmd == LEAVING_ROOM && hasLoggedIn_)
        {
            if (!leaveRoom())
                continue;
        }

        else if (cmd == FREE_ROOM)
        {
            if (!freeRoom())
                continue;
        }

        else if (cmd == ROOMS && isTokenSizeCorrect(tokens.size(), 1) && hasLoggedIn_)
            roomCommand();

        else if (cmd == ADD && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
        {
           if (!addRoom(tokens))
                continue;
        }

        else if (cmd == MODIFY && isTokenSizeCorrect(tokens.size(), 4) && isRoomCmd_ && hasLoggedIn_)
        {
           if (!modifyRoom(tokens))
                continue;
        }

        else if (cmd == REMOVE && isTokenSizeCorrect(tokens.size(), 2) && isRoomCmd_ && hasLoggedIn_)
        {
            if (!removeRoom(tokens[1]))
                continue;
        }

        else if (cmd == LOGOUT && hasLoggedIn_ && isTokenSizeCorrect(tokens.size(), 1))
            logout();

        else if (cmd == EXIT && !hasLoggedIn_)
        {
            close(commandFd_);
            return;
        }

        else
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
    }
}
```
send and recieve functions in all client commands works as follow:

we convert the json tu string and send it to commandFd_ and then empty the readBuffer char* and recv message from commandFd_. Then we conver the recieved string to json and print it.
```cpp
void Client::viewUserInfoCommand()
{
    nlohmann::json message;
    isRoomCmd_ = false;
    message["cmd"] = "View user information";
    std::string messageStr = message.dump();
    if (send(commandFd_, messageStr.c_str(), messageStr.size(), 0) < 0)
        perror("send");
    if (recv(commandFd_, readBuffer, sizeof(readBuffer), 0) < 0)
        perror("recieve");
    nlohmann::json recvMessage = json::parse(readBuffer);
    std::cout << "\u2022 User Info:" << std::endl
              << "  ID:        " << recvMessage["id"] << std::endl
              << "  Username:  " << recvMessage["username"] << std::endl
              << "  Password:  " << recvMessage["password"] << std::endl
              << "  Admin:     " << recvMessage["isAdmin"] << std::endl;
    if (!recvMessage["isAdmin"])
    {
        std::cout << "  Phone:     " << recvMessage["phoneNumber"] << std::endl
                  << "  Money:     " << recvMessage["money"] << std::endl
                  << "  Address:   " << recvMessage["address"] << std::endl;
    }
    logger_.log("view user information.", false);
}
```
## CommandHandler
This class contains an instance of Hotel and handles the clients' commnads by calling the appropriate functions of the hotel.
```cpp
class CommandHandler
{
public:
    CommandHandler();
    void setRooms(json &rooms);
    void setUsers(json &users);
    json isUserExist(std::string errMessage);
    json runCommand(json command, int userFd);
    void setDate(date::year_month_day date);

private:
    Hotel hotel_;
};
```
## Hotel
This class contains and unordered_map of users with the key of userId and User class value. reservations_ is also an unordered_map with the roomNumber key and a vector of the reservations for that room. rooms_ is an unordered_map with the roomNumber key and romm value. date_ attribute stores the current date in year_month_day format. lastAssignedId stores the last id that is assigned to a signed up user in the hotel. Also several methods are defined for management of the commands in this class.
```cpp
class Hotel
{
public:
    Hotel();
    ~Hotel();
    void addRoom(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull);
    void addReservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    User *findUser(int userFd);
    json findUserByName(std::string username, std::string password, int userFd);
    json addUser(json command);
    json viewAllUsers(User *user);
    json viewRooms(bool isUserAdmin, bool isEmptyRoom);
    json editRooms(bool isUserAdmin);
    json addNewRoom(json command);
    json modifyRoom(json command);
    json removeRoom(json command);
    json leavingRoom(User *user, int value);
    json emptyRoom(int roomNo, bool isUserAdmin);
    void addRooms(json &rooms);
    void addUsers(json &users);
    void setDate(date::year_month_day date);
    void setUserFd(User* user, int userFd);
    json checkUsernameExistance(std::string username);
    bool doesRoomExist(int roomNo);
    json logoutUser(User* user);
    json passDay(int daysNo, bool isUserAdmin);
    date::year_month_day convertDate(std::string date);
    json booking(User *user, json command);
    bool doesHaveTimeConflict(date::year_month_day reserveDate1, date::year_month_day checkOutDate1, 
                                 date::year_month_day reserveDate2, date::year_month_day checkOutDate2);
    json showUserReserves(User* user);
    json canceling(User* user, json command);
    void updateRoomsFile();
    void updateUsersFile();
    date::year_month_day findLastCheckOut(int roomNo);
private:
    std::unordered_map<int, User> users_;
    std::unordered_map<int, std::vector<Reservation>> reservations_;
    std::unordered_map<int, Room> rooms_;
    date::year_month_day date_;
    int lastAssignedId_;
};
```
## Reservation
The code shows the declaration of the Reservation class. We save the roomNumber, userId of who reserves the room, checkIn and checkOut date and the number of beds.
```cpp
class Reservation
{
public:
    Reservation(int roomNo, int userId, std::string &reserveDate, std::string &checkOutDate, int numOfBeds);
    ~Reservation();
    int getRoomNo();
    int getUserId();
    std::string getReserveDate();
    std::string getCheckOutDate();
    int getNumOfBeds();

private:
    int roomNo_;
    int userId_;
    std::string reserveDate_;
    std::string checkOutDate_;
    int numOfBeds_;
};
```
## Room
This is the declaration of the Room class. we save the roomNumber, maxCapacity of the room, room's price and current freeCapacity. Also isFull_ attribute is true only if the freeCapacity of the room is 0 on current date.
```cpp
class Room
{
public:
    Room(int roomNo, int maxCapacity, int freeCapacity, int price, bool isFull);
    ~Room();
    int getRoomNo();
    int getMaxCapacity();
    int getFreeCapacity();
    bool getIsFull();
    int getPrice();
    void setPrice(int price);
    void setFreeCapacity(int freeCapacity);
    void setMaxCapacity(int maxCapacity);
    void makeEmpty();
    void increaseSpace(int numOfGonePeople);
    void fill(int numOfBeds);
private:
    int roomNo_;
    int maxCapacity_;
    int price_;
    int freeCapacity_;
    bool isFull_;
};
```
## User

This is the declaration of the User class. we save the fd of the logged in clients besides of their attributes. The default value for fd is set to -1.
```cpp
class User
{
public:
    User(int id, std::string username, std::string password, bool isAdmin, std::string phoneNumber, std::string address, int money);
    ~User();
    std::string getName();
    std::string getPassword();
    int getFd();
    int getId();
    void setInfo(std::string &password, int id, std::string &phoneNumber, std::string &address, int money);
    json viewInfo(bool isSelfInfo);
    json editInformation(json command);
    bool isAdmin();
    void setFd(int fd);
    void logout();
    int getMoney();
    void pay(int payment);
    void getBackMoney(int money);
    std::string getPhoneNumber();
    std::string getAddress();
    bool getIsAdmin();

private:
    std::string username_;
    std::string password_;
    int id_;
    bool isAdmin_;
    std::string phoneNumber_;
    std::string address_;
    int money_;
    int fd_;
};
```
## Logger
The code shows the declaration of the Logger class. we save the path of the log file in path_ attribute.
```cpp
class Logger
{
public:
    Logger();
    void log(std::string msg, bool isError);
    void log(nlohmann::json &msg, int clientFd);
    void setPath(std::string path);
private:
    std::string path_;
};
```
## Read from files
We read configuration including port and hostnam eand save them to both client and server classes.
- Read Configuration

```cpp
    std::ifstream fin(CONFIG_FILE);
    json j;
    fin >> j;
    port_ = j["commandChannelPort"].get<int>();
    hostname_ = j["hostName"].get<std::string>();
```

- Save Rooms Informations

We save each rooms information in the Hotel class, if the room is not empty, we make a reservation for each user's booking.
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

- Save Users Informations

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

At first we have to set the server date and check that it is enter correctly. In checkDate function we parse the date using date.h library; if there exist extra digits in the date, we find it out by ss >> tempStr and return false.

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
![setTime server](/images/setTime.jpg "setTime server")
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
![signup user](/images/signup.jpg "signup user")

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
![signin user](/images/signin.jpg "signin user")

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
- exit
![main page](/images/main-page.jpg "main page")
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
![view info](/images/view-info.jpg "view info")
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
![view users info](/images/view-users-information.jpg "view users info")
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
![view rooms](/images/view-rooms.jpg "view rooms")
## Booking

**book _RoomNum_ _NumOfBeds_ _CheckInDate_ _CheckOutDate_**  
This command is used for the normal user. In this section, the user can book a room with a specified capacity and the exact checkIn and checkOut date.  
The code is for booking a hotel room for a user. It first gets the booking details such as the room number, number of beds, check-in date, and check-out date. It then checks if the requested room exists and if the user has enough money to pay for the booking. It then checks if the room is available during the requested dates and if it has enough beds available. If all these checks pass, the user is charged for the booking and the number of beds are marked as filled in the room. Finally, a response is returned indicating whether the booking was successful or if there was an error with an appropriate error message.  
First, it should be checked whether there is a room with this number or not. If not, error number 101 will be displayed. Second, the user money must be checked whether there is enough money for reserving the room or not. If not, error number 108 will be displayed. If the reservation is not possible due to the room being full, error number 109 will be displayed. This is checked by checking the room freeSpace status at each day between the cheIn and checkOut date. Before all, if the command structure is incorrect, error number 503 will be displayed.

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

```
![booking](/images/booking.jpg "booking")
## Canceling

**cancel _RoomNum_ _Num_**  
This command is used when the user wants to cancel his reservation earlier than Checkout, in this case the user's account will be deleted and half of the money will be returned to the user.  
The code defines a method in a Hotel class that cancels a reservation for a specific user. It takes a User and a json object as inputs, containing the room number and number of beds requested to cancel. It first sets a response object with isError set to true. If the room number is not found in the hotel's records, the response object is immediately returned with an error message.
Otherwise, the method searches for a reservation matching the user and requested number of beds, erasing it from the hotel's records and increasing the room's available space. If a matching reservation is not found, the response object is updated with a corresponding error message and returned.  
After entering the fifth number, a list of user reservations should be displayed. If the desired room is not found, error number 101 will be displayed, and if there is no reservation with the desired user with that number or more in the desired room, error number 102 will be displayed, and if the command format is incorrect, error 401 will be displayed.  
_Note that the system must delete the reservation when the system calendar equals the Checkout of any reservation._

```cpp
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
```

## Pass day

**passday _Value_**  
This command is accessible only to the admin user, and if a normal user enters this command, he will encounter a 403 error and return to the main page of the system. In this section, the admin can enter the number of days that have passed.  
The code is for moving ahead in time by a specified number of days (daysNo). It first checks if the user is an admin, and if not, an error message is returned. If the user is an admin, the date of the hotel is updated to reflect the new date. Then for each room in the hotel, the function checks each reservation and determines if it has passed its checkout date. If so, the function sends notification to the specified room and the reservation record for that booking is deleted. Finally, a response JSON object is returned indicating whether there was an error and whether the date was successfully updated.

```cpp
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

```
![passDay](/images/passDay.jpg "passDay")

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
![passDay](/images/editInformation.jpg "passDay")
## Leaving room

**room _Value_**  
**capacity _new capacity_**  
This command is available for both types of users. A normal user can deliver the room ahead of time and leave the room.  
The code is for a user leaving a hotel room. It checks whether the room exists and whether the user has a reservation for the specified room. If the room does not exist or the user does not have a reservation, an error message is returned as a JSON object. If the user does have a reservation, the reservation record for that room and user is deleted, and the response JSON object indicates a successful checkout.

```cpp
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
```

![leaveRoom](/images/leaveRoom.jpg "leaveRoom")
The admin can empty any room and change the status of the room.  
The code is for emptying a hotel room. It first checks if the user is an admin, and if not, an error message is returned. If the user is an admin, the function checks if the specified room exists. If the room is found, the function updates the room to be empty. Next, it checks each reservation for that room, and if the reservation's check-out date has passed the current date or is equal to the current date, the reservation record for that booking is deleted. Finally, a response JSON object is returned indicating whether there was an error and whether the room was successfully emptied.

```cpp
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

```
![emptyRoom](/images/emptyRoom.jpg "emptyRoom")
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
```
![addRoom](/images/addRoom.jpg "addRoom")
### **MODIFY**

The function loops through each room in the collection of rooms\_ and checks if the room number matches the value that was included in the input JSON object.  
If a matching room is found, the function checks if the new maximum capacity is valid and updates the relevant attributes of the room object with the new values that were provided in the input JSON object. It checks the validity by checking each days reservations number of beds. The function then sets the value of the "isError" key in the respond JSON object to false and the value of the "errorMessage" key to "SUCCESSFULLY_MODIFIED", before returning the object.
If no matching room is found in the room collection, the function sets the value of the "errorMessage" key in the respond JSON object to "ROOM_NOT_FOUND" and returns the object.

```cpp
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
```
![modifyRoom](/images/modifyRoom.jpg "modifyRoom")
### **REMOVE**

The function checks if a room with the given roomNo exists in the rooms\_ collection.

If a matching room is found, it checks if the room is unoccupied. If the room is unoccupied, it removes the room from the rooms\* collection, sets the value of the "isError" key in the respond JSON object to false, sets the value of the "errorMessage" key to "SUCCESSFULLY_DELETED", and returns the object.  
If the room has any reservations, it sets the value of the "errorMessage" key in the respond JSON object to "ROOM_IS_FULL", indicating that the room cannot be removed because it is currently occupied, and returns the object.
If no matching room is found in rooms, the function sets the value of the "errorMessage" key in the respond JSON object to "ROOM_NOT_FOUND" and returns the object with the "isError" key set to true.

```cpp
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
```
![removeRoom](/images/removeRoom.jpg "removeRoom")
## **Logout**

This command is available for both types of users. By entering this command, the user will be exited from the program and the connection of the same client with the server will be terminated. If the user logout is successful, message 201 will be displayed.

```cpp
json Hotel::logoutUser(User* user)
{
    user->logout();
    json response;
    response[ERROR_MSG] = LOGOUT_SUCCESSFULLY;
    return response;
}
```
![logout](/images/logout.jpg "logout")

## **Exit**
In client class Exit command closes the client socket and exits the program.
```cpp
else if (cmd == EXIT && !hasLoggedIn_)
{
    close(commandFd_);
    return;
}
```
