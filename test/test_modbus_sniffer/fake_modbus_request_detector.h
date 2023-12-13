#ifndef FAKE_MODBUS_REQUEST_DETECTOR_H_
#define FAKE_MODBUS_REQUEST_DETECTOR_H_

#include "modbus_frame.h"
#include "modbus_request_detector.h"

using esphome::modbus_spy::IModbusRequestDetector;
using esphome::modbus_spy::ModbusFrame;

class FakeModbusRequestDetector : public IModbusRequestDetector {
 public:
  virtual ModbusFrame* detect_request() override { 
    if (!this->has_been_invoked_) {
      this->has_been_invoked_ = true;
      return this->request_frame_;
    } else {
      return nullptr;
    }
  }
  
  void set_request(ModbusFrame* request_frame) { this->request_frame_ = request_frame; }
 
 protected:
  ModbusFrame* request_frame_;
  bool has_been_invoked_ { false };
};


#endif // FAKE_MODBUS_REQUEST_DETECTOR_H_