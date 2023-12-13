#ifndef UNIT_TEST
#include "esphome/core/log.h"
#endif

#include "modbus_data_splitter.h"

using std::vector;

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusDataSplitter";

vector<ModbusData*>* ModbusDataSplitter::split_request_and_response_data(ModbusFrame* request, ModbusFrame* response) {
  vector<ModbusData*>* split_data { nullptr };

  if (!address_and_function_match(request, response)) {
    return nullptr;
  }
  switch (request->get_function()) {
    case 3: {
      split_data = handle_function_3(request, response);
      break;
    }
    default:
      break;
  }

  return split_data;
}

bool ModbusDataSplitter::address_and_function_match(ModbusFrame* request, ModbusFrame* response) {
  if ((nullptr == request) || (nullptr == response)) {
    return false;
  }
  if (request->get_address() != response->get_address()) {
    return false;
  }
  if (request->get_function() != response->get_function()) {
    return false;
  }
  return true;
}

vector<ModbusData*>* ModbusDataSplitter::handle_function_3(ModbusFrame* request, ModbusFrame* response) {
  if (request->get_data_length() != 4) {
    return nullptr;
  }
  // Check if the response contains the expected amount of data
  uint8_t *request_data = request->get_data();

  uint8_t start_address_high_byte = request_data[0];
  uint8_t start_address_low_byte = request_data[1];
  uint16_t start_address = (start_address_high_byte << 8) | start_address_low_byte;

  uint8_t register_count_requested_high_byte = request_data[2];
  uint8_t register_count_requested_low_byte = request_data[3];
  uint16_t register_count_requested = (register_count_requested_high_byte << 8) | register_count_requested_low_byte;

  uint16_t expected_bytes_in_response = register_count_requested * 2 + 1;
  if (response->get_data_length() != expected_bytes_in_response) {
    return nullptr;
  }
  uint8_t *response_data = response->get_data();
  if (response_data[0] != register_count_requested * 2) {
    return nullptr;
  }

  // Amount of data in response matches the request
  // Now process it
  vector<ModbusData*> *split_data = new vector<ModbusData*>;
  for (uint8_t i = 0; i < register_count_requested; ++i) {
    ModbusData *modbus_data_for_register = new ModbusData;
    modbus_data_for_register->address = start_address + i;
    uint8_t value_high_byte = response_data[i * 2 + 1];
    uint8_t value_low_byte = response_data[i * 2 + 2];
    modbus_data_for_register->value = (value_high_byte << 8) | value_low_byte;
    split_data->push_back(modbus_data_for_register);
  }
  return split_data;
}

} //namespace modbus_spy
} //namespace esphome
