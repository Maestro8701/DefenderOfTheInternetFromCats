#include "udp_communicator.h"
#include "alarm_system.h"
#include <iostream>

UDPCommunicator::UDPCommunicator(int port) : UDP_PORT(port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed");
    }

    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    phoneAddr.sin_family = AF_INET;
    phoneAddr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, "192.168.1.100", &phoneAddr.sin_addr);
}

UDPCommunicator::~UDPCommunicator() {
    closesocket(sock);
    WSACleanup();
}

void UDPCommunicator::sendMessage(const std::string& msg) {
    sendto(sock, msg.c_str(), (int)msg.length(), 0, (sockaddr*)&phoneAddr, sizeof(phoneAddr));
}

void UDPCommunicator::checkForCommands() {
    char buffer[512];
    int addrLen = sizeof(phoneAddr);
    int resp = recvfrom(sock, buffer, 511, 0, (sockaddr*)&phoneAddr, &addrLen);
    if (resp > 0) {
        buffer[resp] = '\0';
        if (std::string(buffer) == "ALARM_OFF") {
            std::cout << "Remote Reset!" << std::endl;
            AlarmSystem::handleAlarm(false, *this);
        }
    }
}
