#include <vector>
#include <string>
#include <netinet/ip_icmp.h> 

#define PING_PKT_SIZE 64
#define RECV_TIMEOUT 1

struct PingPacket {
    icmphdr hdr;
    char msg[PING_PKT_SIZE - sizeof(icmphdr)];
};

/**
 * Get IP addresses of devices in local network.
 * This method utilizes 224.0.0.1 multicast address.
 * By sending a ICMP request to this address,
 *  it is expected that all the devices in the local network will respond.
 * 
 * Note: This behaviour might be blocked by firewall.
 *  In such case different approach would need to be implemented,
 *  for example, iterating over all possible addresses in a subnet.
*/
std::vector<std::string> scanLocalIps();

/**
 * Calculate a checksum for ICMP packet
*/
unsigned short checksum(void* b, int len);