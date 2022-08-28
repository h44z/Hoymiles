#include "HoymilesRadio.h"
#include "Hoymiles.h"
#include "commands/RequestFrameCommand.h"
#include "crc.h"
#include <every.h>
#include <FunctionalInterrupt.h>

namespace Hoymiles
{
    void HoymilesRadio::init()
    {
        _dtuSerial.u64 = 0;

        _radio.reset(new RF24(HOYMILES_PIN_CE, HOYMILES_PIN_CS, HOYMILES_SPI_SPEED));
        _radio->begin(HOYMILES_PIN_CE, HOYMILES_PIN_CS);

        _radio->setDataRate(RF24_250KBPS);
        _radio->enableDynamicPayloads();
        _radio->setCRCLength(RF24_CRC_16);
        _radio->setAddressWidth(5);
        _radio->setRetries(0, 0);
        _radio->maskIRQ(true, true, false); // enable only receiving interrupts, args = "data_sent", "data_fail", "data_ready"
        if (_radio->isChipConnected()) {
            PINFOLN(F("NRF24 connection successfull"));
        } else {
            PERRLN(F("NRF24 connection error, check wiring"));
        }

        //pinMode(HOYMILES_PIN_IRQ, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(HOYMILES_PIN_IRQ), std::bind(&HoymilesRadio::handleIntr, this), FALLING);
        // IMPORTANT: do not call radio.available() before calling
        // radio.whatHappened() when the interruptHandler() is triggered by the
        // IRQ pin FALLING event. According to the datasheet, the pipe information
        // is unreliable during the IRQ pin FALLING transition.

        openReadingPipe();
        _radio->startListening();

        PINFOLN(F("Radio setup completed"));
    }

    void HoymilesRadio::loop()
    {
        EVERY_N_MILLIS(4)
        {
            switchRxCh();
        }

        processReceivedPacket();

        processNrf24Buffer();

        processCommandResponse();
        
        processCommandQueue();
    }

    void HoymilesRadio::processReceivedPacket()
    {
        if (_rxBuffer.empty()) {
            return;
        }

        hm_fragment_t* f = _rxBuffer.getBack();
        if (checkFragmentCrc(f)) {
            std::shared_ptr<InverterAbstract> inv = Hoymiles.getInverterByFragment(f);

            if (nullptr != inv) {
                // Save packet in inverter rx buffer
                dumpBuf("RX ", f->fragment, f->len);
                inv->addRxFragment(f->fragment, f->len);
            } else {
                PERRLN(F("Inverter not found!"));
            }

        } else {
            PWARNLN(F("Frame corrupted"));
        }

        // Remove paket from buffer even it was corrupted
        _rxBuffer.popBack();
    }

    void HoymilesRadio::processNrf24Buffer()
    {
        if (!_packetReceived) {
            return;
        }

        PDBGLN(F("NRF24: packet pending"));
        while (_radio->available()) {
            if (!_rxBuffer.full()) {
                hm_fragment_t* f;
                f = _rxBuffer.getFront();
                memset(f->fragment, 0xcc, HM_MAX_RF_PAYLOAD_SIZE);
                f->len = _radio->getDynamicPayloadSize();
                if (f->len > HM_MAX_RF_PAYLOAD_SIZE)
                    f->len = HM_MAX_RF_PAYLOAD_SIZE;

                _radio->read(f->fragment, f->len);
                _rxBuffer.pushFront(f);
            } else {
                PWARNLN(F("Receive buffer full, skipping packet."));
                _radio->flush_rx();
            }
        }
        PDBGLN(F("NRF24: packet received"));
        _packetReceived = false;
    }

    void HoymilesRadio::processCommandQueue()
    {
        if (_waitingForResponse) {
            return;
        }
        if (_commandQueue.empty()) {
            return;
        }

        CommandAbstract* cmd = _commandQueue.front().get();

        auto inv = Hoymiles.getInverterBySerial(cmd->getTargetAddress());
        inv->clearRxFragmentBuffer();
        sendCommand(cmd);
    }

    void HoymilesRadio::processCommandResponse()
    {
        if (!_waitingForResponse) {
            return;
        }
        if (!_responseTimeout.occured()) {
            return; // receive window not yet closed
        }

        PDBGLN(F("NRF24: TX period finished"));
        CommandAbstract* cmd = _commandQueue.front().get();
        std::shared_ptr<InverterAbstract> inv = Hoymiles.getInverterBySerial(cmd->getTargetAddress());

        if (nullptr == inv) {
            return; // invalid inverter in command...
        }

        uint8_t verifyResult = inv->verifyAllFragments(cmd);
        if (verifyResult == HM_FRAGMENT_ALL_MISSING) {
            if (cmd->getSendCount() <= HM_MAX_RESEND_COUNT) {
                PDBGLN(F("Nothing received, resend whole request"));
                sendCommand(cmd); // re-send
            } else {
                PWARNLN(F("Nothing received, resend count exeeded"));
                _commandQueue.pop();
                _waitingForResponse = false;
            }

        } else if (verifyResult == HM_FRAGMENT_RETRANSMIT_TIMEOUT) {
            PWARNLN(F("Retransmit timeout"));
            _commandQueue.pop();
            _waitingForResponse = false;

        } else if (verifyResult == HM_FRAGMENT_HANDLE_ERROR) {
            PWARNLN(F("Packet handling error"));
            _commandQueue.pop();
            _waitingForResponse = false;

        } else if (verifyResult > 0) {
            // Perform Retransmit
            PDBGF("Request retransmit: %d\n", verifyResult);
            sendCommandFragment(cmd, verifyResult);

        } else {
            // Successfull received all packages
            PDBGLN(F("Success"));
            _commandQueue.pop();
            _waitingForResponse = false;
        }
    }

