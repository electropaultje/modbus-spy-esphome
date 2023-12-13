#ifndef FAKE_MODBUS_DATA_PUBLISHER_H_
#define FAKE_MODBUS_DATA_PUBLISHER_H_

#include <vector>

#include "modbus_binary_sensor.h"
#include "modbus_data.h"
#include "modbus_data_publisher.h"
#include "modbus_register_sensor.h"

using std::vector;
using esphome::modbus_spy::IModbusBinarySensor;
using esphome::modbus_spy::IModbusDataPublisher;
using esphome::modbus_spy::IModbusRegisterSensor;
using esphome::modbus_spy::ModbusData;

class PublishedData {
 public:
  uint8_t device_address { 0 };
  uint8_t function { 0 };
  ModbusData* modbus_data { nullptr };
};

class FakeModbusDataPublisher : public IModbusDataPublisher {
 public:
  virtual ~FakeModbusDataPublisher() override;
  virtual void publish_data(uint8_t device_address, uint8_t function, std::vector<ModbusData*>* data);
  virtual void add_register_sensor(uint8_t device_address, uint16_t register_address, IModbusRegisterSensor* register_sensor) override {}
  virtual void add_binary_sensor(uint8_t device_address, uint16_t register_address, int8_t bit, IModbusBinarySensor* binary_sensor) override {}
  vector<PublishedData*>* get_published_data() { return &this->published_data_; }
  
 protected:
  vector<PublishedData*> published_data_;
};


#endif // FAKE_MODBUS_DATA_PUBLISHER_H_