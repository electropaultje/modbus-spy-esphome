import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import ModbusSpy, modbus_spy_ns
import logging

CODEOWNERS = ["@pdjong"]

DEPENDENCIES = ["modbus_spy"]

_LOGGER = logging.getLogger(__name__)
ModbusBinarySensor = modbus_spy_ns.class_('ModbusBinarySensor', binary_sensor.BinarySensor)
CONF_MODBUS_SPY_ID = 'modbus_spy_id'
CONF_DEVICE_ADDRESS = 'device_address'
CONF_REGISTER_ADDRESS = 'register_address'
CONF_RAW_REGISTER_ADDRESS = 'raw_register_address'
CONF_REGISTER_TYPE = 'register_type'
CONF_BIT = 'bit'

def _validate_register_type(config):
    if CONF_REGISTER_TYPE in config and CONF_RAW_REGISTER_ADDRESS not in config:
        raise cv.Invalid(f"'{CONF_REGISTER_TYPE}' can only be used with '{CONF_RAW_REGISTER_ADDRESS}'")
    if CONF_RAW_REGISTER_ADDRESS in config and CONF_REGISTER_TYPE not in config:
        _LOGGER.warning(f"'{CONF_RAW_REGISTER_ADDRESS}' requires '{CONF_REGISTER_TYPE}' to be set (either 'holding' or 'input'). Assuming 'holding', for 40001 offset.")
    return config

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema().extend({
        cv.GenerateID(CONF_MODBUS_SPY_ID): cv.use_id(ModbusSpy),
        cv.Required(CONF_DEVICE_ADDRESS): int,
        cv.Optional(CONF_REGISTER_ADDRESS): int,
        cv.Optional(CONF_RAW_REGISTER_ADDRESS): int,
        cv.Optional(CONF_REGISTER_TYPE): cv.one_of('holding', 'input', lower=True),
        cv.Optional(CONF_BIT, default=-1): cv.int_range(-1, 15),
    }),
    cv.has_at_most_one_key(CONF_REGISTER_ADDRESS, CONF_RAW_REGISTER_ADDRESS),
    _validate_register_type,
)

async def to_code(config):
    modbus_spy = await cg.get_variable(config[CONF_MODBUS_SPY_ID])
    if CONF_REGISTER_ADDRESS in config:
        register_address = config[CONF_REGISTER_ADDRESS]
    else:
        offset = 40001 if config.get(CONF_REGISTER_TYPE, 'holding') == 'holding' else 30001
        register_address = config[CONF_RAW_REGISTER_ADDRESS] + offset
    modbus_binary_sensor = modbus_spy.create_binary_sensor(config[CONF_DEVICE_ADDRESS], register_address, config[CONF_BIT])
    await binary_sensor.register_binary_sensor(modbus_binary_sensor, config)