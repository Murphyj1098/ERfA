#ifndef EMANERELAY_RELAY_H_
#define EMANERELAY_RELAY_H_

// Pre-defined shared memory paths
#define SHM_PATH_META "/argos_emane_meta"
#define SHM_PATH_POSE "/argos_emane_pose"

namespace emane_relay {

  class Relay
  {
    public:
      Relay();
      ~Relay();
      void doInit();
      void doStart();
      void doStop();
      void doDestroy();

    protected:

    private:

  };

}


#endif // EMANERELAY_RELAY_H_