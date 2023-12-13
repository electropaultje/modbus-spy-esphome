#ifndef TESTABLE_MODBUS_DATA_PUBLISHER_
#define TESTABLE_MODBUS_DATA_PUBLISHER_

#include "modbus_binary_sensor.h"
#include "modbus_data_publisher.h"
#include "modbus_register_sensor.h"

using esphome::modbus_spy::IModbusBinarySensor;
using esphome::modbus_spy::ModbusDataPublisher;
using esphome::modbus_spy::IModbusRegisterSensor;

class TestableModbusDataPublisher : public ModbusDataPublisher {
 public:
  TestableModbusDataPublisher() : ModbusDataPublisher(false) {}
  IModbusRegisterSensor* call_find_register_sensor(uint8_t device_address, uint16_t data_model_register_address) {
    return find_register_sensor(device_address, data_model_register_address);
  }
};


#endif // TESTABLE_MODBUS_DATA_PUBLISHER_