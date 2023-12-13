#ifndef ESP32_ARDUINO_UART_INTERFACE_H_
#define ESP32_ARDUINO_UART_INTERFACE_H_

#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"

#include "modbus_spy.h"
#include "uart_interface.h"

namespace esphome {
namespace modbus_spy {

class Esp32ArduinoUartInterface : public IUartInterface {
 public:
  Esp32ArduinoUartInterface(ModbusSpy* uart_device) : uart_device_(uart_device) {}

  virtual bool read_byte(uint8_t* data) override {
    return this->uart_device_->read_byte(data);
  }

  virtual int available() const override {
    return this->uart_device_->available();
  }
  
  virtual uint32_t get_baud_rate() const override {
    return this->uart_device_->get_baud_rate();
  }
  
 protected:
  ModbusSpy* uart_device_;
};

} //namespace modbus_spy
} //namespace esphome

#endif // ESP32_ARDUINO_UART_INTERFACE_H_