#include "socket.hpp"

#include <fcntl.h>

namespace emane_relay {

  /**
   * @class Socket
   * 
   * @brief Socket service provider
   * 
   * Socket class based on https://github.com/KMakowsky/Socket.cpp
   */
  Socket::Socket():
    sock_(-1),
    sockAddr_(""),
    sockPort_("")
  {}

  Socket::~Socket()
  {
    // Shutdown socket and close
    ::shutdown(sock_, SHUT_RDWR);
    ::close(sock_);
  }

  int Socket::create(const int domain, const int type, const int protocol)
  {
    // open socket file descriptor
    if ((sock_ = ::socket(domain, type, protocol)) < 0)
    {
      // fail to create
      return -1;
    }

    addressInfo_.ai_family = domain;
    addressInfo_.ai_socktype = type;
    addressInfo_.ai_protocol = protocol;

    // success
    return 0;
  }

  int Socket::bind(const char* address, const char* port)
  {
      // local socket
      if (addressInfo_.ai_family == AF_UNIX)
      {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, address, sizeof(addr.sun_path)-1);

        // bind socket using provided path
        if ((::bind(sock_, (struct sockaddr*)&addr, sizeof(addr))) < 0)
        {
          // fail to bind
          return -1;
        }

        // success
        return 0;
      }

      sockAddr_ = address;
      sockPort_ = port;

      struct addrinfo *res;
      addressInfo_.ai_flags = AI_PASSIVE;
      if((getaddrinfo(address, port, &addressInfo_, &res)) < 0)
      {
        // fail to bind
        return -1;
      }

    addressInfo_.ai_addrlen = res->ai_addrlen;
    addressInfo_.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    if ((::bind(sock_, addressInfo_.ai_addr, addressInfo_.ai_addrlen)) < 0)
    {

      // fail to bind
      return -1;
    }

    // success
    return 0;
  }

  int Socket::connect(const char* address, const char* port)
  {
    // local socket
    if (addressInfo_.ai_family == AF_UNIX)
    {
      struct sockaddr_un addr;
      memset(&addr, 0, sizeof(addr));
      addr.sun_family = AF_UNIX;
      strncpy(addr.sun_path, address, sizeof(addr.sun_path)-1);

      // connect socket using provided path
      if ((::connect(sock_, (struct sockaddr*)&addr, sizeof(addr))) < 0)
      {
        // fail to connect
        return -1;
      }

      // success
      return 0;
    }

    sockAddr_ = address;
    sockPort_ = port;

    struct addrinfo *res;
    if((getaddrinfo(address, port, &addressInfo_, &res)) < 0)
    {
      // fail to connect
      return -1;
    }

    addressInfo_.ai_addrlen = res->ai_addrlen;
    addressInfo_.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    if ((::connect(sock_, addressInfo_.ai_addr, addressInfo_.ai_addrlen)) < 0)
    {
      // fail to connect
      return -1;
    }

    // success
    return 0;
  }

  int Socket::listen(int max_queue)
  {
    if((::listen(sock_, max_queue)))
    {
      // fail to listen
      return -1;
    }

    // success
    return 0;
  }

  Socket* Socket::accept()
  {
    struct sockaddr_storage clientAddr;
    socklen_t addrSize;
    addrSize = sizeof(clientAddr);
    int newSock;

    if ((newSock = ::accept(sock_, (struct sockaddr *)&clientAddr, &addrSize)) < 0)
    {
      // TODO: How do we gracefully exit if accept failed
      // Function is expecting to return an instance of this class
    }

    Socket *newSocket = new Socket();

    if((newSocket->create(addressInfo_.ai_family, addressInfo_.ai_socktype, addressInfo_.ai_protocol)) < 0)
    {
      // TODO: How do we gracefully exit if accept failed
      // Function is expecting to return an instance of this class
    }

    newSocket->sock_ = newSock;
    newSocket->sockPort_= sockPort_;

    char host[NI_MAXHOST];

    if((getnameinfo((struct sockaddr *)&clientAddr, addrSize, host, sizeof(host), NULL, 0, NI_NUMERICHOST)) < 0)
    {
      // TODO: How do we gracefully exit if accept failed
      // Function is expecting to return an instance of this class
    }

    newSocket->sockAddr_ = host;
    newSocket->addressInfo_.ai_family = clientAddr.ss_family;
    newSocket->addressInfo_.ai_addr = (struct sockaddr *)&clientAddr;

    return newSocket;
  }

  int Socket::read_socket(char &buf, int len)
  {
    char buffer[len];
    bzero(buffer, len);

    int bRecv = (int)recv(sock_, buffer, len-1, 0);

    if(bRecv < 0)
    {
      // fail to recv
      return -1;
    }

    // success
    return bRecv;
  }

  int Socket::write_socket(const char* msg)
  {
    int len = (int)strlen(msg);

    if(((int)send(sock_, msg, len, 0)) < 0)
    {
      // fail to write
      return -1;
    }

    // success
    return 0;
  }

  int Socket::set_blocking()
  {
    long status = fcntl(sock_, F_GETFL, NULL);

    if(status < 0) { return -1; }

    status &= (~O_NONBLOCK);
    status = fcntl(sock_, F_SETFL, status);

    int retVal = (status < 0) ? -1 : 0;
    return retVal;
  }

  int Socket::set_nblocking()
  {
    long status = fcntl(sock_, F_GETFL, NULL);

    if(status < 0) { return -1; }

    status |= (O_NONBLOCK);
    status = fcntl(sock_, F_SETFL, status);

    int retVal = (status < 0) ? -1 : 0;
    return retVal;
  }

  int Socket::shutdown(int how)
  {
    if((::shutdown(sock_, how)) < 0)
    {
      // fail to write
      return -1;
    }

    // success
    return 0;
  }

  int Socket::close()
  {
    ::close(sock_);
    return 0;
  }
}