#ifndef __StatisticsParser_H__
#define __StatisticsParser_H__

#include "Parser.h"
#include <Arduino.h>
#include <cstdint>

#define HM_STATISTIC_PACKET_SIZE (4 * 16)

// units
enum {
    HM_UNIT_V = 0,
    HM_UNIT_A,
    HM_UNIT_W,
    HM_UNIT_WH,
    HM_UNIT_KWH,
    HM_UNIT_HZ,
    HM_UNIT_C,
    HM_UNIT_PCT,
    HM_UNIT_VA,
    HM_UNIT_CNT
};
const char* const hm_units[] = { "V", "A", "W", "Wh", "kWh", "Hz", "°C", "%", "var", "" };

// field types
enum {
    HM_FLD_UDC = 0,
    HM_FLD_IDC,
    HM_FLD_PDC,
    HM_FLD_YD,
    HM_FLD_YT,
    HM_FLD_UAC,
    HM_FLD_IAC,
    HM_FLD_PAC,
    HM_FLD_F,
    HM_FLD_T,
    HM_FLD_PCT,
    HM_FLD_EFF,
    HM_FLD_IRR,
    HM_FLD_PRA,
    HM_FLD_EVT_LOG
};
const char* const hm_fields[] = { "Voltage", "Current", "Power", "YieldDay", "YieldTotal",
    "Voltage", "Current", "Power", "Frequency", "Temperature", "PowerFactor", "Efficiency", "Irradiation", "ReactivePower", "EventLogCount" };

// indices to calculation functions, defined in hmInverter.h
enum {
    HM_CALC_YT_CH0 = 0,
    HM_CALC_YD_CH0,
    HM_CALC_UDC_CH,
    HM_CALC_PDC_CH0,
    HM_CALC_EFF_CH0,
    HM_CALC_IRR_CH
};
enum { HM_CMD_CALC = 0xffff };

// HM_CH0 is default channel (freq, ac, temp)
enum {
    HM_CH0 = 0,
    HM_CH1,
    HM_CH2,
    HM_CH3,
    HM_CH4
};

typedef struct {
    uint8_t fieldId; // field id
    uint8_t unitId; // uint id
    uint8_t ch; // channel 0 - 4
    uint8_t start; // pos of first byte in buffer
    uint8_t num; // number of bytes in buffer
    uint16_t div; // divisor / calc command
} hm_byteAssign_t;

// prototypes
class StatisticsParser;

using hm_func_t = float(StatisticsParser*, uint8_t);

struct hm_calcFunc_t {
    uint8_t funcId; // unique id
    hm_func_t* func; // function pointer
};

class StatisticsParser : public Parser {
public:
    void clearBuffer();
    void appendFragment(uint8_t offset, uint8_t* payload, uint8_t len);

    void setByteAssignment(const hm_byteAssign_t* byteAssignment, const uint8_t count);

    uint8_t getAssignIdxByChannelField(uint8_t channel, uint8_t fieldId);
    float getChannelFieldValue(uint8_t channel, uint8_t fieldId);
    bool hasChannelFieldValue(uint8_t channel, uint8_t fieldId);
    const char* getChannelFieldUnit(uint8_t channel, uint8_t fieldId);
    const char* getChannelFieldName(uint8_t channel, uint8_t fieldId);

    uint8_t getChannelCount();

    uint16_t getChannelMaxPower(uint8_t channel);
    void setChannelMaxPower(uint8_t channel, uint16_t power);

private:
    uint8_t _payloadStatistic[HM_STATISTIC_PACKET_SIZE];
    uint8_t _statisticLength;
    uint16_t _chanMaxPower[HM_CH4];

    const hm_byteAssign_t* _byteAssignment;
    uint8_t _byteAssignmentCount;
};

#endif // __StatisticsParser_H__