#ifndef MODBUS_BINARY_SENSOR_H_
#define MODBUS_BINARY_SENSOR_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

namespace esphome {
namespace modbus_spy {

class IModbusBinarySensor {
 public:
  virtual ~IModbusBinarySensor();
  virtual void publish_state(bool state) = 0;
};

#ifndef UNIT_TEST

class ModbusBinarySensor : public IModbusBinarySensor {
 public:
  ModbusBinarySensor();
  virtual ~ModbusBinarySensor() override;

  virtual void publish_state(bool state) override;
  binary_sensor::BinarySensor* get_sensor() const;

 protected:
  binary_sensor::BinarySensor* sensor_;
};

#endif // UNIT_TEST

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_BINARY_SENSOR_H_