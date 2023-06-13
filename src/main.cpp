#include <getopt.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "relay.hpp"
#include "logger.hpp"

namespace {
  const std::string VERSION{"v0.0.1"};

  void usage()
  {
    std::cout<<"usage: emanerelayd [OPTIONS]... NEM_ID"<<std::endl;
    std::cout<<std::endl;
    std::cout<<" NEM_ID                          ID number assigned to EMANE NEM"<<std::endl;
    std::cout<<std::endl;
    std::cout<<"options:"<<std::endl;
    std::cout<<"  -d, --daemonize                Run in the background."<<std::endl;
    std::cout<<"  -h, --help                     Print this message and exit."<<std::endl;
    std::cout<<"  -f, --logfile FILE             Log to a file instead of stdout."<<std::endl;
    std::cout<<"  -l, --loglevel [0,4]           Set initial log level."<<std::endl;
    std::cout<<"                                  default: 2"<<std::endl;
    std::cout<<"  --pidfile FILE                 Write application pid to file."<<std::endl;
    std::cout<<"  -v, --version                  Print version and exit."<<std::endl;
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
  emane_relay::NEMId id{};

  while((iOption = getopt_long(argc, argv, sOptString.c_str(), &options[0], &iOptionIndex)) != -1)
  {
    switch(iOption)
    {
      case 'h':
        usage();
        return 0;

      case 'l':
        try // check log level is int
        {
          iLogLevel = std::stoi(optarg);
        }
        catch(...)
        {
          std::cerr<<"invalid log level: "<<optarg<<std::endl;
          return EXIT_FAILURE;
        }

        if(iLogLevel > 4 || iLogLevel < 0) // check log level is [0,4]
        {
          std::cerr<<"invalid log level: "<<optarg<<std::endl;
          return EXIT_FAILURE;
        }
        break;

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

  // Check if non-optional parameter exists
  if(optind >= argc)
  {
    std::cerr<<"Missing NEM_ID"<<std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    id = std::stoi(argv[optind]);
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

  // TODO: Set up logger and logfile
  // TODO: Create PID File

  /**********************************************************************************************/
  /**********************************************************************************************/

  // Start main application
  emane_relay::Relay * application = new emane_relay::Relay(id);

  application->doInit();
  application->doStart();

  // NOTE: Application exit signal handlers here

  application->doStop();
  application->doDestroy();

  return EXIT_SUCCESS;
}