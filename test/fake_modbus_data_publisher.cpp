#include <vector>

#include "fake_modbus_data_publisher.h"
#include "modbus_data.h"
#include "modbus_data_publisher.h"

using std::vector;

FakeModbusDataPublisher::~FakeModbusDataPublisher() {
  // TODO
}

void FakeModbusDataPublisher::publish_data(uint8_t device_address, uint8_t function, vector<ModbusData*>* data) {
  for (ModbusData* data_item : *data) {
    PublishedData *published_data = new PublishedData;
    published_data->device_address = device_address;
    published_data->function = function;
    published_data->modbus_data = data_item;
    this->published_data_.push_back(published_data);
  }
}