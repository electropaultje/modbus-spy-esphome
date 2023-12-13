#ifndef UNIT_TEST
#include "esphome/core/log.h"
#endif // UNIT_TEST

#include "modbus_data_publisher.h"

using std::vector;

namespace esphome {
namespace modbus_spy {

static const char *TAG = "ModbusDataPublisher";

IModbusDataPublisher::~IModbusDataPublisher() {
}

ModbusDataPublisher::ModbusDataPublisher(bool dump_not_configured_data) : 
  should_dump_not_configured_data_(dump_not_configured_data) {
}

ModbusDataPublisher::~ModbusDataPublisher() {
  // for (auto& pair : this->device_sensors_) {
  //   DeviceSensors *device_sensors = pair.second;
  //   if (device_sensors->binary_sensors_ != nullptr) {
  //     for (auto& sensor_register_pair : *device_sensors->binary_sensors_) {
  //       IModbusBinarySensor *binary_sensor = sensor_register_pair.second;
  //       delete binary_sensor;
  //     }
  //     delete device_sensors->binary_sensors_;
  //   }
  //   if (device_sensors->register_sensors_ != nullptr) {
  //     for (auto& sensor_register_pair : *device_sensors->register_sensors_) {
  //       IModbusRegisterSensor *register_sensor = sensor_register_pair.second;
  //       delete register_sensor;
  //     }
  //     delete device_sensors->register_sensors_;
  //   }
  //   delete device_sensors;
  // }
}

void ModbusDataPublisher::add_register_sensor(
  uint8_t device_address, 
  uint16_t register_address,
  IModbusRegisterSensor* register_sensor
) {
  std::map<uint16_t, IModbusRegisterSensor*> *register_sensors_for_device = get_register_sensors_for_device(device_address);
  register_sensors_for_device->insert({ register_address, register_sensor });
}

void ModbusDataPublisher::add_binary_sensor(
  uint8_t device_address, 
  uint16_t register_address,
  int8_t bit,
  IModbusBinarySensor* binary_sensor
) {
  if (-1 == bit) {
    std::map<uint16_t, IModbusBinarySensor*> *binary_sensors_full_register_for_device = get_binary_sensors_full_register_for_device(device_address);
    (*binary_sensors_full_register_for_device)[register_address] = binary_sensor;
  } else {
    std::map<uint16_t, IModbusBinarySensor**> *binary_sensors_bit_for_device = get_binary_sensors_bit_for_device(device_address);
    // See if there's a list of bit sensors for the register. If not, create it.
    IModbusBinarySensor **bit_sensors_for_register = (*binary_sensors_bit_for_device)[register_address];
    if (nullptr == bit_sensors_for_register) {
      bit_sensors_for_register = new IModbusBinarySensor*[16];
      for (uint8_t i = 0; i < 16; ++i) {
        bit_sensors_for_register[i] = nullptr;
      }
      (*binary_sensors_bit_for_device)[register_address] = bit_sensors_for_register;
    }
    bit_sensors_for_register[bit] = binary_sensor;
  }
}

std::map<uint16_t, IModbusRegisterSensor*>* ModbusDataPublisher::get_register_sensors_for_device(uint8_t device_address) {
  DeviceSensors *sensors_for_device = get_sensors_for_device(device_address);
  std::map<uint16_t, IModbusRegisterSensor*>* register_sensors_for_device = sensors_for_device->register_sensors_;
  if (nullptr == register_sensors_for_device) {
    register_sensors_for_device = new std::map<uint16_t, IModbusRegisterSensor*>;
    sensors_for_device->register_sensors_ = register_sensors_for_device;
  }
  return register_sensors_for_device;
}

std::map<uint16_t, IModbusBinarySensor*>* ModbusDataPublisher::get_binary_sensors_full_register_for_device(uint8_t device_address) {
  DeviceSensors *sensors_for_device = get_sensors_for_device(device_address);
  std::map<uint16_t, IModbusBinarySensor*>* binary_sensors_full_register_for_device = sensors_for_device->binary_sensors_full_register_;
  if (nullptr == binary_sensors_full_register_for_device) {
    binary_sensors_full_register_for_device = new std::map<uint16_t, IModbusBinarySensor*>;
    sensors_for_device->binary_sensors_full_register_ = binary_sensors_full_register_for_device;
  }
  return binary_sensors_full_register_for_device;
}

std::map<uint16_t, IModbusBinarySensor**>* ModbusDataPublisher::get_binary_sensors_bit_for_device(uint8_t device_address) {
  DeviceSensors *sensors_for_device = get_sensors_for_device(device_address);
  std::map<uint16_t, IModbusBinarySensor**>* binary_sensors_bit_for_device = sensors_for_device->binary_sensors_bit_;
  if (nullptr == binary_sensors_bit_for_device) {
    binary_sensors_bit_for_device = new std::map<uint16_t, IModbusBinarySensor**>;
    sensors_for_device->binary_sensors_bit_ = binary_sensors_bit_for_device;
  }
  return binary_sensors_bit_for_device;
}

ModbusDataPublisher::DeviceSensors* ModbusDataPublisher::get_sensors_for_device(uint8_t device_address) {
  DeviceSensors *sensors_for_device = this->device_sensors_[device_address];
  if (nullptr == sensors_for_device) {
    sensors_for_device = new DeviceSensors;
    this->device_sensors_[device_address] = sensors_for_device;
  }
  return sensors_for_device;
}

void ModbusDataPublisher::publish_data(uint8_t device_address, uint8_t function, std::vector<ModbusData*>* data) {
  for (ModbusData* modbus_data : *data) {
    uint16_t data_model_register_address = convert_pdu_address_to_data_model_address(function, modbus_data->address);
    find_sensor_and_publish_data(device_address, data_model_register_address, modbus_data->value);
  }
}

uint16_t ModbusDataPublisher::convert_pdu_address_to_data_model_address(uint8_t function, uint16_t pdu_address) {
  // Convert the register address in ModbusData to an actual register address, based on the function.
  // Function 3: add 40001
  uint16_t modbus_data_model_address { 0 };
  switch (function) {
    case 3: {
      modbus_data_model_address = pdu_address + 40001;
      break;
    }
    default:
      break;
  }

  return modbus_data_model_address;
}

void ModbusDataPublisher::find_sensor_and_publish_data(uint8_t device_address, uint16_t data_model_register_address, uint16_t value) {
  ESP_LOGD(TAG, "Finding sensor for register address %d, to publish value %d", data_model_register_address, value);
  bool found_a_sensor { false };
  IModbusRegisterSensor *register_sensor = find_register_sensor(device_address, data_model_register_address);
  if (register_sensor != nullptr) {
    ESP_LOGV(TAG, "Found sensor!");
    found_a_sensor = true;
    register_sensor->publish_state(value);
  }

  IModbusBinarySensor *binary_sensor = find_binary_sensor_full_register(device_address, data_model_register_address);
  if (binary_sensor != nullptr) {
    ESP_LOGV(TAG, "Found binary sensor! For register %d", data_model_register_address);
    found_a_sensor = true;
    binary_sensor->publish_state(static_cast<bool>(value));
  }

  IModbusBinarySensor **bit_sensors = find_binary_bit_sensors(device_address, data_model_register_address);
  if (bit_sensors != nullptr) {
    ESP_LOGV(TAG, "Found bit sensors! For register %d", data_model_register_address);
    found_a_sensor = true;
    for (uint8_t bit = 0; bit < 16; ++bit) {
      IModbusBinarySensor *bit_sensor = bit_sensors[bit];
      if (bit_sensor != nullptr) {
        bool bit_value = (value & (1 << bit)) >> bit;
        bit_sensor->publish_state(bit_value);
      }
    }
  }

  if (!found_a_sensor && this->should_dump_not_configured_data_) {
    ESP_LOGI(TAG, "No sensor for: device 0x%02X, data model address %d, value %d", device_address, data_model_register_address, value);
  }
}

IModbusRegisterSensor* ModbusDataPublisher::find_register_sensor(uint8_t device_address, uint16_t data_model_register_address) {
  DeviceSensors *device_sensors = this->device_sensors_[device_address];
  if (nullptr == device_sensors) {
    return nullptr;
  }
  if (nullptr == device_sensors->register_sensors_) {
    return nullptr;
  }
  std::map<uint16_t, IModbusRegisterSensor*>& register_sensors_for_device = *device_sensors->register_sensors_;
  return register_sensors_for_device[data_model_register_address];
}

IModbusBinarySensor* ModbusDataPublisher::find_binary_sensor_full_register(uint8_t device_address, uint16_t data_model_register_address) {
  DeviceSensors *device_sensors = this->device_sensors_[device_address];
  if (nullptr == device_sensors) {
    return nullptr;
  }
  if (nullptr == device_sensors->binary_sensors_full_register_) {
    return nullptr;
  }
  std::map<uint16_t, IModbusBinarySensor*>& binary_sensors_full_register_for_device = *device_sensors->binary_sensors_full_register_;
  return binary_sensors_full_register_for_device[data_model_register_address];
}

IModbusBinarySensor** ModbusDataPublisher::find_binary_bit_sensors(uint8_t device_address, uint16_t data_model_register_address) {
  DeviceSensors *device_sensors = this->device_sensors_[device_address];
  if (nullptr == device_sensors) {
    return nullptr;
  }
  if (nullptr == device_sensors->binary_sensors_bit_) {
    return nullptr;
  }
  return (*device_sensors->binary_sensors_bit_)[data_model_register_address];
}

} //namespace modbus_spy
} //namespace esphome
