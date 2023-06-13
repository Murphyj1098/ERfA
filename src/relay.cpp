#include "relay.hpp"

#include <iostream>

namespace emane_relay {

  /**
   * @class Relay
   * 
   * @brief Main application class
   */
  Relay::Relay(NEMId id) :
  id_(id)
  {}

  Relay::~Relay()
  {}

  void Relay::doInit()
  {
    std::cout<<"Doing initialization for NEM-"<<id_<<std::endl;

    // Set up ARGoSRelay and RelayEMANE threads
    // Set up sockets for each thread
  }

  void Relay::doStart()
  {
    std::cout<<"Doing start"<<std::endl;
    // Start ARGoSRelay and RelayEMANE threads
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