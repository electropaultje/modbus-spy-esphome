#include <vector>

#include <Arduino.h>
#include <unity.h>

#include "fake_modbus_data_publisher.h"
#include "fake_modbus_request_detector.h"
#include "fake_modbus_response_detector.h"
#include "fake_uart_interface.h"
#include "modbus_data.h"
#include "modbus_frame.h"
#include "modbus_frame_detector_factory.h"
#include "modbus_response_detector.h"
#include "modbus_request_detector.h"
#include "modbus_sniffer.h"
#include <test_includes.h>

using std::vector;
using namespace esphome::modbus_spy;

// What to test?
// - combination of actual request- and response detector, using fake uart interface
// - combination of matching and non-matching results of detectors, using fake detectors

void test_modbus_sniffer_with_fake_detectors_good() {
  // Arrange
  FakeModbusRequestDetector *request_detector = new FakeModbusRequestDetector;
  uint8_t *request_data = new uint8_t[4] { 0x01, 0xAF, 0x00, 0x01 };
  ModbusFrame *request_frame = new ModbusFrame(0x09, 3, request_data, 4);
  request_detector->set_request(request_frame);
  ModbusFrameDetectorFactory::set_request_detector(request_detector);
  
  FakeModbusResponseDetector *response_detector = new FakeModbusResponseDetector;
  uint8_t *response_data = new uint8_t[3] { 0x02, 0xAB, 0xCD };
  ModbusFrame *response_frame = new ModbusFrame(0x09, 3, response_data, 3);
  response_detector->set_response(response_frame);
  ModbusFrameDetectorFactory::set_response_detector(response_detector);

  FakeUartInterface fake_uart_interface;
  FakeModbusDataPublisher fake_data_publisher;
  ModbusSniffer modbus_sniffer(&fake_uart_interface, &fake_data_publisher);

  // Act
  modbus_sniffer.start_sniffing();
  delay(20);
  modbus_sniffer.stop_sniffing();
  // Delay 10 ms to make sure that the sniffer task is done
  delay(20);

  // Assert
  vector<PublishedData*> *published_data = fake_data_publisher.get_published_data();
  TEST_ASSERT_EQUAL_UINT8(1, published_data->size());
  PublishedData *published_data_item = published_data->at(0);
  TEST_ASSERT_EQUAL_UINT8(0x09, published_data_item->device_address);
  TEST_ASSERT_EQUAL_UINT8(3, published_data_item->function);
  TEST_ASSERT_EQUAL_UINT16(0x01AF, published_data_item->modbus_data->address);
  TEST_ASSERT_EQUAL_UINT16(0xABCD, published_data_item->modbus_data->value);
}

void test_modbus_sniffer_with_actual_detectors_good() {
  // Arrange
  FakeUartInterface fake_uart_interface;
  uint8_t *fake_data = new uint8_t[15] { 0x02, 0x03, 0x04, 0xAF, 0x00, 0x01, 0xB5, 0x28, 0x02, 0x03, 0x02, 0x34, 0x56, 0x6A, 0xBA };
  FakeModbusDataPublisher fake_data_publisher;
  ModbusSniffer modbus_sniffer(&fake_uart_interface, &fake_data_publisher);
  bool uart_task_should_stop = false;
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &fake_uart_interface,
    .initial_delay_in_ms = 0,
    .delay_between_bytes_in_us = 573,
    .data_to_return = fake_data,
    .len_of_data_to_return = 15,
    .should_stop = &uart_task_should_stop
  };
  TaskHandle_t uart_task_handle { nullptr };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    &uart_task_handle,          // Handle, not needed
                    0                           // core
  );

  // Act
  modbus_sniffer.start_sniffing();
  delay(15);
  modbus_sniffer.stop_sniffing();
  uart_task_should_stop = true;
  // Delay 15 ms to make sure that the fake uart and sniffer tasks are done
  delay(1200);

  // Assert
  vector<PublishedData*> *published_data = fake_data_publisher.get_published_data();
  TEST_ASSERT_EQUAL_UINT8(1, published_data->size());
  PublishedData *published_data_item = published_data->at(0);
  TEST_ASSERT_EQUAL_UINT8(0x02, published_data_item->device_address);
  TEST_ASSERT_EQUAL_UINT8(3, published_data_item->function);
  TEST_ASSERT_EQUAL_UINT16(0x04AF, published_data_item->modbus_data->address);
  TEST_ASSERT_EQUAL_UINT16(0x3456, published_data_item->modbus_data->value);
}

void test_modbus_sniffer_with_actual_detectors_no_data() {
  // Arrange
  FakeUartInterface fake_uart_interface;
  FakeModbusDataPublisher fake_data_publisher;
  ModbusSniffer modbus_sniffer(&fake_uart_interface, &fake_data_publisher);

  // Act
  modbus_sniffer.start_sniffing();
  delay(5);
  modbus_sniffer.stop_sniffing();
  // Delay to make sure that the sniffer task is done
  delay(1200);

  // Assert
  vector<PublishedData*> *published_data = fake_data_publisher.get_published_data();
  TEST_ASSERT_EQUAL_UINT8(0, published_data->size());
}

void generate_crc() {
  uint8_t crc_data[] = { 0x02, 0x03, 0x02, 0x34, 0x56 };
  uint16_t expected_crc = esphome::crc16(crc_data, 5);
  TEST_ASSERT_EQUAL_UINT16(expected_crc, 0);
}

int runUnityTests(void) {
  UNITY_BEGIN();

  // ModbusSniffer tests
  RUN_TEST(test_modbus_sniffer_with_fake_detectors_good);
  RUN_TEST(test_modbus_sniffer_with_actual_detectors_good);
  RUN_TEST(test_modbus_sniffer_with_actual_detectors_no_data);

  // CRC generation tool :P
  // RUN_TEST(generate_crc);

  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}