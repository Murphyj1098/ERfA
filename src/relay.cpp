#include "relay.hpp"

#include <iostream>

#include <limits.h>
#include <sys/stat.h>

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
    emane0Addr_ = "10.100.0." + std::to_string(id_);  // Assume EMANE convetion Radio IP = 10.100.0.${nem_id}/24
                                                      // Value found in EMANE configuration files
  }

  Relay::~Relay()
  {}

  void Relay::doInit()
  {
    std::cout<<"Doing initialization for NEM-"<<id_<<std::endl;

    // Set up EMANE-side receive socket (sending doesn't use persistent socket)
    pEMANESock_ = new Socket();
    
    if((pEMANESock_->create(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      std::cerr<<"Failed to create EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }

    if((pEMANESock_->bind(emane0Addr_, EMANE_SOCK_PORT)) < 0)
    {
      std::cerr<<"Failed to bind EMANE-side rx socket for NEM-"<<id_<<std::endl;
    }

    // Set up ARGoS-side socket
    Socket * socketFacilitator = new Socket();
    
    if((socketFacilitator->create(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      std::cerr<<"Failed to create ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    // EMANE runs this program as root, change umask to give permission to non-root processes
    mode_t oldMask = umask(0);
    if((socketFacilitator->bind(sockAddr_, "")) < 0)
    {
      std::cerr<<"Failed to bind ARGoS-side socket for NEM-"<<id_<<std::endl;
    }
    umask(oldMask);

    // Should only listen for one socket (0 backlog)
    if((socketFacilitator->listen(0)) < 0)
    {
      std::cerr<<"Failed to listen on ARGoS-side socket for NEM-"<<id_<<std::endl;
    }

    // NOTE: We block here until ARGoS connects to us
    pARGoSSock_ = socketFacilitator->accept();
  }

  void Relay::doStart()
  {
    std::cout<<"Doing start"<<std::endl;
    
    toARGoSThread_ = std::thread(&Relay::toARGoS, this);
    fromARGoSThread_ = std::thread(&Relay::fromARGoS, this);
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
    SPreamble sTransmittedPreamble;
    SMessage  sTransmittedMessage;

    ssize_t preambleSize = sizeof(sTransmittedPreamble);

    uint8_t* pARGoSPayloadBuf;
    uint8_t* pARGoSTransmitBuf;

    while(true)
    {
      if(emaneQueue_.empty()) { continue; }
     
      // Lock ARGoS queue and get front
      std::unique_lock<std::mutex> lock(emaneQueueLock_);
      sTransmittedMessage = std::move(emaneQueue_.front());
      emaneQueue_.pop();
      lock.unlock();

      std::cout
      <<"Sent to ARGoS\n"
      <<" time: "<<sTransmittedMessage.timestamp
      <<" src: "<<sTransmittedMessage.src
      <<" dst: "<<sTransmittedMessage.dst
      <<" size: "<<sTransmittedMessage.payloadSize
      <<std::endl;

      // Convert vector to byte array
      pARGoSPayloadBuf = new uint8_t[sTransmittedMessage.payload.size()];
      std::copy(sTransmittedMessage.payload.begin(), sTransmittedMessage.payload.end(), pARGoSPayloadBuf);

      pARGoSTransmitBuf = new uint8_t[preambleSize + sizeof(pARGoSPayloadBuf)];
      memcpy(pARGoSTransmitBuf, &sTransmittedMessage, preambleSize);
      pARGoSTransmitBuf += preambleSize;
      memcpy(pARGoSTransmitBuf, pARGoSPayloadBuf, sizeof(pARGoSPayloadBuf));
      pARGoSTransmitBuf -= preambleSize;

      // Write message from buffer onto pARGoSSock_
      pARGoSSock_->write_socket(pARGoSTransmitBuf);
    }
  }

  void Relay::fromARGoS()
  {
    SPreamble sReceivedPreamble;
    SMessage  sReceivedMessage;

    ssize_t preambleSize = sizeof(sReceivedPreamble);

    uint8_t * pARGoSPreambleBuf = new uint8_t[preambleSize];
    uint8_t * pARGoSPayloadBuf;

    while(true)
    {
      // Receive the preamble and cast to SPreamble
      pARGoSSock_->read_socket(pARGoSPreambleBuf, preambleSize);
      sReceivedPreamble = std::move(*reinterpret_cast<SPreamble*>(pARGoSPreambleBuf));

      std::cout
      <<"Received from ARGoS\n"
      <<" time: "<<sReceivedPreamble.timestamp
      <<" src: "<<sReceivedPreamble.src
      <<" dst: "<<sReceivedPreamble.dst
      <<" size: "<<sReceivedPreamble.payloadSize
      <<std::endl;

      // Receive rest of message (payload)
      pARGoSPayloadBuf = new uint8_t[sReceivedPreamble.payloadSize];
      pARGoSSock_->read_socket(pARGoSPayloadBuf, sReceivedPreamble.payloadSize);

      // Copy preamble and payload into SMessage struct
      sReceivedMessage.CopyPreamble(sReceivedPreamble);
      sReceivedMessage.payload = std::vector<uint8_t>(pARGoSPayloadBuf, pARGoSPayloadBuf+sReceivedPreamble.payloadSize);

      // Put SMessage into queue
      std::unique_lock<std::mutex> lock(argosQueueLock_);
      argosQueue_.push(std::move(sReceivedMessage));
    }
  }

  void Relay::toEMANE()
  {
    SPreamble   sTransmittedPreamble;
    SMessage    sTransmittedMessage;
    Socket *    sendSock;
    std::string dstAddress;

    ssize_t preambleSize = sizeof(sTransmittedPreamble);

    uint8_t* pEMANEPayloadBuf;
    uint8_t* pEMANETransmitBuf;

    while(true)
    {
      if(argosQueue_.empty()) { continue; }

      // Lock ARGoS queue and get front
      std::unique_lock<std::mutex> lock(argosQueueLock_);
      sTransmittedMessage = std::move(argosQueue_.front()); 
      argosQueue_.pop();
      lock.unlock();

      // Create socket to receiving EMANE NEM
      dstAddress = "10.100.0." + std::to_string(sTransmittedMessage.dst+1);     // ARGoS IDs are one less than EMANE IDs (EMANE indexes at 1)
      if(sTransmittedMessage.dst == USHRT_MAX) { dstAddress = "10.100.0.254"; } // Special gateway node case

      // Not yet handling gateway case (gw doesn't connect to ARGoS and needs special handling)
      
      if(sTransmittedMessage.dst == USHRT_MAX) { continue; }

      sendSock = new Socket();
      sendSock->create(AF_INET, SOCK_STREAM, 0);
      if((sendSock->connect(dstAddress, EMANE_SOCK_PORT)) < 0)
      {
        std::cout<<"Failed to connect to "<<dstAddress<<" on toEMANE: "<<::strerror(errno)<<std::endl;
        continue; // Can't reach the node, assume the packet is "lost"
      }

      // Copy payload vector into byte array
      pEMANEPayloadBuf = new uint8_t[sTransmittedMessage.payload.size()];
      std::copy(sTransmittedMessage.payload.begin(), sTransmittedMessage.payload.end(), pEMANEPayloadBuf);

      pEMANETransmitBuf = new uint8_t[preambleSize + sizeof(pEMANEPayloadBuf)];
      memcpy(pEMANETransmitBuf, &sTransmittedMessage, preambleSize);
      pEMANETransmitBuf += preambleSize;
      memcpy(pEMANETransmitBuf, pEMANEPayloadBuf, sizeof(pEMANEPayloadBuf));
      pEMANETransmitBuf -= preambleSize;

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
    uint8_t * pEMANEPayloadBuf;

    pEMANESock_->listen(5);

    while(true)
    {
      // Wait until other NEM initiates connection
      pCurrSock = pEMANESock_->accept();

      // Read the preamble and cast to SPreamble
      pCurrSock->read_socket(pEMANEPreambleBuf, preambleSize);
      sReceivedPreamble = std::move(*reinterpret_cast<SPreamble*>(pEMANEPreambleBuf));

      // Receive rest of message
      pEMANEPayloadBuf = new uint8_t[sReceivedPreamble.payloadSize];
      pCurrSock->read_socket(pEMANEPayloadBuf, sReceivedPreamble.payloadSize);

      // Copy preamble and payload into SMessage struct
      sReceivedMessage.CopyPreamble(sReceivedPreamble);
      sReceivedMessage.payload = std::vector<uint8_t>(pEMANEPayloadBuf, pEMANEPayloadBuf+sReceivedPreamble.payloadSize);

      // Put SMessage into queue
      std::unique_lock<std::mutex> lock(emaneQueueLock_);
      emaneQueue_.push(std::move(sReceivedMessage));
      pCurrSock->close();
    }
  }
}