import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

DEPENDENCIES = ['pzem6l24']

# Define the namespace
ns = cg.global_ns.namespace('esphome').namespace('pzem6l24')

# Enums
PZEMType = ns.enum('PZEMType')
PZEM_TYPES = {
    'VOLTAGE': PZEMType.PZEM_TYPE_VOLTAGE,
    'CURRENT': PZEMType.PZEM_TYPE_CURRENT,
    'ACTIVE_POWER': PZEMType.PZEM_TYPE_ACTIVE_POWER,
    'ACTIVE_ENERGY': PZEMType.PZEM_TYPE_ACTIVE_ENERGY,
    'REACTIVE_POWER': PZEMType.PZEM_TYPE_REACTIVE_POWER,
    'REACTIVE_ENERGY': PZEMType.PZEM_TYPE_REACTIVE_ENERGY,
    'APPARENT_POWER': PZEMType.PZEM_TYPE_APPARENT_POWER,
    'APPARENT_ENERGY': PZEMType.PZEM_TYPE_APPARENT_ENERGY,
    'POWER_FACTOR': PZEMType.PZEM_TYPE_POWER_FACTOR,
    'FREQUENCY': PZEMType.PZEM_TYPE_FREQUENCY,
    'VOLTAGE_PHASE_ANGLE': PZEMType.PZEM_TYPE_VOLTAGE_PHASE_ANGLE,
    'CURRENT_PHASE_ANGLE': PZEMType.PZEM_TYPE_CURRENT_PHASE_ANGLE,
}

PZEMPhase = ns.enum('PZEMPhase')
PZEM_PHASES = {
    'A': PZEMPhase.PHASE_A,
    'B': PZEMPhase.PHASE_B,
    'C': PZEMPhase.PHASE_C,
    'COMBINED': PZEMPhase.PHASE_COMBINED,
}

# Define the Sensor class within the namespace
PZEMSensor = ns.class_('PZEMSensor', sensor.Sensor, cg.Component)

CONF_PHASE = "phase"
CONF_TYPE = "type"
CONF_PZEM6L24_ID = "pzem6l24_id"

CONFIG_SCHEMA = (
    sensor.sensor_schema(PZEMSensor)
    .extend({
        cv.Required(CONF_TYPE): cv.enum(PZEM_TYPES, upper=True),
        cv.Optional(CONF_PHASE, default='COMBINED'): cv.enum(PZEM_PHASES, upper=True),
        cv.Required(CONF_PZEM6L24_ID): cv.use_id("pzem6l24"),
    })
)

async def to_code(config):
    # Ensure the header is included so the compiler knows the type definition
    cg.add_global(cg.RawStatement('#include "esphome/components/pzem6l24/pzem6l24.h"'))
    
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    
    cg.add(var.set_type(config[CONF_TYPE]))
    cg.add(var.set_phase(config[CONF_PHASE]))
    
    # Register sensor with parent hub
    parent = await cg.get_variable(config[CONF_PZEM6L24_ID])
    cg.add(parent.register_sensor(var))