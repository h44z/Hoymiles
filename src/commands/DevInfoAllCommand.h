#ifndef __DevInfoAllCommand_H__
#define __DevInfoAllCommand_H__

#include "MultiDataCommand.h"

class DevInfoAllCommand : public MultiDataCommand {
public:
    DevInfoAllCommand(uint64_t target_address = 0, uint64_t router_address = 0, time_t time = 0);

    virtual bool handleResponse(InverterAbstract* inverter, hm_fragment_t fragment[], uint8_t max_fragment_id);
};

#endif // __DevInfoAllCommand_H__