#include <iostream>
#include "socket/socket.hpp"

namespace emane_relay {
    int createSock() {
        Socket *pSock_ = new Socket();

        return 0;
    }
}

int main(int argc, char* argv[]) {
  std::cout << "Initialzing EMANE Relay for ARGoS\n";

  // Parse startup arguments
    // required args:
    // EMANE NEM ID (derive from ${node-name}???)

    // optional args:
    // --pidfile FILE
    // -f, --logfile FILE
    // -l, --loglevel [0,4]
    // -d, --daemonize

  // Set up handlers for graceful exit

  // Create thread for receiving socket (AF_UNIX)
  // Create thread for sending socket (AF_INET)

  return 0;
}