import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import ModbusSpy, modbus_spy_ns

CODEOWNERS = ["@pdjong"]

DEPENDENCIES = ["modbus_spy"]

ModbusBinarySensor = modbus_spy_ns.class_('ModbusBinarySensor', binary_sensor.BinarySensor)
CONF_MODBUS_SPY_ID = 'modbus_spy_id'
CONF_DEVICE_ADDRESS = 'device_address'
CONF_REGISTER_ADDRESS = 'register_address'
CONF_RAW_REGISTER_ADDRESS = 'raw_register_address'
CONF_BIT = 'bit'

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema().extend({
        cv.GenerateID(CONF_MODBUS_SPY_ID): cv.use_id(ModbusSpy),
        cv.Required(CONF_DEVICE_ADDRESS): int,
        cv.Optional(CONF_REGISTER_ADDRESS): int,
        cv.Optional(CONF_RAW_REGISTER_ADDRESS): int,
        cv.Optional(CONF_BIT, default=-1): cv.int_range(-1, 15)
    }),
    cv.has_at_least_one_key(CONF_REGISTER_ADDRESS, CONF_RAW_REGISTER_ADDRESS)
)

async def to_code(config):
    modbus_spy = await cg.get_variable(config[CONF_MODBUS_SPY_ID])
    if CONF_REGISTER_ADDRESS in config:
        register_address = config[CONF_REGISTER_ADDRESS]
    else:
        # Assume holding register; convert raw address to data model address
        register_address = config[CONF_RAW_REGISTER_ADDRESS] + 40001
    modbus_binary_sensor = modbus_spy.create_binary_sensor(config[CONF_DEVICE_ADDRESS], register_address, config[CONF_BIT])
    await binary_sensor.register_binary_sensor(modbus_binary_sensor, config)