#ifndef __HoymilesRadio_H__
#define __HoymilesRadio_H__

#include "CircularBuffer.h"
#include "TimeoutHelper.h"
#include "commands/CommandAbstract.h"
#include "types.h"
#include <RF24.h>
#include <RF24_config.h>
#include <memory>
#include <queue>
#include <Arduino.h>

using namespace std;

namespace Hoymiles
{

// number of fragments hold in buffer
#define HM_FRAGMENT_BUFFER_SIZE 30

#define HM_MAX_RESEND_COUNT 4

#ifndef HOYMILES_PIN_IRQ
#define HOYMILES_PIN_IRQ 0
#endif

#ifndef HOYMILES_PIN_CE
#define HOYMILES_PIN_CE 2
#endif

#ifndef HOYMILES_PIN_CS
#define HOYMILES_PIN_CS 15
#endif

#ifndef HOYMILES_SPI_SPEED
#define HOYMILES_SPI_SPEED 1000000 // 1Mhz
#endif

    class HoymilesRadio {
    public:
        void init();
        void loop();
        void setPALevel(rf24_pa_dbm_e paLevel);

        hm_serial_u getDtuSerial();
        void setDtuSerial(uint64_t serial);
        uint32_t now();

        bool isIdle();

        template <typename T>
        T* enqueCommand()
        {
            _commandQueue.push(make_shared<T>());
            return static_cast<T*>(_commandQueue.back().get());
        }

    private:
        void handleIntr();
        static hm_serial_u convertSerialToRadioId(hm_serial_u serial);
        uint8_t getRxNxtChannel();
        uint8_t getTxNxtChannel();
        void switchRxCh();
        void openReadingPipe();
        void openWritingPipe(hm_serial_u serial);
        bool checkFragmentCrc(hm_fragment_t* fragment);
        void dumpBuf(const char* info, uint8_t buf[], uint8_t len);

        void sendCommand(CommandAbstract* cmd);
        void sendCommandFragment(CommandAbstract* cmd, uint8_t fragment_id);

        void processReceivedPacket();
        void processNrf24Buffer();
        void processCommandQueue();
        void processCommandResponse();

        std::unique_ptr<RF24> _radio;
        uint8_t _rxChLst[5] = { 3, 23, 40, 61, 75 };
        uint8_t _rxChIdx;

        uint8_t _txChLst[5] = { 3, 23, 40, 61, 75 };
        uint8_t _txChIdx;

        volatile bool _packetReceived;

        CircularBuffer<hm_fragment_t, HM_FRAGMENT_BUFFER_SIZE> _rxBuffer;


        hm_serial_u _dtuSerial;

        bool _waitingForResponse = false;
        TimeoutHelper _responseTimeout;

        queue<shared_ptr<CommandAbstract>> _commandQueue;
    };
}

#endif // __HoymilesRadio_H__