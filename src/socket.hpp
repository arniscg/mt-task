#include <netinet/ip.h> // IPPROTO_UDP

#define LISTEN_BACKLOG 100

/**
 * Wrapper around Linux socket C API
*/
class Socket {
    public:
        Socket();
        ~Socket();

        /**
         * Handle incoming requests by returning a message.
         * This uses socket blocking API!
        */
        void handleRequests();

        /**
         * Check if given address runs our service.
        */
        void checkAddress(in_addr_t& inAddr);

    private:
        int socketfd;

        void createSocket();
        void prepareServerSocket();
        void closeSocket(int socket);
};