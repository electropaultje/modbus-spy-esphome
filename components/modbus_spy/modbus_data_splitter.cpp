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
    ESP_LOGD(TAG, "Request and response do not match in address or function");
    return nullptr;
  }
  switch (request->get_function()) {
    case 3: [[fallthrough]];
    case 4: {
      split_data = handle_function_3_and_4(request, response);
      break;
    }
    case 6: {
      split_data = handle_function_6(request, response);
      break;
    }
    default:
      ESP_LOGD(TAG, "Unsupported Modbus function: %d", request->get_function());
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

vector<ModbusData*>* ModbusDataSplitter::handle_function_3_and_4(ModbusFrame* request, ModbusFrame* response) {
  if (request->get_data_length() != 4) {
    ESP_LOGD(TAG, "Request data length for function 3/4 is not 4, but %d", request->get_data_length());
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
    ESP_LOGD(TAG, "Response data length for function 3/4 does not match expected length: expected %d, got %d",
             expected_bytes_in_response, response->get_data_length());
    return nullptr;
  }
  uint8_t *response_data = response->get_data();
  if (response_data[0] != register_count_requested * 2) {
    ESP_LOGD(TAG, "Response byte count does not match requested register count: expected %d, got %d",
             register_count_requested * 2, response_data[0]);
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

vector<ModbusData*>* ModbusDataSplitter::handle_function_6(ModbusFrame* request, ModbusFrame* response) {
  if ((request->get_data_length() != 4) || (response->get_data_length() != 4)) {
    return nullptr;
  }

  // Response data should be identical to request data
  uint8_t *request_data = request->get_data();
  uint8_t *response_data = response->get_data();
  for (uint8_t i = 0; i < 4; ++i) {
    if (response_data[i] != request_data[i]) {
      return nullptr;
    }
  }

  // Passed all tests! Let's get the register's address, and the value set in it
  uint8_t address_high_byte = request_data[0];
  uint8_t address_low_byte = request_data[1];
  uint16_t address = (address_high_byte << 8) | address_low_byte;

  uint8_t value_high_byte = request_data[2];
  uint8_t value_low_byte = request_data[3];
  uint16_t value = (value_high_byte << 8) | value_low_byte;

  uint16_t data_model_address = 40001 + address;

  vector<ModbusData*> *split_data = new vector<ModbusData*>;
  ModbusData *modbus_data = new ModbusData;
  modbus_data->address = address;
  modbus_data->value = value;
  split_data->push_back(modbus_data);

  return split_data;
}

} //namespace modbus_spy
} //namespace esphome
