
/**
 * Simple wrapper around Linux socket API for RAII purposes.
*/
class Socket {
    public:
        int fd;

        Socket();
        Socket(int fd);
        ~Socket();
};

/**
 * Handle incoming requests.
 * This uses blocking socket API!
*/
void handleRequests();

/**
 * Check if the IP address runs our service
*/
bool checkIpAddress(const char* ip);