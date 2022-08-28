#include <Hoymiles.h>
#include <Arduino.h>

#define SERIAL_BAUDRATE 115200

unsigned long previousMillis = 0;
unsigned long interval = 2000;

//-----------------------------------------------------------------------------
void setup() {
    // Initialize serial output
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial)
        yield(); // let ESP run its background tasks
    Serial.flush();
    Serial.println();
    Serial.println(F("Starting SimpleDTU..."));

    Serial.println(F("Initialize Hoymiles interface... "));
    Hoymiles::Hoymiles.init();

    Hoymiles::Hoymiles.getRadio()->setPALevel(RF24_PA_HIGH);
    Hoymiles::Hoymiles.getRadio()->setDtuSerial(0x99978563412);
    Hoymiles::Hoymiles.setPollInterval(5);

    auto inv = Hoymiles::Hoymiles.addInverter("HM-300", 0x121112345678); // serial is 121112345678
    if (inv != nullptr) {
        inv->Statistics()->setChannelMaxPower(0, 385); // channel 0 has 385W
    } else {
        Serial.println(F("Failed to add inverter"));
        while( true ) {}
    }

    Serial.println(F("Entering main loop..."));
}


void printField(std::shared_ptr<Hoymiles::InverterAbstract> inv, uint8_t channel, uint8_t fieldId) {
    Serial.print("  - ");
    Serial.print(hm_fields[fieldId]);
    Serial.print(" : ");
    Serial.print(inv->Statistics()->getChannelFieldValue(channel, fieldId));
    Serial.print(" ");
    Serial.println(inv->Statistics()->getChannelFieldUnit(channel, fieldId));
}


//-----------------------------------------------------------------------------
void loop() {
    Hoymiles::Hoymiles.loop();

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;

        auto inv = Hoymiles::Hoymiles.getInverterByPos(0);
        Serial.print("INVERTER STATS: ");
        Serial.print(inv->name());
        Serial.print("(last contact: ");
        Serial.print(inv->Statistics()->getLastUpdate());
        Serial.println(")");

        for (uint8_t c = 0; c <= inv->Statistics()->getChannelCount(); c++) {
            Serial.print("Channel: ");
            Serial.println(c);

            Serial.println(" - DC Stats");
            printField(inv, c, Hoymiles::HM_FLD_PDC);
            if (c > 0) { // only DC channels
                printField(inv, c, Hoymiles::HM_FLD_UDC);
                printField(inv, c, Hoymiles::HM_FLD_IDC);
            }

            if (c == 0) { // only AC channel
                Serial.println(" - AC Stats");
                printField(inv, c, Hoymiles::HM_FLD_PAC);
                printField(inv, c, Hoymiles::HM_FLD_UAC);
                printField(inv, c, Hoymiles::HM_FLD_IAC);
            }

            Serial.println(" - Other");
            printField(inv, c, Hoymiles::HM_FLD_YD);
            printField(inv, c, Hoymiles::HM_FLD_YT);
            if (c == 0) {
                printField(inv, c, Hoymiles::HM_FLD_F);
                printField(inv, c, Hoymiles::HM_FLD_T);
                printField(inv, c, Hoymiles::HM_FLD_EFF);
            }
            printField(inv, c, Hoymiles::HM_FLD_PCT);
            printField(inv, c, Hoymiles::HM_FLD_PRA);
            if (c > 0) {
                printField(inv, c, Hoymiles::HM_FLD_IRR);
            }
        }
    }
}