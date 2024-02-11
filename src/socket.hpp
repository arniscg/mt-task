#include <netinet/ip.h>
#include <optional>
#include <tuple>
#include <string>

/**
 * Wrapper around Linux socket API
*/
class Socket {
    public:
        int fd;

        Socket();
        Socket(int fd);
        Socket(int type, int protocol);

        void bindSock(in_addr_t &ipAddr, int port);
        void startListening();
        std::optional<std::tuple<Socket,std::string>> waitConnection();
        void writeMessage(std::string msg);
        void connectTo(std::string ip, int port);
        void setReadTimeout(int timeout);
        void setSendTimeout(int timeout);
        std::optional<std::string> waitMessage();

        ~Socket();
};