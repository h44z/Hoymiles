#ifndef __TimeoutHelper_H__
#define __TimeoutHelper_H__

#include <stdint.h>

namespace Hoymiles
{
    class TimeoutHelper {
    public:
        TimeoutHelper();
        void set(uint32_t ms);
        void extend(uint32_t ms);
        bool occured();

    private:
        uint32_t startMillis;
        uint32_t timeout;
    };
}

#endif // __TimeoutHelper_H__