#ifndef MODBUS_FRAME_H_
#define MODBUS_FRAME_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#include "esphome/core/log.h"
#endif // UNIT_TEST

namespace esphome {
namespace modbus_spy {

class ModbusFrame {
 public:
  ModbusFrame(uint8_t address, uint8_t function, uint8_t* data, uint8_t data_length);
  ~ModbusFrame();

 private:
  ModbusFrame(const ModbusFrame&);

 public:
  uint8_t get_address() const { return this->address_; }
  uint8_t get_function() const { return this->function_; }
  uint8_t* get_data() const { return this->data_; }
  uint8_t get_data_length() const { return this->data_length_; }

 protected:
  uint8_t address_;
  uint8_t function_;
  uint8_t* data_;
  uint8_t data_length_;
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_FRAME_H_