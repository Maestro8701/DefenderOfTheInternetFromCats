#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class UDPCommunicator {
public:
    UDPCommunicator(int port);
    ~UDPCommunicator();
    void sendMessage(const std::string& msg);
    void checkForCommands();

private:
    SOCKET sock;
    sockaddr_in serverAddr;
    sockaddr_in phoneAddr;
    const int UDP_PORT;
};
