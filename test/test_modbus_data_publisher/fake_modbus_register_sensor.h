#ifndef FAKE_MODBUS_REGISTER_SENSOR_
#define FAKE_MODBUS_REGISTER_SENSOR_

#include <vector>

#include "modbus_register_sensor.h"

using std::vector;
using esphome::modbus_spy::IModbusRegisterSensor;

class FakeModbusRegisterSensor : public IModbusRegisterSensor {
 public:
  virtual void publish_state(uint16_t state) override;
  vector<uint16_t>* get_published_states();
 protected:
  vector<uint16_t> published_states_;
};


#endif // FAKE_MODBUS_REGISTER_SENSOR_