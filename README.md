# Neighbor Discovery Service

A service which scans for devices in local network that are also running the same service. Built on top of Linux socket API.

## How it works

Service starts a **ResponderService** in a separate thread which:
1. Opens a socket on one of ports in range 4320-4330 (first available)
2. Waits for a connection
3. Responds with a service ID when connection is made
4. Repeat from 2.

Service starts a **DiscoveryService** in the main thread which:
1. Discovers local IP addresses by sending ICMP request (ping) to the all host multicast address 224.0.0.1
2. For each address and each port in the range 4320-4330 creates a TCP/IP connection and expects to receive message with our service ID
3. For each discovered service, gets a MAC address from local ARP cache
4. Stores the IP and MAC addresses of neighbors in a file `neighbors.txt`
5. Repeats the discovery process every 30 seconds

## Requirements
- g++ (should support at lest C++11)
- make
- Since the discovery depends on devices answering to broadcast pings, it is expected that device can answer such pings

    In case it's disabled, enable responding to pings by running command:

        sudo sysctl net.ipv4.icmp_echo_ignore_broadcasts=0

    Alternatively, I could have scanned all of the IPs in a subnet, but I though that this is a more elegant solution in case where we can ensure that devices will respond to the broadcasts.

## Build

    make

## Run the service

    sudo ./service

Note: Elevated permissions (sudo) are required by "RAW" socket used in ICMP requests.

## Show the neighbors
Since the neighbors are written to file, I skipped creating a CLI tool for reading the file since it's a very trivial task. Instead, just look into the contents of the file:

    cat ./neighbors.txt

Note: In a more serious service the neighbors would be stored differently and then a CLI tool would be required to show them.

## Logging
The services are logging what they are doing into separate log files:
- `./discovery_service.log`
- `./responder_service.log`