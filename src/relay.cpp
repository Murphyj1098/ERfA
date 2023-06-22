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
  {}

  void Relay::doInit()
  {
    std::cout<<"Doing initialization for NEM-"<<id_<<std::endl;

    // Set up ARGoS-side socket
    Socket * socketFacilitator = new Socket();
    // pARGoSSock_ = new Socket();
    
    if((socketFacilitator->create(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      std::cerr<<"Failed to create ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    if((socketFacilitator->set_blocking()) < 0)
    {
      std::cerr<<"Failed to set ARGoS-side socket to blocking for NEM-"<<id_<<std::endl;
    }

    if((socketFacilitator->bind(sockAddr_, "")) < 0)
    {
      std::cerr<<"Failed to bind ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    // Should only listen for one socket (0 backlog)
    if((socketFacilitator->listen(0)) < 0)
    {
      std::cerr<<"Failed to listen on ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    // NOTE: We block here until ARGoS connects to us
    pARGoSSock_ = socketFacilitator->accept();
    socketFacilitator->close();


    // Set up EMANE-side receive socket (sending happens differently)
    pEMANESock_ = new Socket();
    
    if((pEMANESock_->create(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      std::cerr<<"Failed to create EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }

    if((pEMANESock_->set_blocking()) < 0)
    {
      std::cerr<<"Failed to set EMANE-side rx socket to blocking for NEM-"<<id_<<std::endl;
    }

    if((pEMANESock_->bind("127.0.0.1", EMANE_SOCK_PORT)) < 0) // TODO: Change this address?
    {
      std::cerr<<"Failed to bind EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }

  }

  void Relay::doStart()
  {
    std::cout<<"Doing start"<<std::endl;
    
    // Create thread for passing from ARGoS to EMANE
    toARGoSThread_ = std::thread(&Relay::toARGoS, this);
    fromARGoSThread_ = std::thread(&Relay::fromARGoS, this);

    // Create thread for passing from EMANE to ARGoS
    toEMANEThread_ = std::thread(&Relay::toEMANE, this);
    fromEMANEThread_ = std::thread(&Relay::fromEMANE, this);
  }

  void Relay::doStop()
  {
    std::cout<<"Doing stop"<<std::endl;

    if(toARGoSThread_.joinable())
    {
      pthread_cancel(toARGoSThread_.native_handle());
      toARGoSThread_.join();
    }
    if(fromARGoSThread_.joinable())
    {
      pthread_cancel(fromARGoSThread_.native_handle());
      fromARGoSThread_.join();
    }
    if(toEMANEThread_.joinable())
    {
      pthread_cancel(toEMANEThread_.native_handle());
      toEMANEThread_.join();
    }
    if(fromEMANEThread_.joinable())
    {
      pthread_cancel(fromEMANEThread_.native_handle());
      fromEMANEThread_.join();
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

  void Relay::toARGoS()
  {
    while(true)
    {
      SMessage sTransmittedMessage;

      // Wait until message exists in queue
      if(!emaneQueue_.empty())
      {
        // Lock ARGoS queue
        std::lock_guard<std::mutex> lock(emaneQueueLock_);

        // Get message out of queue into send buffer
        sTransmittedMessage = std::move(emaneQueue_.front());
        //HACK: Is there a better way to get the size than recreating this buffer each loop?
        uint8_t* pARGoSTransmitBuf = new uint8_t[sizeof(sTransmittedMessage)];
        memcpy(pARGoSTransmitBuf, &sTransmittedMessage, sizeof(sTransmittedMessage));
        pARGoSSock_->write_socket(pARGoSTransmitBuf);

        // Write message from buffer onto pARGoSSock_
        emaneQueue_.pop();
      }
    }
  }

  void Relay::fromARGoS()
  {
    SPreamble sReceivedPreamble;
    SMessage  sReceivedMessage;

    ssize_t preambleSize = sizeof(sReceivedPreamble);

    uint8_t * pARGoSPreambleBuf = new uint8_t[preambleSize];

    while(true)
    {
      // Read the preamble
      pARGoSSock_->read_socket(pARGoSPreambleBuf, preambleSize);

      // Cast to SPreamble and get payload size
      sReceivedPreamble = std::move(*reinterpret_cast<SPreamble*>(pARGoSPreambleBuf));
      
      // Receive rest of message
      //HACK: Is there a better way to get the size than recreating this buffer each loop?
      uint8_t * pARGoSReceiveBuf = new uint8_t[preambleSize + sReceivedPreamble.payloadSize];
      pARGoSReceiveBuf += preambleSize; // move pointer
      pARGoSSock_->read_socket(pARGoSReceiveBuf, sReceivedPreamble.payloadSize);
      pARGoSReceiveBuf -= preambleSize; // move pointer back
      memcpy(pARGoSReceiveBuf, pARGoSPreambleBuf, preambleSize);

      // Put SMessage into queue
      std::lock_guard<std::mutex> lock(argosQueueLock_);

      sReceivedMessage = std::move(*reinterpret_cast<SMessage*>(pARGoSReceiveBuf));
      argosQueue_.push(sReceivedMessage);
    }
  }

  void Relay::toEMANE()
  {
    SMessage sTransmittedMessage;
    Socket * sendSock;
    std::string dstAddress;

    while(true)
    {
      if(argosQueue_.empty()) { continue; }

      // Lock ARGoS queue
      std::lock_guard<std::mutex> lock(argosQueueLock_);
      // Get message out of queue into send buffer
      sTransmittedMessage = std::move(argosQueue_.front());
      //HACK: Is there a better way to get the size than recreating this buffer each loop?
      uint8_t* pEMANETransmitBuf = new uint8_t[sizeof(sTransmittedMessage)];
      // Write message from buffer onto pARGoSSock_
      argosQueue_.pop();

      dstAddress = "10.100.0." + std::to_string(sTransmittedMessage.dst);

      sendSock->create(AF_INET, SOCK_STREAM, 0);
      sendSock->connect(dstAddress, EMANE_SOCK_PORT);
      sendSock->write_socket(pEMANETransmitBuf);
      sendSock->shutdown(SHUT_RDWR);
      sendSock->close();
    }
  }

  void Relay::fromEMANE()
  {
    Socket * pCurrSock;

    SPreamble sReceivedPreamble;
    SMessage  sReceivedMessage;

    ssize_t preambleSize = sizeof(sReceivedPreamble);
    uint8_t * pEMANEPreambleBuf = new uint8_t[preambleSize];

    pEMANESock_->listen(5); // ???: Is 5 connections a large enough backlog?

    while(true) {
      // Block on accepting server socket
      pCurrSock = pEMANESock_->accept();

      // Read the preamble
      pARGoSSock_->read_socket(pEMANEPreambleBuf, preambleSize);

      // Cast to SPreamble and get payload size
      sReceivedPreamble = std::move(*reinterpret_cast<SPreamble*>(pEMANEPreambleBuf));
      
      // Receive rest of message
      //HACK: Is there a better way to get the size than recreating this buffer each loop?
      uint8_t * pEMANEReceiveBuf = new uint8_t[preambleSize + sReceivedPreamble.payloadSize];
      pEMANEReceiveBuf += preambleSize; // move pointer
      pARGoSSock_->read_socket(pEMANEReceiveBuf, sReceivedPreamble.payloadSize);
      pEMANEReceiveBuf -= preambleSize; // move pointer back
      memcpy(pEMANEReceiveBuf, pEMANEPreambleBuf, preambleSize);

      // Put SMessage into queue
      std::lock_guard<std::mutex> lock(emaneQueueLock_);

      sReceivedMessage = std::move(*reinterpret_cast<SMessage*>(pEMANEReceiveBuf));
      emaneQueue_.push(sReceivedMessage);

      pCurrSock->close();
    }
  }
}