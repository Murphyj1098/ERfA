#ifndef EMANERELAY_TYPES_H_
#define EMANERELAY_TYPES_H_

#include <vector>

namespace emane_relay {

  typedef unsigned char   uint8_t;
  typedef unsigned short  uint16_t;
  typedef unsigned int    uint32_t;
  typedef unsigned long   uint64_t;

  typedef signed char     sint8_t;
  typedef signed short    sint16_t;
  typedef signed int      sint32_t;
  typedef signed long     sint64_t;

  typedef unsigned int    NEMId;

  typedef int             pid_t;

  struct SMeta
  {
    uint16_t  numRobots;
    double    deltaT;
    pid_t     argosPID;
    pid_t     emanePID;
    double    gwLat;
    double    gwLon;
    double    gwAlt;
  };
  
  struct SPose
  {
    uint16_t  id;
    char      addr[256];  // Address string for AF_UNIX socket
    double    lat;
    double    lon;
    double    alt;
  };

  struct SPreamble
  {
    uint32_t timestamp;
    uint16_t src;
    uint16_t dst;
    uint16_t payloadSize;
  };

  struct SMessage
  {
    /**
     * @brief Calculate the size of a given SMessage struct
     * 
     * Member payload is dynamically sized vector
     */
      inline size_t CalculateBufferSize() {
         size_t unBufferSize = 0;
         // Calculate the size for the fixed-size members
         unBufferSize += sizeof(timestamp);
         unBufferSize += sizeof(src);
         unBufferSize += sizeof(dst);
         unBufferSize += sizeof(payloadSize);
         // Calculate the size for the variable-length member
         unBufferSize += payload.size();
         return unBufferSize;
      }

      /**
       * 
       */
      inline void CopyPreamble(SPreamble preamble) {
        timestamp = preamble.timestamp;
        src = preamble.src;
        dst = preamble.dst;
        payloadSize = preamble.payloadSize;
      }

    uint32_t timestamp;
    uint16_t src;
    uint16_t dst;
    uint16_t payloadSize;
    std::vector<uint8_t> payload;
  };
  
}

#endif // EMANERELAY_TYPES_H_