#pragma once

#include <netinet/ip.h>
#include <tuple>
#include <string>
#include <functional>

typedef void (*LoggerFunction)(std::string msg); 

/**
 * Wrapper around Linux socket API
*/
class Socket {
    public:
        int fd;

        Socket();
        Socket(int fd);
        Socket(int type, int protocol);

        void setLogger(std::function<void(std::string)> log);
        void bindSock(in_addr_t &ipAddr, int port);
        void startListening();
        Socket waitConnection();
        void writeMessage(std::string msg);
        void connectTo(std::string ip, int port);
        void setReadTimeout(int timeout);
        void setSendTimeout(int timeout);
        std::string waitMessage();

        ~Socket();

    private:
        std::function<void(std::string)> log = [](std::string){};
};