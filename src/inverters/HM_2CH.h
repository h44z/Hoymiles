#ifndef __HM_2CH_H__
#define __HM_2CH_H__

#include "HM_Abstract.h"

class HM_2CH : public HM_Abstract {
public:
    HM_2CH(uint64_t serial);
    static bool isValidSerial(uint64_t serial);
    String typeName();
    const hm_byteAssign_t* getByteAssignment();
    const uint8_t getAssignmentCount();

private:
    const hm_byteAssign_t byteAssignment[23] = {
        { HM_FLD_UDC, HM_UNIT_V, HM_CH1, 2, 2, 10 },
        { HM_FLD_IDC, HM_UNIT_A, HM_CH1, 4, 2, 100 },
        { HM_FLD_PDC, HM_UNIT_W, HM_CH1, 6, 2, 10 },
        { HM_FLD_YD, HM_UNIT_WH, HM_CH1, 22, 2, 1 },
        { HM_FLD_YT, HM_UNIT_KWH, HM_CH1, 14, 4, 1000 },
        { HM_FLD_IRR, HM_UNIT_PCT, HM_CH1, HM_CALC_IRR_CH, HM_CH1, HM_CMD_CALC },

        { HM_FLD_UDC, HM_UNIT_V, HM_CH2, 8, 2, 10 },
        { HM_FLD_IDC, HM_UNIT_A, HM_CH2, 10, 2, 100 },
        { HM_FLD_PDC, HM_UNIT_W, HM_CH2, 12, 2, 10 },
        { HM_FLD_YD, HM_UNIT_WH, HM_CH2, 24, 2, 1 },
        { HM_FLD_YT, HM_UNIT_KWH, HM_CH2, 18, 4, 1000 },
        { HM_FLD_IRR, HM_UNIT_PCT, HM_CH2, HM_CALC_IRR_CH, HM_CH2, HM_CMD_CALC },

        { HM_FLD_UAC, HM_UNIT_V, HM_CH0, 26, 2, 10 },
        { HM_FLD_IAC, HM_UNIT_A, HM_CH0, 34, 2, 100 },
        { HM_FLD_PAC, HM_UNIT_W, HM_CH0, 30, 2, 10 },
        { HM_FLD_PRA, HM_UNIT_VA, HM_CH0, 32, 2, 10 },
        { HM_FLD_F, HM_UNIT_HZ, HM_CH0, 28, 2, 100 },
        { HM_FLD_T, HM_UNIT_C, HM_CH0, 38, 2, 10 },
        { HM_FLD_EVT_LOG, HM_UNIT_CNT, HM_CH0, 40, 2, 1 },
        { HM_FLD_YD, HM_UNIT_WH, HM_CH0, HM_CALC_YD_CH0, 0, HM_CMD_CALC },
        { HM_FLD_YT, HM_UNIT_KWH, HM_CH0, HM_CALC_YT_CH0, 0, HM_CMD_CALC },
        { HM_FLD_PDC, HM_UNIT_W, HM_CH0, HM_CALC_PDC_CH0, 0, HM_CMD_CALC },
        { HM_FLD_EFF, HM_UNIT_PCT, HM_CH0, HM_CALC_EFF_CH0, 0, HM_CMD_CALC }
    };
};

#endif // __HM_2CH_H__