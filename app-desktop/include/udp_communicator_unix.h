#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

class UDPCommunicator {
public:
    UDPCommunicator();
    ~UDPCommunicator();
    void sendMessage(const std::string& msg);
    void checkForCommands();
    bool isReady() const { return isInitialized; }

private:
    int sock;
    struct sockaddr_in serverAddr;
    struct sockaddr_in phoneAddr;
    const int UDP_PORT = 12345;
    bool isInitialized = false;
    const char* default_dest_ip = "192.168.1.100";
};


