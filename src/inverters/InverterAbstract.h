#ifndef __InverterAbstract_H__
#define __InverterAbstract_H__

#include "../parser/AlarmLogParser.h"
#include "../parser/StatisticsParser.h"
#include "../parser/DevInfoParser.h"
#include "../HoymilesRadio.h"
#include "../types.h"
#include <Arduino.h>
#include <cstdint>

namespace Hoymiles
{
    #define HM_MAX_NAME_LENGTH 32
    #define HM_OFFLINE_TRESHOLD 30000 // milliseconds

    enum {
        HM_FRAGMENT_ALL_MISSING = 255,
        HM_FRAGMENT_RETRANSMIT_TIMEOUT = 254,
        HM_FRAGMENT_HANDLE_ERROR = 253,
        HM_FRAGMENT_OK = 0
    };

    #define HM_MAX_RF_FRAGMENT_COUNT 12
    #define HM_MAX_RETRANSMIT_COUNT 5

    class CommandAbstract;

    class InverterAbstract {
    public:
        InverterAbstract(uint64_t serial);
        void init();
        uint64_t serial();
        void setName(const char* name);
        const char* name();
        void setLastResponse(uint32_t responseTime);
        uint32_t lastResponse();
        void setLastRequest(uint32_t requestTime);
        uint32_t lastRequest();
        bool online();
        virtual String typeName() = 0;
        virtual const hm_byteAssign_t* getByteAssignment() = 0;
        virtual const uint8_t getAssignmentCount() = 0;

        void clearRxFragmentBuffer();
        void addRxFragment(uint8_t fragment[], uint8_t len);
        uint8_t verifyAllFragments(CommandAbstract* cmd);

        virtual bool sendStatsRequest(HoymilesRadio* radio) = 0;
        virtual bool sendAlarmLogRequest(HoymilesRadio* radio) = 0;
        virtual bool sendDevInfoRequest(HoymilesRadio* radio) = 0;

        AlarmLogParser* EventLog();
        DevInfoParser* DevInfo();
        StatisticsParser* Statistics();

    private:
        hm_serial_u _serial;
        char _name[HM_MAX_NAME_LENGTH];
        hm_fragment_t _rxFragmentBuffer[HM_MAX_RF_FRAGMENT_COUNT];
        uint8_t _rxFragmentMaxPacketId = 0;
        uint8_t _rxFragmentLastPacketId = 0;
        uint8_t _rxFragmentRetransmitCnt = 0;
        uint32_t _lastResponse = 0;
        uint32_t _lastRequest = 0;

        std::unique_ptr<AlarmLogParser> _alarmLogParser;
        std::unique_ptr<DevInfoParser> _devInfoParser;
        std::unique_ptr<StatisticsParser> _statisticsParser;
    };
}

#endif // __InverterAbstract_H__