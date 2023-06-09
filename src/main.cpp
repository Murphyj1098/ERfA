#include <getopt.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "relay.hpp"

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

  std::string sOptString{"hdvl:f:"};

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
        std::cerr<<"loglevel: Not Yet Implemented"<<std::endl;
        return EXIT_SUCCESS;

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
        return EXIT_FAILURE;
    }
  }

  // Daemonize process
  if(bDaemonize)
  {
    if(sLogFile.empty() && iLogLevel != 0)
    {
      std::cerr<<"Program cannot daemonize, log level must be 0 if logging to stdout"<<std::endl;
      return EXIT_FAILURE;
    }

    if(daemon(0,1))
    {
      std::cerr<<"unable to daemonize"<<std::endl;
      return EXIT_FAILURE;
    }
  }

  // TODO: Set up logfile / logger
  // TODO: Create PID File


  // Start main application
  emane_relay::Relay * Application = new emane_relay::Relay();

  Application->doInit();
  Application->doStart();

  // NOTE: Application exit signal handlers here

  Application->doStop();
  Application->doDestroy();

  return EXIT_SUCCESS;
}