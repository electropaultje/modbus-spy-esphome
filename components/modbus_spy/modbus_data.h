#ifndef MODBUS_DATA_H_
#define MODBUS_DATA_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

namespace esphome {
namespace modbus_spy {

typedef struct ModbusData {
  uint16_t address { 0 };
  uint16_t value { 0 };
} ModbusData;

} // namespace esphome
} // namespace modbus_spy

#endif // MODBUS_DATA_H_