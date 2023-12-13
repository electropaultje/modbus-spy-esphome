#ifndef FAKE_UART_INTERFACE_H_
#define FAKE_UART_INTERFACE_H_

#include <queue>

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <test_includes.h>
#include <uart_interface.h>

using std::queue;

class FakeUartInterface : public esphome::modbus_spy::IUartInterface {
 public:
  virtual bool read_byte(uint8_t* data) {
    if (0 == this->fake_data_to_return_.size()) {
      return false;
    } else {
      *data = this->fake_data_to_return_.front();
      this->fake_data_to_return_.pop();
      return true;
    }
  }

  virtual bool read_array(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
      if (this->fake_data_to_return_.empty()) {
        break;
      } else {
        *data++ = this->fake_data_to_return_.front();
        this->fake_data_to_return_.pop();
      }
    }
    return true;
  }

  virtual int available() const {
    return this->fake_data_to_return_.size();
  }

  // Access / configure methods
  void set_fake_data_to_return(const uint8_t* fake_data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
      this->fake_data_to_return_.push(fake_data[i]);
    }
  }

  virtual uint32_t get_baud_rate() const override {
    return this->baud_rate_;
  }

  void set_baud_rate(uint32_t baud_rate) {
    this->baud_rate_ = baud_rate;
  }

 protected:
  queue<uint8_t> fake_data_to_return_;
  uint32_t baud_rate_ { 19200 };
};

typedef struct FakeUartInterfaceTaskArgs {
  FakeUartInterface* uart_interface;
  const uint16_t initial_delay_in_ms;
  const uint16_t delay_between_bytes_in_us;
  const uint8_t* data_to_return;
  const size_t len_of_data_to_return;
  bool* should_stop;
} FakeUartInterfaceTaskArgs;

void fake_uart_interface_task(void* param) {
  FakeUartInterfaceTaskArgs *args = reinterpret_cast<FakeUartInterfaceTaskArgs*>(param);
  FakeUartInterface *uart_interface = args->uart_interface;
  if (args->initial_delay_in_ms > 0) {
    delay(args->initial_delay_in_ms);
  }
  if (*args->should_stop) {
    vTaskDelete(NULL);
    return;
  }
  size_t next_byte_index { 0 };
  while (next_byte_index < args->len_of_data_to_return) {
    uint16_t delay_remaining_for_next_byte = args->delay_between_bytes_in_us;
    const uint16_t MAX_DELAY_PER_LOOP_IN_US = 500;
    while (delay_remaining_for_next_byte > 0) {
      uint16_t actual_delay_this_loop = delay_remaining_for_next_byte;
      if (delay_remaining_for_next_byte > MAX_DELAY_PER_LOOP_IN_US) {
        actual_delay_this_loop = MAX_DELAY_PER_LOOP_IN_US;
      }
      delayMicroseconds(actual_delay_this_loop);
      if (delay_remaining_for_next_byte > actual_delay_this_loop) {
        delay_remaining_for_next_byte -= actual_delay_this_loop;
      } else {
        delay_remaining_for_next_byte = 0;
      }
      if (*args->should_stop) {
        vTaskDelete(NULL);
        break;
      }
    }
    if (*args->should_stop) {
      vTaskDelete(NULL);
      break;
    }
    uart_interface->set_fake_data_to_return(&(args->data_to_return[next_byte_index]), 1);
    ++next_byte_index;
  }
  vTaskDelete(NULL);
}

#endif // FAKE_UART_INTERFACE_H_