#ifndef __DevInfoSampleCommand_H__
#define __DevInfoSampleCommand_H__

#include "MultiDataCommand.h"

namespace Hoymiles
{
    class DevInfoSampleCommand : public MultiDataCommand {
    public:
        DevInfoSampleCommand(uint64_t target_address = 0, uint64_t router_address = 0, time_t time = 0);

        virtual bool handleResponse(InverterAbstract* inverter, hm_fragment_t fragment[], uint8_t max_fragment_id);
    };
}

#endif // __DevInfoSampleCommand_H__