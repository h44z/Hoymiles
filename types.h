#ifndef __HM_types_H__
#define __HM_types_H__

#include <cstdint>

union hm_serial_u {
    uint64_t u64;
    uint8_t b[8];
};

// maximum buffer length of packet received / sent to RF24 module
#define HM_MAX_RF_PAYLOAD_SIZE 32

typedef struct {
    uint8_t fragment[HM_MAX_RF_PAYLOAD_SIZE];
    uint8_t len;
    bool wasReceived;
} hm_fragment_t;

#endif // __HM_types_H__