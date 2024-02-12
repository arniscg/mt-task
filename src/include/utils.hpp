#pragma once

#include <vector>
#include <string>

/**
 * Return gateway IP addresses of each interface.
 * Need these IPs to ignore them.
*/
std::vector<std::string> getGatewayAddresses();

/**
 * Return local interface IP addresses.
 * Need these IPs to ignore own IP in discovery.
*/
std::vector<std::string> getLocalAddresses();

/**
 * Split a space-separated string into words
*/
std::vector<std::string> splitString(std::string str);
