#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/datatypes.h"
#include "esphome/core/log.h"
#endif // UNIT_TEST

#include "modbus_binary_sensor.h"

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusBinarySensor";

IModbusBinarySensor::~IModbusBinarySensor() {
}

#ifndef UNIT_TEST

ModbusBinarySensor::ModbusBinarySensor() {
  this->sensor_ = new binary_sensor::BinarySensor;
}

ModbusBinarySensor::~ModbusBinarySensor() {
  delete this->sensor_;
}

void ModbusBinarySensor::publish_state(bool state) {
  ESP_LOGV(TAG, "About to publish state: %d", state);
  this->sensor_->publish_state(state);
}

binary_sensor::BinarySensor* ModbusBinarySensor::get_sensor() const {
  return this->sensor_;
}

#endif // UNIT_TEST

} //namespace modbus_spy
} //namespace esphome
