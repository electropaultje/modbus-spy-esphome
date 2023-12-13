#include "modbus_frame.h"

namespace esphome {
namespace modbus_spy {

ModbusFrame::ModbusFrame(uint8_t address, uint8_t function, uint8_t* data, uint8_t data_length) : 
    address_(address), function_(function), data_(data), data_length_(data_length) {}

ModbusFrame::~ModbusFrame() {
  if (this->data_ != nullptr) {
    delete[] this->data_;
    this->data_ = nullptr;
  }
}

} //namespace modbus_spy
} //namespace esphome
