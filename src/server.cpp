#include "include/server.hpp"
#include <fstream> 

using namespace std;

Server::Server()
{
    readPortHostnameConfig();
    json rooms = readRoomsConfig();
    json users = readUsersConfig();
    CommandHandler commandHandler = CommandHandler();
    commandHandler_ = commandHandler;
    commandHandler_.setRooms(rooms);
    commandHandler_.setUsers(users);
    cout << "Server is running on port " << port_ << endl;
}

void Server::readPortHostnameConfig()
{
    std::ifstream fin(CONFIG_FILE);
    json j;
    fin >> j;
    port_ = j["commandChannelPort"].get<int>();
    hostname_ = j["hostName"].get<std::string>();
}

json Server::readRoomsConfig()
{
    std::ifstream fin(ROOMS_FILE);
    json rooms;
    fin >> rooms;
    return rooms;
}

json Server::readUsersConfig()
{
    std::ifstream fin(USERS_FILE);
    json users;
    fin >> users;
    return users;
}

int Server::acceptClient(int server_fd)
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&address_len);

    return client_fd;
}

int Server::setupServer()
{
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 4);

    return server_fd;
}

std::vector<std::string> Server::tokenizeCommand(std::string command)
{
    std::vector<std::string> tokens;
    std::stringstream check1(command);
    std::string line;
    while (getline(check1, line, ' '))
    {
        tokens.push_back(line);
    }
    return tokens;
}

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
    date::year_month_day tempServerDate;
    std::cin >> date::parse("%F", tempServerDate);
    if (!std::cin)
    {
        std::cin.clear();
        return false;
    }
    return true;
}

void Server::start()
{

    int server_fd, max_sd , new_socket;
    char buffer[1024] = {0};
    fd_set master_set, working_set;

    server_fd = setupServer();

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    printf("server is starting ...\n");
    bool isDateCorrect = false;
    while (!isDateCorrect)
    {
        std::string date;
        printf("Enter the date:\n");
        std::getline(std::cin, date);
        std::vector<std::string> tokens = tokenizeCommand(date);
        if (tokens.size() != 2 || tokens[0] != SET_TIME)
        {
            std::cout << BAD_SEQUENCE_OF_COMMANDS << std::endl;
            break;
        }
        else
        {
            std::string command = tokens[0];
            std::string date = tokens[1];
            if (!checkDate(date))
            {
                std::cout << INVALID_VALUE << std::endl;
                break;
            }
            setServerDate(date);
            isDateCorrect = true;
        }
    }

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {

                if (i == server_fd)
                { // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }

                else
                { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i, buffer, 1024, 0);
                    nlohmann::json message = json::parse(buffer);
                    commandHandler_.runCommand(message, new_socket);
                    if (bytes_received == 0)
                    { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, 1024);
                }
            }
        }
    }
}

int main()
{
    Server server;
    server.start();
}