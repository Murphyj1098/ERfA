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
  {
    sockAddr_ = "/tmp/argos_emane_node_" + std::to_string(id_-1);
    emane0Addr_ = "10.100.0." + std::to_string(id_); // NOTE: Assume we use convention of IP addr = 10.100.0.${nem_id}
  }

  Relay::~Relay()
  {
    if(argosThread_.joinable())
    {
      pthread_cancel(argosThread_.native_handle());
      argosThread_.join();
    }
    if(emaneThread_.joinable())
    {
      pthread_cancel(emaneThread_.native_handle());
      emaneThread_.join();
    }
    pARGoSSock_->shutdown(SHUT_RDWR);
    pEMANESock_->shutdown(SHUT_RDWR);
    pARGoSSock_->close();
    pEMANESock_->close();
  }

  void Relay::doInit()
  {
    std::cout<<"Doing initialization for NEM-"<<id_<<std::endl;

    // Set up ARGoS-side socket
    pARGoSSock_ = new Socket();
    
    if((pARGoSSock_->create(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      std::cerr<<"Failed to create ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    if((pARGoSSock_->bind(sockAddr_, "")) < 0)
    {
      std::cerr<<"Failed to bind ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    // Set up EMANE-side receive socket (sending happens differently)
    pEMANESock_ = new Socket();
    
    if((pEMANESock_->create(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      std::cerr<<"Failed to create EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }

    if((pEMANESock_->bind("127.0.0.1", EMANE_SOCK_PORT)) < 0)
    {
      std::cerr<<"Failed to bind EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }
  }

  void Relay::doStart()
  {
    std::cout<<"Doing start"<<std::endl;
    
    // Create thread for passing from ARGoS to EMANE
    argosThread_ = std::thread(&Relay::toARGoS, this);

    // Create thread for passing from EMANE to ARGoS
    emaneThread_ = std::thread(&Relay::toEMANE, this);
  }

  void Relay::doStop()
  {
    std::cout<<"Doing stop"<<std::endl;

    if(argosThread_.joinable())
    {
      pthread_cancel(argosThread_.native_handle());
      argosThread_.join();
    }
    if(emaneThread_.joinable())
    {
      pthread_cancel(emaneThread_.native_handle());
      emaneThread_.join();
    }
    pARGoSSock_->shutdown(SHUT_RDWR);
    pEMANESock_->shutdown(SHUT_RDWR);
    pARGoSSock_->close();
    pEMANESock_->close();
  }

  void Relay::doDestroy()
  {
    std::cout<<"Doing final destruction"<<std::endl;
  }

  /**********************************************************************************************/
  /**********************************************************************************************/

  // TODO:
  void Relay::toARGoS()
  {
    // loop read full package from pEMANESock_
    // send full package into pARGoSSock_
  }

  // TODO:
  void Relay::toEMANE()
  {
    // read in:
      // a. SRC ID
      // b. DST ID
      // c. Payload len
      // d. Payload buffer

    // create a 

    // write full package in segments of (???: some) size
  }
}