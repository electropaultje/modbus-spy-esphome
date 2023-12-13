#ifndef MODBUS_REQUEST_DETECTOR_H_
#define MODBUS_REQUEST_DETECTOR_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#include "modbus_frame.h"
#include "uart_interface.h"

namespace esphome {
namespace modbus_spy {

class IModbusRequestDetector {
 public:
  virtual ModbusFrame* detect_request() = 0;
};

class ModbusRequestDetector : public IModbusRequestDetector {
 private:
  static const uint32_t MAX_TIME_TO_WAIT_FOR_REQUEST_IN_MS = 500;
  static const uint16_t MIN_TIMEOUT_BETWEEN_BYTES_IN_US = 750;

 public:
  ModbusRequestDetector(IUartInterface* uart_interface);
  
 private:
  ModbusRequestDetector(ModbusRequestDetector&);

 public:
  virtual ModbusFrame* detect_request() override;

 protected:
  IUartInterface* uart_interface_ { nullptr };
  uint32_t time_last_byte_received_ { 0 };
  uint16_t max_time_between_bytes_in_us_ { 860 };

 private:
  bool read_next_byte(uint8_t* byte);
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_REQUEST_DETECTOR_H_