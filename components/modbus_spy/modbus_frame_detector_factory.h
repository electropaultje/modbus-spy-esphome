#ifndef MODBUS_FRAME_DETECTOR_FACTORY_
#define MODBUS_FRAME_DETECTOR_FACTORY_

#include "modbus_request_detector.h"
#include "modbus_response_detector.h"
#include "uart_interface.h"

namespace esphome {
namespace modbus_spy {

class ModbusFrameDetectorFactory {
 public: 
  static IModbusRequestDetector* create_request_detector(IUartInterface* uart_interface);
  static IModbusResponseDetector* create_response_detector(IUartInterface* uart_interface);
  static void clear_detectors();
  static void set_request_detector(IModbusRequestDetector* request_detector);
  static void set_response_detector(IModbusResponseDetector* response_detector);

 private:
  static IModbusRequestDetector* request_detector_;
  static IModbusResponseDetector* response_detector_;
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_FRAME_DETECTOR_FACTORY_