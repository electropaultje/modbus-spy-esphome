# modbus-spy-esphome

## What is this all about?

This repository contains an integration for a Modbus spy in ESPHome.
It provides access to data exchanged on a Modbus data bus without sending data on the bus itself.

Originally it was intended to get live data from an Itho Daalderop HP-S95 fully electric heat pump to integrate with ESPHome in Home Assistant.

It allows a user to configure the modbus_spy component in /components as an external component.
The component can listen to Modbus messages between a client and servers on a bus in RTU transmission mode.
Modbus data in the request-response pair can then be published to configured sensors.
These sensors are fully configurable in the YAML.

## How to use it

### Hardware

First of all, some hardware is required. In principle this component works on an ESP32 that has a UART RX connected to the Modbus hardware (e.g. using an RS-485 transceiver chip / module).

#### Ready-to-use Hardware
I have designed a PCB that needs to be powered using a micro USB B connector.  
The board can be connected to the bus using a detachable screw terminal (one part of the connector sits on the PCB, the bus wires are fastened with screws in the other part). An ESP32 WROVER module is part of the PCB, and takes care of the ESPHome integration.

I have a few of these boards available, and can produce more on demand for a fair price.

### Software

This repository contains the modbus_spy component. It requires sensors to be defined in the YAML.
Sensor addresses need to be the Modbus data model addresses (e.g.: in the 40001-49999 range for "normal" 16-bit registers), and NOT the PDU address (always in range 0-65535, e.g.: 0x04AF).

### Example YAML

This YAML is not complete. It also requires things like ota, wifi, api etc.  
It is merely intended to show the parts relevant for this integration.
```
esp32: 
  board: esp32dev
  framework:
    type: arduino

external_components:
  - source:
      type: git
      url: https://github.com/pdjong/modbus-spy-esphome
      ref: 1.0

# Note: The uart settings depend on the Modbus configuration of the system that you connect it to
uart:
  baud_rate: 19200
  rx_pin: 32
  parity: ODD

modbus_spy:
  id: modbusspy
  log_not_configured_data: true
  
sensor:
  - platform: modbus_spy
    name: "Inside Temperature"
    device_class: "temperature"
    accuracy_decimals: 1
    state_class: "measurement"
    unit_of_measurement: "°C"
    icon: "mdi:thermometer"    
    device_address: 0x0B
    register_address: 41200
    filters:
      - offset: -3000
      - multiply: .01
  - platform: modbus_spy
    name: "Sanitary Hot Water Temperature"
    device_class: "temperature"
    accuracy_decimals: 1
    state_class: "measurement"
    unit_of_measurement: "°C"
    icon: "mdi:thermometer"    
    device_address: 0x0B
    register_address: 41201
    filters:
      - offset: -3000
      - multiply: .01

binary_sensor:
  - platform: modbus_spy
    name: "Pump 1 Active"
    device_address: 0x0B
    register_address: 41301
  - platform: modbus_spy
    name: "Internal Pump Active"
    device_address: 0x0B
    register_address: 41213
```

As you can see, the temperature sensors have a filter to offset by -3000 and then divide by 100. A value of 5380 results in 23,80 °C. 
This is done because Itho Daalderop defined it like that, probably so that a wide range of negative and positive values with an accuracy of 0.01 °C is supported. 
Note that this is just an example and it is required to put suitable filters in as needed.

# More info

For more information, look at [more-info.md](more-info.md)