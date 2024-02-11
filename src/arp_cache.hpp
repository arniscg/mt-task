#include <string>
#include <vector>

struct ArpEntry {
    std::string ipAddress;
    std::string hwAddress;
    std::string device;
};

/**
 * Split ARP cache entries into separate strings - ip, mac address (hw address), and device name
 * 
 * Expected ARP cache format:
 * IP address       HW type     Flags       HW address            Mask     Device
 * 192.168.0.111    0x1         0x0         00:00:00:00:00:00     *        enp34s0
 * 192.168.0.1      0x1         0x2         e8:48:b8:f9:21:a3     *        enp34s0
*/
std::vector<ArpEntry> parseArpCache();

/**
 * Find a Mac address of an IP address
*/
std::string arpMacAddressLookup(std::string ipAddress);