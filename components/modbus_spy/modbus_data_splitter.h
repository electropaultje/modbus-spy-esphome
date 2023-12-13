#ifndef MODBUS_DATA_SPLITTER_H_
#define MODBUS_DATA_SPLITTER_H_

#include <vector>

#include "modbus_data.h"
#include "modbus_frame.h"
#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

namespace esphome {
namespace modbus_spy {

class ModbusDataSplitter {
 public:
  std::vector<ModbusData*>* split_request_and_response_data(ModbusFrame* request, ModbusFrame* response);

 private:
  bool address_and_function_match(ModbusFrame* request, ModbusFrame* response);
  std::vector<ModbusData*>* handle_function_3(ModbusFrame* request, ModbusFrame* response);
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_DATA_SPLITTER_H_