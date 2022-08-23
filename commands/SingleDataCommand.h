#ifndef __SingleDataCommand_H__
#define __SingleDataCommand_H__

#include "CommandAbstract.h"

class SingleDataCommand : public CommandAbstract {
public:
    SingleDataCommand(uint64_t target_address = 0, uint64_t router_address = 0);
};

#endif // __SingleDataCommand_H__