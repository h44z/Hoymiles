#ifndef __DevControlCommand_H__
#define __DevControlCommand_H__

#include "CommandAbstract.h"

namespace Hoymiles
{
    class DevControlCommand : public CommandAbstract {
    public:
        DevControlCommand(uint64_t target_address = 0, uint64_t router_address = 0);
    };
}

#endif // __DevControlCommand_H__