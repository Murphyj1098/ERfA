#ifndef EMANERELAY_SOCKET_H_
#define EMANERELAY_SOCKET_H_

#include <string>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

namespace emane_relay{
  class Socket {
    private:
      int sock_;
      std::string sockAddr_;
      std::string sockPort_;
      struct addrinfo addressInfo_;

    public:
      Socket();
      ~Socket();

      /**
       * Create server/client socket file descriptor
       * 
       * @param domain
       * @param type
       * @param protocol
       * 
       * @return 0 on success, -1 on error
       */
      int create(const int domain, const int type, const int protocol);

      /**
       * Bind server socket to provided address
       * 
       * @param address
       * @param port
       * 
       * @return 0 on success, -1 on error
       */
      int bind(const char* address, const char* port);

      /**
       * Client socket connect to server
       * 
       * @param address
       * @param port
       * 
       * @return 0 on success, -1 on error
       */
      int connect(const char* address, const char* port);

      /**
       * Server socket listen for incoming connections
       * 
       * @param max_queue
       * 
       * @return 0 on success, -1 on error
       */
      int listen(const int max_queue);

      /**
       * Server socket accept incoming connections
       * 
       * @return pointer to connected client socket
      */
      Socket* accept();

      /**
       * Read data into buffer from socket
       * 
       * @param buf
       * @param len
       * 
       * @return number of bytes received, -1 on error
       */
      int read_socket(char &buf, int len);

      /**
       * Write data to socket
       * 
       * @param msg
       * 
       * @return number of bytes sent, -1 on error
       */
      int write_socket(const char* msg);

      /**
       * Set socket into blocking mode
       * 
       * @return
       */
      int set_blocking();

      /**
       * Set socket into non-blocking mode
       * 
       * @return
       */
      int set_nblocking();

      /**
       * Disables additional send/recv functionality based on @param how
       * 
       * @param how is defined as:
       * SHUT_RD   = No more receptions;
       * SHUT_WR   = No more transmissions;
       * SHUT_RDWR = No more receptions or transmissions.
       * 
       * @return 0 on success, -1 on error
       */
      int shutdown(int how);

      /**
       * Close socket
       * 
       * @return 0 on success, -1 on error
       */
      int close();
  };
}

#endif // EMANERELAY_SOCKET_H_