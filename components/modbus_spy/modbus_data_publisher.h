#ifndef MODBUS_DATA_PUBLISHER_H_
#define MODBUS_DATA_PUBLISHER_H_

#include <map>
#include <vector>

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#include "modbus_binary_sensor.h"
#include "modbus_data.h"
#include "modbus_register_sensor.h"

namespace esphome {
namespace modbus_spy {

class IModbusDataPublisher {
 public:
  virtual ~IModbusDataPublisher();
  virtual void add_register_sensor(uint8_t device_address, uint16_t register_address, IModbusRegisterSensor* register_sensor) = 0;
  virtual void add_binary_sensor(uint8_t device_address, uint16_t register_address, int8_t bit, IModbusBinarySensor* binary_sensor) = 0;
  virtual void publish_data(uint8_t device_address, uint8_t function, std::vector<ModbusData*>* data) = 0;
};

class ModbusDataPublisher : public IModbusDataPublisher {
 private:
  typedef struct DeviceSensors {
    std::map<uint16_t, IModbusRegisterSensor*>* register_sensors_ { nullptr };
    std::map<uint16_t, IModbusBinarySensor*>* binary_sensors_full_register_ { nullptr };
    std::map<uint16_t, IModbusBinarySensor**>* binary_sensors_bit_ { nullptr };
  } DeviceSensors;

 public:
  ModbusDataPublisher(bool dump_not_configured_data = false);

 private:
  ModbusDataPublisher(const ModbusDataPublisher&);

 public:
  virtual ~ModbusDataPublisher() override;
  
  virtual void add_register_sensor(uint8_t device_address, uint16_t register_address, IModbusRegisterSensor* register_sensor) override;
  virtual void add_binary_sensor(uint8_t device_address, uint16_t register_address, int8_t bit, IModbusBinarySensor* binary_sensor) override;
  virtual void publish_data(uint8_t device_address, uint8_t function, std::vector<ModbusData*>* data) override;

 protected:
  std::map<uint8_t, DeviceSensors*> device_sensors_;
  bool should_dump_not_configured_data_ { false };
  IModbusRegisterSensor* find_register_sensor(uint8_t device_address, uint16_t data_model_register_address);
  IModbusBinarySensor* find_binary_sensor_full_register(uint8_t device_address, uint16_t data_model_register_address);
  IModbusBinarySensor** find_binary_bit_sensors(uint8_t device_address, uint16_t data_model_register_address);

 private:
  uint16_t convert_pdu_address_to_data_model_address(uint8_t function, uint16_t pdu_address);
  void find_sensor_and_publish_data(uint8_t device_address, uint16_t data_model_register_address, uint16_t value);
  std::map<uint16_t, IModbusRegisterSensor*>* get_register_sensors_for_device(uint8_t device_address);
  std::map<uint16_t, IModbusBinarySensor*>* get_binary_sensors_full_register_for_device(uint8_t device_address);
  std::map<uint16_t, IModbusBinarySensor**>* get_binary_sensors_bit_for_device(uint8_t device_address);
  DeviceSensors* get_sensors_for_device(uint8_t device_address);
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_DATA_PUBLISHER_H_