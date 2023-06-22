#ifndef EMANERELAY_RELAY_H_
#define EMANERELAY_RELAY_H_

#include "types.hpp"

#include <mutex>
#include <queue>
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
      std::thread toARGoSThread_;
      std::thread fromARGoSThread_;
      std::thread toEMANEThread_;
      std::thread fromEMANEThread_;
      std::mutex argosQueueLock_;
      std::mutex emaneQueueLock_;
      std::queue<SMessage> argosQueue_; // From ARGoS
      std::queue<SMessage> emaneQueue_; // From EMANE
      NEMId id_;
      Socket * pARGoSSock_;
      Socket * pEMANESock_;

      /**
       * @brief Relay data from rx queue to ARGoS
       * 
       */
      void toARGoS();

      /**
       * @brief Relay data from ARGoS to tx queue
       * 
       */
      void fromARGoS();

      /**
       * @brief Relay data from tx queue to NEM
       * 
       */
      void toEMANE();

      /**
       * @brief Relay data from NEM to rx queue
       * 
       */
      void fromEMANE();
  };

}


#endif // EMANERELAY_RELAY_H_