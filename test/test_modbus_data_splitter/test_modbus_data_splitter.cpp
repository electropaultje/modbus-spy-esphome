#include <vector>

#include <Arduino.h>
#include <unity.h>

#include "modbus_data.h"
#include "modbus_data_splitter.h"
#include "modbus_frame.h"
#include <test_includes.h>

using std::vector;
using namespace esphome::modbus_spy;

void test_nullptr_request_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *response_data = new uint8_t[5] { 0x04, 0x01, 0xAB, 0x02, 0x5A };
  ModbusFrame response(0x0B, 3, response_data, 5);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(nullptr, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_nullptr_response_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x02 };
  ModbusFrame request(0x0B, 3, request_data, 4);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, nullptr);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_different_function_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x01 };
  ModbusFrame request(0x0A, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[2] { 0x01, 0x04 };
  ModbusFrame response(0x0A, 1, response_data, 2);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_different_address_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x01 };
  ModbusFrame request(0x0A, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[3] { 0x02, 0x34, 0x56 };
  ModbusFrame response(0x0B, 3, response_data, 3);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_matching_pair_function_3_with_one_holding_register() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x01 };
  ModbusFrame request(0x0B, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[3] { 0x02, 0x34, 0x56 };
  ModbusFrame response(0x0B, 3, response_data, 3);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data != nullptr);
  TEST_ASSERT_EQUAL_UINT8(1, split_data->size());
  ModbusData* data0 = split_data->at(0);
  TEST_ASSERT_EQUAL_UINT8(0x04AF, data0->address);
  TEST_ASSERT_EQUAL_UINT16(0x3456, data0->value);
}

void test_matching_pair_function_3_with_two_holding_registers() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x02 };
  ModbusFrame request(0x0B, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[5] { 0x04, 0x01, 0xAB, 0x02, 0x5A };
  ModbusFrame response(0x0B, 3, response_data, 5);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data != nullptr);
  TEST_ASSERT_EQUAL_UINT8(2, split_data->size());
  ModbusData* data0 = split_data->at(0);
  TEST_ASSERT_EQUAL_UINT8(0x04AF, data0->address);
  TEST_ASSERT_EQUAL_UINT16(0x01AB, data0->value);
  ModbusData* data1 = split_data->at(1);
  TEST_ASSERT_EQUAL_UINT8(0x04B0, data1->address);
  TEST_ASSERT_EQUAL_UINT16(0x025A, data1->value);
}

void test_incorrect_request_data_length_function_3_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[3] { 0x04, 0xAF, 0x00 };
  ModbusFrame request(0x0B, 3, request_data, 3);
  uint8_t *response_data = new uint8_t[3] { 0x02, 0x34, 0x56 };
  ModbusFrame response(0x0B, 3, response_data, 3);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_incorrect_byte_count_in_response_data_function_3_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x01 };
  ModbusFrame request(0x0B, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[3] { 0x03, 0x34, 0x56 };
  ModbusFrame response(0x0B, 3, response_data, 3);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}

void test_incorrect_data_length_in_response_function_3_results_in_nullptr() {
  // Arrange
  ModbusDataSplitter data_splitter;
  uint8_t *request_data = new uint8_t[4] { 0x04, 0xAF, 0x00, 0x01 };
  ModbusFrame request(0x0B, 3, request_data, 4);
  uint8_t *response_data = new uint8_t[2] { 0x02, 0x34 };
  ModbusFrame response(0x0B, 3, response_data, 2);

  // Act
  vector<ModbusData*>* split_data = data_splitter.split_request_and_response_data(&request, &response);

  // Assert
  TEST_ASSERT_TRUE(split_data == nullptr);
}
int runUnityTests(void) {
  UNITY_BEGIN();

  // ModbusDataSplitter tests
  RUN_TEST(test_nullptr_request_results_in_nullptr);
  RUN_TEST(test_nullptr_response_results_in_nullptr);
  RUN_TEST(test_different_function_results_in_nullptr);
  RUN_TEST(test_different_address_results_in_nullptr);
  RUN_TEST(test_matching_pair_function_3_with_one_holding_register);
  RUN_TEST(test_matching_pair_function_3_with_two_holding_registers);
  RUN_TEST(test_incorrect_request_data_length_function_3_results_in_nullptr);
  RUN_TEST(test_incorrect_byte_count_in_response_data_function_3_results_in_nullptr);
  RUN_TEST(test_incorrect_data_length_in_response_function_3_results_in_nullptr);

  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}