# ESPHome Hoymiles DTU Sensor 

See: https://esphome.io/guides/getting_started_command_line.html and https://esphome.io/cookbook/arduino_port_extender.html

# Init (already done)
```
cd examples/esphome/config
docker run --rm -v "${PWD}":/config -it esphome/esphome wizard HoymilesSensor.yaml
```

# Upload
```
cd examples/esphome/config
docker run --rm -v "${PWD}":/config --device=/dev/ttyUSB1 -it esphome/esphome run HoymilesSensor.yaml
```