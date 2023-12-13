import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import ModbusSpy, modbus_spy_ns

CODEOWNERS = ["@pdjong"]

DEPENDENCIES = ["modbus_spy"]

ModbusRegisterSensor = modbus_spy_ns.class_('ModbusRegisterSensor', sensor.Sensor)
CONF_MODBUS_SPY_ID = 'modbus_spy_id'
CONF_DEVICE_ADDRESS = 'device_address'
CONF_REGISTER_ADDRESS = 'register_address'

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.GenerateID(CONF_MODBUS_SPY_ID): cv.use_id(ModbusSpy),
    cv.Required(CONF_DEVICE_ADDRESS): int,
    cv.Required(CONF_REGISTER_ADDRESS): int
})

async def to_code(config):
    modbus_spy = await cg.get_variable(config[CONF_MODBUS_SPY_ID])
    modbus_register_sensor = modbus_spy.create_sensor(config[CONF_DEVICE_ADDRESS], config[CONF_REGISTER_ADDRESS])
    await sensor.register_sensor(modbus_register_sensor, config)