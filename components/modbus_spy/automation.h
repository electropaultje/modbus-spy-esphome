#ifndef MODBUS_SPY_AUTOMATION_H_
#define MODBUS_SPY_AUTOMATION_H_

#include "esphome/core/automation.h"
#include "esphome/core/datatypes.h"
#include "modbus_spy.h"

namespace esphome {
namespace modbus_spy {

template<typename... Ts> class SetLogUnconfiguredItemsAction : public Action<Ts...> {
 public:
  explicit SetLogUnconfiguredItemsAction(ModbusSpy* modbus_spy) : modbus_spy_(modbus_spy) {}
  TEMPLATABLE_VALUE(bool, log_unconfigured_items)

  void play(const Ts&... x) override {
    bool log_unconfigured_items_value = this->log_unconfigured_items_.value(x...);
    this->modbus_spy_->set_log_unconfigured_items(log_unconfigured_items_value);
  }

 protected:
  ModbusSpy* modbus_spy_;
};

} //namespace modbus_spy
} //namespace esphome

#endif // MODBUS_SPY_AUTOMATION_H_