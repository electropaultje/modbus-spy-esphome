import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

CODEOWNERS = ["@pdjong"]

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor"]
MULTI_CONF = True

modbus_spy_ns = cg.esphome_ns.namespace('modbus_spy')

ModbusSpy = modbus_spy_ns.class_('ModbusSpy', cg.Component, uart.UARTDevice)
CONF_LOG_NOT_CONFIGURED_DATA = 'log_not_configured_data'

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(): cv.declare_id(ModbusSpy),
  cv.Optional(CONF_LOG_NOT_CONFIGURED_DATA, default=False): cv.boolean
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
  rhs = ModbusSpy.new(config[CONF_LOG_NOT_CONFIGURED_DATA])
  var = cg.Pvariable(config[CONF_ID], rhs)
  await cg.register_component(var, config)
  await uart.register_uart_device(var, config)