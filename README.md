# PZEM6L24_esphome


Here is a `README.md` file suitable for your GitHub repository.

```markdown
# ESPHome PZEM-6L24 Energy Monitor Component

This is a custom external component for ESPHome to interface with the **PZEM-6L24** three-phase energy monitoring module.

This component allows ESPHome to read detailed electrical metrics including voltage, current, active/reactive/apparent power, energy, power factor, and phase angles for each individual phase (A, B, C) as well as combined totals.

## Features

- **Individual Phase Reading**: Read data for Phase A, B, and C separately.
- **Combined Totals**: Read total system Power and Energy.
- **Bulk Data Handling**: Efficiently reads multiple registers over Modbus to minimize bus traffic.
- **Average Voltage**: Calculates the average system voltage for the combined phase sensor.
- **ESPHome Native**: No `lambda` coding required; sensors are automatically updated based on the hub's update interval.

## Credits & Attribution

This code is based on the original Arduino library work by [Lucas Hudson](https://github.com/lucashudson-eng/PZEMPlus).

The port to the ESPHome external component architecture and the generated C++ code were significantly assisted by **GLM-4.7**.

## ⚠️ Disclaimer

Please note that **very limited support can be provided** for this custom component. It is provided "as-is" and may require debugging or modification for specific use cases.

## Installation

1.  In your ESPHome configuration directory, create the folder structure:
    ```bash
    custom_components/
    └── pzem6l24/
        ├── __init__.py
        ├── pzem6l24.h
        ├── pzem6l24.cpp
        └── sensor/
            └── __init__.py
    ```
2.  Copy the source files from this repository into the corresponding folders.
3.  Reference the component in your YAML file using `external_components`.

## Wiring

| ESP32 Pin | PZEM-6L24 Pin |
|-----------|---------------|
| RX (e.g., GPIO16) | TX |
| TX (e.g., GPIO17) | RX |
| GND | GND |
| VCC (5V/3.3V) | VCC |

*Note: The PZEM-6L24 is 5V tolerant, but logic levels should be matched if your MCU is strictly 3.3V logic. If issues occur, use a logic level shifter.*

## Example Configuration

```yaml
esphome:
  name: pzem6l24_monitor
  platform: ESP32
  board: esp32dev

# Enable logging
logger:
  level: DEBUG

# UART Configuration
uart:
  id: uart_bus
  tx_pin: GPIO16
  rx_pin: GPIO17
  baud_rate: 9600

# Load Custom Component
external_components:
  - source:
      type: local
      path: custom_components

# Hub Component
pzem6l24:
  id: pzem_hub
  address: 0xF8  # Default PZEM address
  update_interval: 5s

# Sensors
sensor:
  - platform: pzem6l24
    pzem6l24_id: pzem_hub
    type: VOLTAGE
    phase: A
    name: "Phase A Voltage"
    unit_of_measurement: V
    accuracy_decimals: 1

  - platform: pzem6l24
    pzem6l24_id: pzem_hub
    type: VOLTAGE
    phase: B
    name: "Phase B Voltage"
    unit_of_measurement: V
    accuracy_decimals: 1

  - platform: pzem6l24
    pzem6l24_id: pzem_hub
    type: CURRENT
    phase: A
    name: "Phase A Current"
    unit_of_measurement: A
    accuracy_decimals: 2

  - platform: pzem6l24
    pzem6l24_id: pzem_hub
    type: ACTIVE_POWER
    phase: COMBINED
    name: "Total Power"
    unit_of_measurement: W
    accuracy_decimals: 1

  - platform: pzem6l24
    pzem6l24_id: pzem_hub
    type: ACTIVE_ENERGY
    phase: COMBINED
    name: "Total Energy"
    unit_of_measurement: kWh
    accuracy_decimals: 3
    filters:
      - multiply: 0.001 # Convert Wh to kWh
```

## Sensor Types & Phases

You can configure sensors for the following types:
- `VOLTAGE`
- `CURRENT`
- `FREQUENCY`
- `ACTIVE_POWER`
- `REACTIVE_POWER`
- `APPARENT_POWER`
- `ACTIVE_ENERGY`
- `REACTIVE_ENERGY`
- `APPARENT_ENERGY`
- `POWER_FACTOR`
- `VOLTAGE_PHASE_ANGLE`
- `CURRENT_PHASE_ANGLE`

Supported phases:
- `A`
- `B`
- `C`
- `COMBINED` (Returns hardware sum for Power/Energy, and calculated average for Voltage)

## License

This project is provided as-is. Please refer to the original [PZEMPlus repository](https://github.com/lucashudson-eng/PZEMPlus) for specific licensing details regarding the underlying logic.
```
