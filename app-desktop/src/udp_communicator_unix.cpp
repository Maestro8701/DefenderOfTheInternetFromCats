#include <iostream>
#include <cstring>

#include "udp_communicator_unix.h"

#include <arpa/inet.h>
#include <errno.h>

UDPCommunicator::UDPCommunicator() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] Failed to create UDP socket: " << strerror(errno) << std::endl;
        isInitialized = false;
        return;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        close(sock);
        isInitialized = false;
        return;
    }
    int newFlags = flags | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, newFlags) < 0) {
        close(sock);
        isInitialized = false;
        return;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ERROR] Failed to bind UDP socket: " << strerror(errno) << std::endl;
        close(sock);
        isInitialized = false;
        return;
    }

    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, default_dest_ip, &peer_addr.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid destination IP address." << std::endl;
        close(sock);
        isInitialized = false;
        return;
    }

    isInitialized = true;
    std::cout << "[OK] UDP network initialized successfully (Linux)." << std::endl;
}

UDPCommunicator::~UDPCommunicator() {
    if (isInitialized && sock >= 0) {
        close(sock);
    }
}

void UDPCommunicator::sendMessage(const std::string& msg) {
    if (!isInitialized || sock < 0) {
        std::cerr << "[WARNING] Network is not ready. Message skipped (Linux)." << std::endl;
        return;
    }

    ssize_t sent = sendto(sock, msg.c_str(), msg.length(), 0,
        (const struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (sent <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
        }
        else {
            std::cerr << "[ERROR] Failed to send UDP message: " << strerror(errno) << std::endl;
            isInitialized = false;
        }
    }
}

void UDPCommunicator::checkForCommands() {
    if (!isInitialized || sock < 0) return;

    char buffer[512] = { 0 };
    socklen_t addr_len = sizeof(buffer);
    ssize_t resp = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
        (struct sockaddr*)&serverAddr, &addr_len);
    if (resp > 0) {
        buffer[resp] = '\0';
        if (std::string(buffer) == "ALARM_OFF") {
            std::cout << "Remote Reset!" << std::endl;
        }
    }
    else if (resp < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
        }
        else {
            std::cerr << "[ERROR] Failed to receive UDP command: " << strerror(errno) << std::endl;
            isInitialized = false;
        }
    }
}
