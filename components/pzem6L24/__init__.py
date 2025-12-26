import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_ADDRESS, CONF_UPDATE_INTERVAL

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor']
MULTI_CONF = True

CODEOWNERS = ["@esphome/core"]

# Define the namespace matching the C++ code
ns = cg.global_ns.namespace('esphome').namespace('pzem6l24')

# Define the Hub Component within the namespace
PZEM6L24Component = ns.class_('PZEM6L24Component', uart.UARTDevice, cg.Component)

# Hub Configuration
CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(PZEM6L24Component),
    cv.Optional(CONF_ADDRESS, default=0xF8): cv.hex_uint8_t,
    cv.Optional(CONF_UPDATE_INTERVAL, default='10s'): cv.update_interval,
}).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Add the include directive manually to the generated main.cpp
    # We must use cg.RawStatement to add a raw string
    cg.add_global(cg.RawStatement('#include "esphome/components/pzem6l24/pzem6l24.h"'))
    
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))