#include "esphome.h"
#include <Hoymiles.h>
#include <vector>
#include <map>

#define get_hm(constructor) static_cast<HoymilesDTU *>(constructor.get_component(0))

#define hm_sensor(compId, inv, channel, field) get_hm(compId)->get_sensor(inv, channel, field)
#define hm_status_sensor(compId, inv) get_hm(compId)->get_status_sensor(inv)

typedef struct {
  uint64_t serial;                // Inverter serial (add a 0x infront)
  const char* name;               // Inverter name
  std::vector<uint16_t> strings;  // Module powers in Watt
} inverterConfig;

class HoymilesDTU;

class HoymilesStatusSensor : public BinarySensor {
  public:
    HoymilesStatusSensor(HoymilesDTU *parent, uint8_t invIdx)
    {
      this->_inv = invIdx;
    }
    uint8_t inverter() 
    { 
      return this->_inv; 
    }

  protected:
    uint8_t _inv;
};

class HoymilesSensor : public Sensor {
  public:
    HoymilesSensor(HoymilesDTU *parent, uint8_t invIdx, uint8_t channel, uint8_t field)
    {
      this->_inv = invIdx;
      this->_channel = channel;
      this->_field = field;
    }
    uint8_t inverter() 
    { 
      return this->_inv; 
    }
    uint8_t channel() 
    { 
      return this->_channel; 
    }
    uint8_t field() 
    { 
      return this->_field; 
    }

  protected:
    uint8_t _inv;
    uint8_t _channel;
    uint8_t _field;
};

class HoymilesDTU : public Component {
 private:
    // DTU config
    uint64_t dtu_serial = 0x99978563412;                 // Serial of the DTU (simply keep it as it is^^)
    rf24_pa_dbm_e dtu_transmission_power = RF24_PA_LOW;  // transmission power of the RF24 module

    // Inverter config
    std::vector<inverterConfig> _inverters;
    uint8_t _currentInverterIndex = 0;
   
    // Internal settings
    const unsigned long INTERVAL = 30000;  // sensor update interval
    const unsigned long LAST_SEEN_MAX = 2; // times INTERVAL where no change was received
    unsigned long last_millis = 0;
    bool offline_update_sent = false;

    // Sensors
    std::map<uint8_t, HoymilesStatusSensor *> _binary_sensors;
    std::map<uint8_t, std::vector<HoymilesSensor *>> _sensors;

 public:
  HoymilesDTU(std::vector<inverterConfig> inverters)
  {
    this->_inverters = inverters;
  }

  void setup() override {
    ESP_LOGI("hoymiles", "Setting up hoymiles DTU with %d inverters", this->_inverters.size());
    Hoymiles::Hoymiles.init();

    Hoymiles::Hoymiles.getRadio()->setPALevel(dtu_transmission_power);
    Hoymiles::Hoymiles.getRadio()->setDtuSerial(dtu_serial);
    Hoymiles::Hoymiles.setPollInterval(5);

    for (int i = 0; i < _inverters.size(); i++) {
      auto cfg = _inverters[i];

      ESP_LOGI("hoymiles", "Setting up inverter %d (%s)", i, cfg.name);

      auto inv = Hoymiles::Hoymiles.addInverter(cfg.name, cfg.serial);
      if (inv != nullptr) {
        for (int s = 0; s < cfg.strings.size(); s++) {
          inv->Statistics()->setChannelMaxPower(s, cfg.strings[s]);
        }
      } else {
          ESP_LOGE("hoymiles", "Failed to add inverter %s!", cfg.name);
      }
    }

    ESP_LOGI("hoymiles", "Setup finished for %d inverters", Hoymiles::Hoymiles.getNumInverters());
  }

  void loop() override {
    Hoymiles::Hoymiles.loop();

    if(millis() - last_millis > INTERVAL) {
      time_t now =  homeassistant_time->timestamp_now();
      ESP_LOGV("hoymiles", "Current time: %d", now);
      auto inv = Hoymiles::Hoymiles.getInverterByPos(this->_currentInverterIndex);
      if (inv == nullptr) {
        ESP_LOGE("hoymiles", "Inverter %d not ready/found!", this->_currentInverterIndex);
        return;
      }

      ESP_LOGD("hoymiles", "Publishing inverter %d (%s)", this->_currentInverterIndex, inv->name());

      ESP_LOGV("hoymiles", "Last request %d, last response %d, last stats %d", inv->lastRequest(), inv->lastResponse(), inv->Statistics()->getLastUpdate());

      auto is_online = inv->online();

      HoymilesStatusSensor *bs = this->_binary_sensors[this->_currentInverterIndex];
      if (bs != NULL) {
        bs->publish_state(is_online);
      }

      if (is_online) {
        offline_update_sent = false;
        for (HoymilesSensor *s : this->_sensors[this->_currentInverterIndex])
        {
          if (s == NULL) {
            continue;
          }
          auto val = inv->Statistics()->getChannelFieldValue(s->channel(), s->field());
          s->publish_state(val);
        }
      } else if(!offline_update_sent){
        offline_update_sent = true;
        for (HoymilesSensor *s : this->_sensors[this->_currentInverterIndex])
        {
          if (s == NULL) {
            continue;
          }
          s->publish_state(0.0);
        }
      }

      ESP_LOGD("hoymiles", "Published inverter %d (%s)", this->_currentInverterIndex, inv->name());

      if (++this->_currentInverterIndex >= Hoymiles::Hoymiles.getNumInverters()) {
        this->_currentInverterIndex = 0;
      }

      last_millis = millis();
    }
  }

  BinarySensor *get_status_sensor(uint8_t inverterIndex)
  {
    HoymilesStatusSensor *binarySensor = new HoymilesStatusSensor(this, inverterIndex);
    _binary_sensors[inverterIndex] = binarySensor;
    return binarySensor;
  }

  Sensor *get_sensor(uint8_t inverterIndex, uint8_t channel, uint8_t field)
  {
    HoymilesSensor *normalSensor = new HoymilesSensor(this, inverterIndex, channel, field);
    _sensors[inverterIndex].push_back(normalSensor);
    return normalSensor;
  }
};