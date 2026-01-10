#include <cmath>

#ifndef UNIT_TEST
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#endif
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include <rom/ets_sys.h>

#include "modbus_response_detector.h"

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusResponseDetector";

ModbusResponseDetector::ModbusResponseDetector(IUartInterface* uart_interface) :
  uart_interface_(uart_interface) {
  const uint32_t baud_rate = uart_interface->get_baud_rate();
  if (baud_rate <= 19200) {
    const float us_per_bit = 1000000.0f / baud_rate;
    constexpr uint8_t bits_per_byte = 11;
    const float us_per_byte = bits_per_byte * us_per_bit;
    this->max_time_between_bytes_in_us_ = static_cast<uint16_t>(round(1.5 * us_per_byte));
    ESP_LOGI(TAG, "Baud rate (<= 19200): %u, Max time between bytes set to %u us", baud_rate, this->max_time_between_bytes_in_us_);
  } else {
    this->max_time_between_bytes_in_us_ = MIN_TIMEOUT_BETWEEN_BYTES_IN_US;
    ESP_LOGI(TAG, "Baud rate (> 19200): %u, Max time between bytes set to %u us", baud_rate, this->max_time_between_bytes_in_us_);
  }
}

ModbusFrame* ModbusResponseDetector::detect_response() {
	//  1. Read the first byte. Assume it is the address
	//  2. Read the second byte. Assume it is the function
	//  If function is 1, 2, 3 or 4:
  //  	○ Read the 3rd byte. Assume it is the number of data bytes
  //  	○ Read the data bytes
  //  	○ Read two more bytes. Assume it is the CRC
	//    ○ See if the last two bytes contain the correct CRC
	//  	○ If so, it is a response. If not, it is not a response.
	//  If function is 5, 6, 15 or 16:
  //  	○ Read five more bytes. Assum it is the data bytes
  //  	○ Read two more bytes. Assume it is the CRC
	//    ○ See if the last two bytes contain the correct CRC
	//  	○ If so, it is a response. If not, it is not a response.
  //  If other function:
  //  	○ Return nothing. Unsupported for now.
  uint32_t time_before_waiting_for_response = pdTICKS_TO_MS(xTaskGetTickCount());
  while (this->uart_interface_->available() == 0) {
    esp_rom_delay_us(50);
    if (pdTICKS_TO_MS(xTaskGetTickCount()) - time_before_waiting_for_response >= MAX_TIME_BETWEEN_REQUEST_AND_RESPONSE_IN_MS) {
      ESP_LOGI(TAG, "Timeout waiting for response");
      return nullptr;
    }
  }
  this->time_last_byte_received_ = esp_timer_get_time();
  uint8_t address { 0 };
  if (!read_next_byte(&address)) {
    ESP_LOGI(TAG, "Failed to read address");
    return nullptr;
  }
  uint8_t function { 0 };
  if (!read_next_byte(&function)) {
    ESP_LOGI(TAG, "Failed to read function");
    return nullptr;
  }
  if ((function >= 1) && (function <= 4)) {
    uint8_t byte_count { 0 };
    if (!read_next_byte(&byte_count)) {
      ESP_LOGI(TAG, "Failed to read byte count");
      return nullptr;
    }
    uint8_t *crc_data = new uint8_t[byte_count + 3];
    crc_data[0] = address;
    crc_data[1] = function;
    crc_data[2] = byte_count;
    for (uint8_t i { 0 }; i < byte_count; ++i) {
      if (!read_next_byte(&crc_data[i + 3])) {
        ESP_LOGI(TAG, "Failed to read data byte %u", i);
        delete[] crc_data;
        return nullptr;
      }
    }
    uint8_t crc_low_byte { 0x00 };
    if (!read_next_byte(&crc_low_byte)) {
      ESP_LOGI(TAG, "Failed to read CRC low byte");
      delete[] crc_data;
      return nullptr;
    }
    uint8_t crc_high_byte { 0x00 };
    if (!read_next_byte(&crc_high_byte)) {
      ESP_LOGI(TAG, "Failed to read CRC high byte");
      delete[] crc_data;
      return nullptr;
    }
    uint16_t calculated_crc = crc16(crc_data, byte_count + 3);
    uint16_t received_crc = crc_low_byte | (crc_high_byte << 8);
    if (calculated_crc != received_crc) {
      ESP_LOGI(TAG, "CRC mismatch: calculated 0x%04X, received 0x%04X", calculated_crc, received_crc);
      delete[] crc_data;
      return nullptr;
    }
    // CRC is right! So this must be a response.
    uint8_t *data = new uint8_t[byte_count + 1];
    data[0] = byte_count;
    for (uint8_t i {0}; i < byte_count; ++i) {
      data[i + 1] = crc_data[i + 3];
    }
    delete[] crc_data;
    ModbusFrame *response_frame = new ModbusFrame(address, function, data, byte_count + 1);
    return response_frame;
  } else if ((function == 5) || (function == 6) || (function == 15) || (function == 16)) {
    ESP_LOGI(TAG, "Function %u detected, reading response", function);
    uint8_t *crc_data = new uint8_t[6];
    crc_data[0] = address;
    crc_data[1] = function;
    for (uint8_t i { 0 }; i < 4; ++i) {
      if (!read_next_byte(&crc_data[i + 2])) {
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
    uint16_t calculated_crc = crc16(crc_data, 6);
    uint16_t received_crc = crc_low_byte | (crc_high_byte << 8);
    if (calculated_crc != received_crc) {
      delete[] crc_data;
      return nullptr;
    }
    // CRC is right! So this must be a response.
    uint8_t *data = new uint8_t[4];
    for (uint8_t i { 0 }; i < 4; ++i) {
      data[i] = crc_data[i + 2];
    }
    delete[] crc_data;
    ModbusFrame *response_frame = new ModbusFrame(address, function, data, 4);
    return response_frame;
  } else {
    // Unsupported function!
    ESP_LOGI(TAG, "Unsupported function %u", function);
    return nullptr;
  }

  return nullptr;
}

bool ModbusResponseDetector::read_next_byte(uint8_t* byte) {
  if (this->uart_interface_->available() == 0) {
    // Next byte didn't arrive yet. Wait for it, with a timeout.
    bool waiting_too_long { false };
    do {
      esp_rom_delay_us(100);
      waiting_too_long = (esp_timer_get_time() - this->time_last_byte_received_) > this->max_time_between_bytes_in_us_;
    } while ((this->uart_interface_->available() == 0) && !waiting_too_long);
    if (this->uart_interface_->available() == 0) {
      // Still nothing after waiting, so no byte in time...
      return false;
    }
  }
  bool is_byte_received = this->uart_interface_->read_byte(byte);
  if (is_byte_received) {
    this->time_last_byte_received_ = esp_timer_get_time();
  }
  return is_byte_received;
}

} //namespace modbus_spy
} //namespace esphome
