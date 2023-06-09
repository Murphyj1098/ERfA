#include "relay.hpp"

#include <iostream>

namespace emane_relay {

  /**
   * @class Relay
   * 
   * @brief Main application class
   */
  Relay::Relay()
  {}

  Relay::~Relay()
  {}

  void Relay::doInit()
  {
    std::cout<<"Doing initialization"<<std::endl;

    // Set up rx and tx threads
  }

  void Relay::doStart()
  {
    std::cout<<"Doing start"<<std::endl;

    // Start rx and tx thread relaying
  }

  void Relay::doStop()
  {
    std::cout<<"Doing stop"<<std::endl;
  }

  void Relay::doDestroy()
  {
    std::cout<<"Doing destruction"<<std::endl;
  }
}