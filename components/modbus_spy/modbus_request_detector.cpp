#include <cmath>

#include <Arduino.h>
#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#endif

#include "modbus_request_detector.h"

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusRequestDetector";

ModbusRequestDetector::ModbusRequestDetector(IUartInterface* uart_interface) : 
  uart_interface_(uart_interface) {
  const uint32_t baud_rate = uart_interface->get_baud_rate();
  if (baud_rate <= 19200) {
    const float us_per_bit = 1000000.0f / baud_rate;
    constexpr uint8_t bits_per_byte = 11;
    const float us_per_byte = bits_per_byte * us_per_bit;
    this->max_time_between_bytes_in_us_ = static_cast<uint16_t>(round(1.5 * us_per_byte));
  } else {
   this->max_time_between_bytes_in_us_ = MIN_TIMEOUT_BETWEEN_BYTES_IN_US;
  }
}

ModbusFrame* ModbusRequestDetector::detect_request() {
	//  1. Read the first byte. Assume it is the address
	//  2. Read the second byte. Assume it is the function
	//  If function 1-6:
	//  	○ Read six more bytes
	//  	○ See if the last two bytes contain the correct CRC
	//  	○ If so, it is a request. If not, it is not a request.
	//  If function 15-16:
	//  	○ Read five more bytes
	//  	○ Last byte is the number of data bytes to follow.
	//  	○ Read this number of bytes, + 2 extra
	//  	○ See if the last two bytes contain the correct CRC
	//  	○ If so, it is a request. If not, it is not a request.
  
  uint32_t time_before_waiting_for_request = millis();
  while (this->uart_interface_->available() == 0) {
    delayMicroseconds(50);
    if (millis() - time_before_waiting_for_request >= MAX_TIME_TO_WAIT_FOR_REQUEST_IN_MS) {
      return nullptr;
    }
  }
  this->time_last_byte_received_ = micros();
  uint8_t address { 0 };
  if (!read_next_byte(&address)) {
    return nullptr;
  }
  uint8_t function { 0 };
  if (!read_next_byte(&function)) {
    return nullptr;
  }

  if ((function >= 1) && (function <= 6)) {
    uint8_t *raw_data = new uint8_t[6];
    raw_data[0] = address;
    raw_data[1] = function;
    for (uint8_t i { 2 }; i < 6; ++i) {
      if (!read_next_byte(&raw_data[i])) {
        delete[] raw_data;
        return nullptr;
      }
    }
    uint8_t crc_low_byte { 0x00 };
    if (!read_next_byte(&crc_low_byte)) {
      delete[] raw_data;
      return nullptr;
    }
    uint8_t crc_high_byte { 0x00 };
    if (!read_next_byte(&crc_high_byte)) {
      delete[] raw_data;
      return nullptr;
    }
    uint16_t calculated_crc = crc16(raw_data, 6);
    uint16_t received_crc = crc_low_byte | (crc_high_byte << 8);
    if (calculated_crc != received_crc) {
      delete[] raw_data;
      return nullptr;
    }
    // CRC is right! So this must be a request.
    uint8_t *data = new uint8_t[4];
    for (uint8_t i {0}; i < 4; ++i) {
      data[i] = raw_data[i + 2];
    }
    delete[] raw_data;
    ModbusFrame *request_frame = new ModbusFrame(address, function, data, 4);
    return request_frame;
  } else if ((function >= 15) && (function <= 16)) {
    uint8_t starting_address_and_byte_count_data[4];
    for (uint8_t i { 0 }; i < 4; ++i) {
      if (!read_next_byte(&starting_address_and_byte_count_data[i])) {
        return nullptr;
      }
    }
    uint8_t byte_count { 0 };
    if (!read_next_byte(&byte_count)) {
      return nullptr;
    }
    uint8_t *crc_data = new uint8_t[byte_count + 4 + 2 + 1];
    crc_data[0] = address;
    crc_data[1] = function;
    for (uint8_t i { 0 }; i < 4; ++i) {
      crc_data[i + 2] = starting_address_and_byte_count_data[i];
    }
    crc_data[6] = byte_count;
    for (uint8_t i { 0 }; i < byte_count; ++i) {
      if (!read_next_byte(&crc_data[i + 7])) {
        delete[] crc_data;
        return nullptr;
      }
    }
    uint8_t crc_low_byte { 0x00 };
    if (!read_next_byte(&crc_low_byte)) {
      delete[] crc_data;
      return nullptr;
    }
    uint8_t crc_high_byte { 0x00 };
    if (!read_next_byte(&crc_high_byte)) {
      delete[] crc_data;
      return nullptr;
    }
    uint16_t calculated_crc = crc16(crc_data, byte_count + 4 + 2 + 1);
    uint16_t received_crc = crc_low_byte | (crc_high_byte << 8);
    if (calculated_crc != received_crc) {
      delete[] crc_data;
      return nullptr;
    }
    // CRC is right! So this must be a request.
    uint8_t *data = new uint8_t[byte_count + 4 + 1];
    for (uint8_t i { 0 }; i < 4; ++i) {
      data[i] = starting_address_and_byte_count_data[i];
    }
    data[4] = byte_count;
    for (uint8_t i { 0 }; i < byte_count; ++i) {
      data[i + 5] = crc_data[i + 7];
    }
    delete[] crc_data;
    ModbusFrame *request_frame = new ModbusFrame(address, function, data, byte_count + 5);
    return request_frame;
  } else {
    // Unsupported function!
    return nullptr;
  }

  return nullptr;
}

bool ModbusRequestDetector::read_next_byte(uint8_t* byte) {
  if (this->uart_interface_->available() == 0) {
    // Next byte didn't arrive yet. Wait for it, with a timeout.
    bool waiting_too_long { false };
    do {
      waiting_too_long = (micros() - this->time_last_byte_received_) > this->max_time_between_bytes_in_us_;
      delayMicroseconds(100);
    } while ((this->uart_interface_->available() == 0) && !waiting_too_long);
    if (this->uart_interface_->available() == 0) {
      // Still nothing after waiting, so no byte in time...
      return false;
    }
  }
  bool is_byte_received = this->uart_interface_->read_byte(byte);
  if (is_byte_received) {
    this->time_last_byte_received_ = micros();
  }
  return is_byte_received;
}

} //namespace modbus_spy
} //namespace esphome
