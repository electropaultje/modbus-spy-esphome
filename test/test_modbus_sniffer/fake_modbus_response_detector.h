#ifndef FAKE_MODBUS_RESPONSE_DETECTOR_H_
#define FAKE_MODBUS_RESPONSE_DETECTOR_H_

#include "modbus_frame.h"
#include "modbus_response_detector.h"

using esphome::modbus_spy::IModbusResponseDetector;
using esphome::modbus_spy::ModbusFrame;

class FakeModbusResponseDetector : public IModbusResponseDetector {
 public:
  virtual ModbusFrame* detect_response() override {
    if (!this->has_been_invoked_) {
      this->has_been_invoked_ = true;
      return this->response_frame_;
    } else {
      return nullptr;
    }
  }

  void set_response(ModbusFrame* response_frame) { this->response_frame_ = response_frame; }
 
 protected:
  ModbusFrame* response_frame_;
  bool has_been_invoked_ { false };
};


#endif // FAKE_MODBUS_RESPONSE_DETECTOR_H_