#include "InverterAbstract.h"
#include "../crc.h"
#include <cstring>

namespace Hoymiles
{
    InverterAbstract::InverterAbstract(uint64_t serial)
    {
        _serial.u64 = serial;
        _alarmLogParser.reset(new AlarmLogParser());
        _devInfoParser.reset(new DevInfoParser());
        _statisticsParser.reset(new StatisticsParser());
    }

    void InverterAbstract::init()
    {
        // This has to be done here because:
        // Not possible in constructor --> virtual function
        // Not possible in verifyAllFragments --> Because no data if nothing is ever received
        // It has to be executed because otherwise the getChannelCount method in stats always returns 0
        _statisticsParser.get()->setByteAssignment(getByteAssignment(), getAssignmentCount());
    }

    uint64_t InverterAbstract::serial()
    {
        return _serial.u64;
    }

    void InverterAbstract::setName(const char* name)
    {
        uint8_t len = strlen(name);
        if (len + 1 > HM_MAX_NAME_LENGTH) {
            len = HM_MAX_NAME_LENGTH - 1;
        }
        strncpy(_name, name, len);
        _name[len] = '\0';
    }

    const char* InverterAbstract::name()
    {
        return _name;
    }

    uint32_t InverterAbstract::lastResponse()
    {
        return _lastResponse;
    }

    void InverterAbstract::setLastResponse(uint32_t responseTime)
    {
        _lastResponse = responseTime;
    }

    uint32_t InverterAbstract::lastRequest()
    {
        return _lastRequest;
    }

    void InverterAbstract::setLastRequest(uint32_t requestTime)
    {
        _lastRequest = requestTime;
    }

    bool InverterAbstract::online()
    {
        if (_lastResponse == 0 || _lastRequest == 0) {
            return false; // missing initial response
        }

        if ((_lastResponse - _lastRequest) > HM_OFFLINE_TRESHOLD) {
            return false; // no response in time detected, inverter is offline
        }

        return true;
    }

    AlarmLogParser* InverterAbstract::EventLog()
    {
        return _alarmLogParser.get();
    }

    DevInfoParser* InverterAbstract::DevInfo()
    {
        return _devInfoParser.get();
    }

    StatisticsParser* InverterAbstract::Statistics()
    {
        return _statisticsParser.get();
    }

    void InverterAbstract::clearRxFragmentBuffer()
    {
        memset(_rxFragmentBuffer, 0, HM_MAX_RF_FRAGMENT_COUNT * sizeof(hm_fragment_t));
        _rxFragmentMaxPacketId = 0;
        _rxFragmentLastPacketId = 0;
        _rxFragmentRetransmitCnt = 0;
    }

    void InverterAbstract::addRxFragment(uint8_t fragment[], uint8_t len)
    {
        if (len < 11) {
            PERRF("FATAL: (%s, %d) fragment too short\n", __FILE__, __LINE__);
            return;
        }

        if (len - 11 > HM_MAX_RF_PAYLOAD_SIZE) {
            PERRF("FATAL: (%s, %d) fragment too large\n", __FILE__, __LINE__);
            return;
        }

        uint8_t fragmentCount = fragment[9];
        if (fragmentCount == 0) {
            PERRF("ERROR: fragment number zero received and ignored");
            return;
        }

        if ((fragmentCount & 0b01111111) < HM_MAX_RF_FRAGMENT_COUNT) {
            // Packets with 0x81 will be seen as 1
            memcpy(_rxFragmentBuffer[(fragmentCount & 0b01111111) - 1].fragment, &fragment[10], len - 11);
            _rxFragmentBuffer[(fragmentCount & 0b01111111) - 1].len = len - 11;
            _rxFragmentBuffer[(fragmentCount & 0b01111111) - 1].wasReceived = true;

            if ((fragmentCount & 0b01111111) > _rxFragmentLastPacketId) {
                _rxFragmentLastPacketId = fragmentCount & 0b01111111;
            }
        }

        // 0b10000000 == 0x80
        if ((fragmentCount & 0b10000000) == 0b10000000) {
            _rxFragmentMaxPacketId = fragmentCount & 0b01111111;
        }
    }

    // Returns Zero on Success or the Fragment ID for retransmit or error code
    uint8_t InverterAbstract::verifyAllFragments(CommandAbstract* cmd)
    {
        // All missing
        if (_rxFragmentLastPacketId == 0) {
            return HM_FRAGMENT_ALL_MISSING;
        }

        // Last fragment is missing (thte one with 0x80)
        if (_rxFragmentMaxPacketId == 0) {
            PVERBLN(F("Last missing"));
            if (_rxFragmentRetransmitCnt++ < HM_MAX_RETRANSMIT_COUNT) {
                return _rxFragmentLastPacketId + 1;
            } else {
                return HM_FRAGMENT_RETRANSMIT_TIMEOUT;
            }
        }

        // Middle fragment is missing
        for (uint8_t i = 0; i < _rxFragmentMaxPacketId - 1; i++) {
            if (!_rxFragmentBuffer[i].wasReceived) {
                PVERBLN(F("Middle missing"));
                if (_rxFragmentRetransmitCnt++ < HM_MAX_RETRANSMIT_COUNT) {
                    return i + 1;
                } else {
                    return HM_FRAGMENT_RETRANSMIT_TIMEOUT;
                }
            }
        }

        if (!cmd->handleResponse(this, _rxFragmentBuffer, _rxFragmentMaxPacketId)) {
            return HM_FRAGMENT_HANDLE_ERROR;
        }

        return HM_FRAGMENT_OK;
    }
}