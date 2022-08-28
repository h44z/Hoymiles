#include "HM_2CH.h"

namespace Hoymiles
{
    HM_2CH::HM_2CH(uint64_t serial)
        : HM_Abstract(serial) {};

    bool HM_2CH::isValidSerial(uint64_t serial)
    {
        return serial >= 0x114100000000 && serial <= 0x114199999999;
    }

    String HM_2CH::typeName()
    {
        return String(F("HM-600, HM-700, HM-800"));
    }

    const hm_byteAssign_t* HM_2CH::getByteAssignment()
    {
        return byteAssignment;
    }

    const uint8_t HM_2CH::getAssignmentCount()
    {
        return sizeof(byteAssignment) / sizeof(hm_byteAssign_t);
    }
}