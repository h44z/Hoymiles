#ifndef __RealTimeRunDataCommand_H__
#define __RealTimeRunDataCommand_H__

#include "MultiDataCommand.h"

class RealTimeRunDataCommand : public MultiDataCommand {
public:
    RealTimeRunDataCommand(uint64_t target_address = 0, uint64_t router_address = 0, time_t time = 0);

    virtual bool handleResponse(InverterAbstract* inverter, hm_fragment_t fragment[], uint8_t max_fragment_id);
};

#endif // __RealTimeRunDataCommand_H__