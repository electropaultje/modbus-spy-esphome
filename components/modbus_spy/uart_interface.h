#ifndef UART_INTERFACE_H_
#define UART_INTERFACE_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

namespace esphome {
namespace modbus_spy {

class IUartInterface {
 public:
  virtual bool read_byte(uint8_t* data) = 0;
  virtual int available() const = 0;
  virtual uint32_t get_baud_rate() const = 0;
};

} //namespace modbus_spy
} //namespace esphome

#endif // UART_INTERFACE_H_