    void HoymilesRadio::setPALevel(rf24_pa_dbm_e paLevel)
    {
        _radio->setPALevel(paLevel);
    }

    hm_serial_u HoymilesRadio::getDtuSerial()
    {
        return _dtuSerial;
    }

    void HoymilesRadio::setDtuSerial(uint64_t serial)
    {
        _dtuSerial.u64 = serial;
        openReadingPipe();
    }

    bool HoymilesRadio::isIdle()
    {
        return !_waitingForResponse;
    }

    void HoymilesRadio::openReadingPipe()
    {
        hm_serial_u s;
        s = convertSerialToRadioId(_dtuSerial);
        _radio->openReadingPipe(1, s.u64);
    }

    void HoymilesRadio::openWritingPipe(hm_serial_u serial)
    {
        hm_serial_u s;
        s = convertSerialToRadioId(serial);
        _radio->openWritingPipe(s.u64);
    }

    void HoymilesRadio::handleIntr()
    {
        bool tx_ds, tx_df, rx_dr;                   // declare variables for IRQ masks
        _radio->whatHappened(tx_ds, tx_df, rx_dr);  // get values for IRQ masks
        // whatHappened() clears the IRQ masks also. This is required for
        // continued TX operations when a transmission fails.
        // clearing the IRQ masks resets the IRQ pin to its inactive state (HIGH)

        PVERBF("Interrupt received, tx_ds=%d, tx_df=%d, rx_dr=%d\n", tx_ds, tx_df, rx_dr);

        if (rx_dr) {
            _packetReceived = true;
        }
    }

    uint32_t HoymilesRadio::now()
    {
        return millis();
    }

    uint8_t HoymilesRadio::getRxNxtChannel()
    {
        if (++_rxChIdx >= sizeof(_rxChLst))
            _rxChIdx = 0;
        return _rxChLst[_rxChIdx];
    }

    uint8_t HoymilesRadio::getTxNxtChannel()
    {
        if (++_txChIdx >= sizeof(_txChLst))
            _txChIdx = 0;
        return _txChLst[_txChIdx];
    }

    void HoymilesRadio::switchRxCh()
    {

        // portDISABLE_INTERRUPTS();
        _radio->stopListening();
        _radio->setChannel(getRxNxtChannel());
        _radio->startListening();
        // portENABLE_INTERRUPTS();
    }

    hm_serial_u HoymilesRadio::convertSerialToRadioId(hm_serial_u serial)
    {
        hm_serial_u radioId;
        radioId.u64 = 0;
        radioId.b[4] = serial.b[0];
        radioId.b[3] = serial.b[1];
        radioId.b[2] = serial.b[2];
        radioId.b[1] = serial.b[3];
        radioId.b[0] = 0x01;
        return radioId;
    }

    bool HoymilesRadio::checkFragmentCrc(hm_fragment_t* fragment)
    {
        uint8_t crc = hm_crc8(fragment->fragment, fragment->len - 1);
        return (crc == fragment->fragment[fragment->len - 1]);
    }

    void HoymilesRadio::sendCommand(CommandAbstract* cmd)
    {
        cmd->incrementSendCount();
        cmd->setRouterAddress(getDtuSerial().u64);

        _radio->stopListening();
        _radio->setChannel(getTxNxtChannel());

        hm_serial_u s;
        s.u64 = cmd->getTargetAddress();
        openWritingPipe(s);
        _radio->setRetries(3, 15);

        PVERBF("TX Channel: %d  --> \n", _radio->getChannel());
        cmd->dumpDataPayload();
        _radio->write(cmd->getDataPayload(), cmd->getDataSize());

        _radio->setRetries(0, 0);
        openReadingPipe();
        _radio->setChannel(getRxNxtChannel());
        _radio->startListening();
        _waitingForResponse = true;
        _responseTimeout.set(cmd->getTimeout());
    }

    void HoymilesRadio::sendCommandFragment(CommandAbstract* cmd, uint8_t fragment_id)
    {
        CommandAbstract* requestCmd = cmd->getRequestFrameCommand(fragment_id);

        if (requestCmd != nullptr) {
            sendCommand(requestCmd);
        }
    }

    void HoymilesRadio::dumpBuf(const char* info, uint8_t buf[], uint8_t len)
    {
        if (HM_SERIAL_LOG_LEVEL == HM_LOG_VERBOSE) {
            if (NULL != info)
                Serial.print(String(info));

            for (uint8_t i = 0; i < len; i++) {
                Serial.print(buf[i], 16);
                Serial.print(" ");
            }
            Serial.println(F(""));
        }
    }
}