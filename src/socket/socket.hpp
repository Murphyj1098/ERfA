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
      int create(const int, const int, const int);

      /**
       * Bind server socket to provided address
       * 
       * @param address
       * @param port
       * 
       * @return 0 on success, -1 on error
       */
      int bind(const char*, const char*);

      /**
       * Client socket connect to server
       * 
       * @param address
       * @param port
       * 
       * @return 0 on success, -1 on error
       */
      int connect(const char*, const char*);

      /**
       * Server socket listen for incoming connections
       * 
       * @param max_queue
       * 
       * @return 0 on success, -1 on error
       */
      int listen(const int);

      /**
       * Server socket accept incoming connections
       * 
       * @return pointer to connected client socket
      */
      Socket* accept();

      int read_socket(char &, int); // TODO: What is the typing on this?

      int write_socket(const char*);

      /**
       * @param how is defined as:
       * SHUT_RD   = No more receptions;
       * SHUT_WR   = No more transmissions;
       * SHUT_RDWR = No more receptions or transmissions.
       */
      int shutdown(int);

      int close();
  };
}

#endif // EMANERELAY_SOCKET_H_