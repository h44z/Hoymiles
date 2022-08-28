#ifndef __Hoymiles_H__
#define __Hoymiles_H__

#include "HoymilesRadio.h"
#include "inverters/InverterAbstract.h"
#include "types.h"
#include <memory>
#include <vector>

namespace Hoymiles
{
    class HoymilesClass {
    public:
        void init();
        void loop();

        std::shared_ptr<InverterAbstract> addInverter(const char* name, uint64_t serial);
        std::shared_ptr<InverterAbstract> getInverterByPos(uint8_t pos);
        std::shared_ptr<InverterAbstract> getInverterBySerial(uint64_t serial);
        std::shared_ptr<InverterAbstract> getInverterByFragment(hm_fragment_t* fragment);
        void removeInverterByPos(uint8_t pos);
        void removeInverterBySerial(uint64_t serial);
        size_t getNumInverters();

        HoymilesRadio* getRadio();

        uint32_t PollInterval();
        void setPollInterval(uint32_t interval);

    private:
        std::vector<std::shared_ptr<InverterAbstract>> _inverters;
        std::unique_ptr<HoymilesRadio> _radio;

        uint32_t _pollInterval;
        uint32_t _lastPoll = 0;
    };

    extern HoymilesClass Hoymiles;
}

#endif // __Hoymiles_H__