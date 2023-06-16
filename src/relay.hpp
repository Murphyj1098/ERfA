#ifndef EMANERELAY_RELAY_H_
#define EMANERELAY_RELAY_H_

#include "types.hpp"

#include <thread>

#include "socket.hpp"

// Pre-defined shared memory paths
#define SHM_PATH_META "/argos_emane_meta"
#define SHM_PATH_POSE "/argos_emane_pose"

#define EMANE_SOCK_PORT "54713"

namespace emane_relay {

  class Relay
  {
    public:
      Relay(NEMId id);
      ~Relay();
      void doInit();
      void doStart();
      void doStop();
      void doDestroy();

    private:
      std::string sockAddr_;
      std::string emane0Addr_;
      std::thread argosThread_;
      std::thread emaneThread_;
      NEMId id_;
      Socket * pARGoSSock_;
      Socket * pEMANESock_;

      /**
       * @brief Relay data from NEM to ARGoS
       * 
       */
      void toARGoS();

      /**
       * @brief Relay data from ARGoS to NEM
       * 
       */
      void toEMANE();
  };

}


#endif // EMANERELAY_RELAY_H_