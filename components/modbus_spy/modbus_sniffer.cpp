#include <vector>

#include <Arduino.h>
#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#include "esphome/core/log.h"
#endif // UNIT_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "modbus_data.h"
#include "modbus_data_splitter.h"
#include "modbus_frame.h"
#include "modbus_frame_detector_factory.h"
#include "modbus_sniffer.h"

using std::vector;

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusSniffer";

ModbusSniffer::ModbusSniffer(IUartInterface* uart_interface, IModbusDataPublisher* data_publisher) :
    uart_interface_(uart_interface),
    data_publisher_(data_publisher) {
}

ModbusSniffer::~ModbusSniffer() {
  if (this->is_sniffing_) {
    stop_sniffing();
  }
}

void ModbusSniffer::start_sniffing() {
  ESP_LOGD(TAG, "ModbusSniffer::start_sniffing");
  xTaskCreatePinnedToCore(ModbusSniffer::sniff_loop_task,
                              "sniff_task", // name
                              50000,        // stack size (in words)
                              this,         // input params
                              1,            // priority
                              nullptr,      // Handle, not needed
                              0             // core
  );
}

void ModbusSniffer::stop_sniffing() {
  this->should_stop_sniffing_ = true;
  while (this->is_sniffing_) {
    delay(1);
  }
}

void ModbusSniffer::sniff_loop_task(void* params) {
  ModbusSniffer *modbus_sniffer = reinterpret_cast<ModbusSniffer*>(params);
  IModbusRequestDetector *request_detector = 
    ModbusFrameDetectorFactory::create_request_detector(modbus_sniffer->uart_interface_);
  IModbusResponseDetector *response_detector =
    ModbusFrameDetectorFactory::create_response_detector(modbus_sniffer->uart_interface_);
  ModbusDataSplitter data_splitter;
  while (true) {
    if (modbus_sniffer->should_stop_sniffing_) {
      ModbusFrameDetectorFactory::clear_detectors();
      delete request_detector;
      delete response_detector;
      modbus_sniffer->is_sniffing_ = false;
      vTaskDelete(NULL);
      break;
    }
    delay(5);

    ModbusFrame *request_frame = request_detector->detect_request();
    if (nullptr == request_frame) {
      modbus_sniffer->empty_rx_buffer();
      continue;
    }
    ModbusFrame *response_frame = response_detector->detect_response();
    if (nullptr == response_frame) {
      delete request_frame;
      modbus_sniffer->empty_rx_buffer();
      continue;
    }
    vector<ModbusData*> *split_data = data_splitter.split_request_and_response_data(request_frame, response_frame);
    if (nullptr == split_data) {
      delete request_frame;
      delete response_frame;
      modbus_sniffer->empty_rx_buffer();
      continue;
    }
    uint8_t device_address = request_frame->get_address();
    uint8_t function = request_frame->get_function();
    delete request_frame;
    delete response_frame;
    modbus_sniffer->data_publisher_->publish_data(device_address, function, split_data);
    for (ModbusData* modbus_data : *split_data) {
      delete modbus_data;
    }
    delete split_data;
  }
}

void ModbusSniffer::empty_rx_buffer() {
  uint8_t dummy_byte { 0 };
  while (this->uart_interface_->available() != 0) {
    this->uart_interface_->read_byte(&dummy_byte);
  }
}

} // namespace modbus_spy
} // namespace esphome