#ifndef __Parser_H__
#define __Parser_H__

#include <cstdint>

namespace Hoymiles
{
    class Parser {
    public:
        uint32_t getLastUpdate();
        void setLastUpdate(uint32_t lastUpdate);

    private:
        uint32_t _lastUpdate = 0;
    };
}

#endif // __Parser_H__