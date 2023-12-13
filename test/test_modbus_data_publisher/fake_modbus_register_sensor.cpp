#include "fake_modbus_register_sensor.h"

void FakeModbusRegisterSensor::publish_state(uint16_t state) {
  this->published_states_.push_back(state);
}

vector<uint16_t>* FakeModbusRegisterSensor::get_published_states() {
  return &this->published_states_;
}