#ifndef FAKE_MODBUS_BINARY_SENSOR_
#define FAKE_MODBUS_BINARY_SENSOR_

#include <vector>

#include "modbus_binary_sensor.h"

using std::vector;
using esphome::modbus_spy::IModbusBinarySensor;

class FakeModbusBinarySensor : public IModbusBinarySensor {
 public:
  virtual void publish_state(bool state) override;
  vector<bool>* get_published_states();
 protected:
  vector<bool> published_states_;
};


#endif // FAKE_MODBUS_BINARY_SENSOR_