#ifndef UNIT_TEST

#include "esphome/core/log.h"

#include "esp32_arduino_uart_interface.h"
#include "modbus_binary_sensor.h"
#include "modbus_register_sensor.h"
#include "modbus_spy.h"

using namespace std; 

namespace esphome {
namespace modbus_spy {

static const char *TAG = "modbus_spy";

ModbusSpy::ModbusSpy(bool log_not_configured_data) : data_publisher_(log_not_configured_data) {
  Esp32ArduinoUartInterface *uart_interface = new Esp32ArduinoUartInterface(this);
  this->sniffer_ = new ModbusSniffer(uart_interface, &this->data_publisher_);
}

ModbusSpy::~ModbusSpy() {
  delete this->sniffer_;
}

sensor::Sensor* ModbusSpy::create_sensor(uint8_t device_address, uint16_t register_address) {
  ModbusRegisterSensor *register_sensor = new ModbusRegisterSensor();
  this->data_publisher_.add_register_sensor(device_address, register_address, register_sensor);
  return register_sensor->get_sensor();
}

binary_sensor::BinarySensor* ModbusSpy::create_binary_sensor(
  uint8_t device_address,
  uint16_t register_address,
  int8_t bit
) {
  ModbusBinarySensor *binary_sensor = new ModbusBinarySensor();
  this->data_publisher_.add_binary_sensor(device_address, register_address, bit, binary_sensor);
  return binary_sensor->get_sensor();
}

void ModbusSpy::setup() {
  this->sniffer_->start_sniffing();
}

void ModbusSpy::loop() {
}

float ModbusSpy::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void ModbusSpy::dump_config() {
  ESP_LOGCONFIG(TAG, "ModbusSpy");
}

} // namespace modbus_spy
} // namespace esphome

#endif // UNIT_TEST