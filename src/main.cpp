#include <iostream>
#include <vector>
#include <getopt.h>
#include "socket/socket.hpp"

namespace {
  const std::string VERSION{"v0.0.1"};

  void usage()
  {
    std::cout<<"EMANE-Relay Usage"<<std::endl;
    std::cout<<std::endl;
  }
}
int main(int argc, char * argv[]) {
  // Parse startup arguments
  // required args:
  // EMANE NEM ID (???: derive from ${node-name})

  // optional args:
  // --pidfile FILE
  // -f, --logfile FILE
  // -l, --loglevel [0,4]
  // -d, --daemonize
  std::cout<<"Initialzing EMANE Relay for ARGoS"<<std::endl;

  std::vector<option> options = 
  {
    {"help",      0, nullptr, 'h'},
    {"loglevel",  1, nullptr, 'l'},
    {"logfile",   1, nullptr, 'f'},
    {"daemonize", 0, nullptr, 'd'},
    {"version",   0, nullptr, 'v'},
    {"pidfile",   1, nullptr,   1},
    {0,           0, nullptr,   0}
  };

  std::string sOptString{"hdl:f:"};

  int iOption{};
  int iOptionIndex{};
  bool bDaemonize{};
  int iLogLevel{};
  std::string sLogFile{};
  std::string sPIDFile{};

  while((iOption = getopt_long(argc, argv, sOptString.c_str(), &options[0], &iOptionIndex)) != -1)
  {
    switch(iOption)
    {
      case 'h':
        usage();
        return 0;

      case 'l':
        // TODO: need to convert optarg to int (see EMANE::Utils::ParameterConvert)
        std::cout<<"loglevel: Not Yet Implemented"<<std::endl;
        return 1;

      case 'f':
        sLogFile = optarg;
        break;

      case 'd':
        bDaemonize = true;
        break;

      case 'v':
        std::cout<<VERSION<<std::endl;
        return 0;

      case 1:
        sPIDFile = optarg;
        break;

      default:
        return -1;
    }
  }

  // Set up handlers for graceful exit

  // Create thread for receiving socket (AF_UNIX)
  // Create thread for sending socket (AF_INET)

  return 0;
}