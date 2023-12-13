#include "modbus_frame_detector_factory.h"

using namespace std; 

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusFrameDetectorFactory";

IModbusRequestDetector* ModbusFrameDetectorFactory::request_detector_ = nullptr;
IModbusResponseDetector* ModbusFrameDetectorFactory::response_detector_ = nullptr;

IModbusRequestDetector* ModbusFrameDetectorFactory::create_request_detector(IUartInterface* uart_interface) {
  if (nullptr == request_detector_) {
    ModbusFrameDetectorFactory::request_detector_ = new ModbusRequestDetector(uart_interface);
  }
  return ModbusFrameDetectorFactory::request_detector_;
}

IModbusResponseDetector* ModbusFrameDetectorFactory::create_response_detector(IUartInterface* uart_interface) {
  if (nullptr == response_detector_) {
    ModbusFrameDetectorFactory::response_detector_ = new ModbusResponseDetector(uart_interface);
  }
  return ModbusFrameDetectorFactory::response_detector_;
}

void ModbusFrameDetectorFactory::clear_detectors() {
  set_request_detector(nullptr);
  set_response_detector(nullptr);
}

void ModbusFrameDetectorFactory::set_request_detector(IModbusRequestDetector* request_detector) {
  ModbusFrameDetectorFactory::request_detector_ = request_detector;
}

void ModbusFrameDetectorFactory::set_response_detector(IModbusResponseDetector* response_detector) {
  ModbusFrameDetectorFactory::response_detector_ = response_detector;
}

} // namespace modbus_spy
} // namespace esphome