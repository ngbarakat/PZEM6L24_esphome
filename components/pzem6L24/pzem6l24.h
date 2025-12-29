#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include <vector>

namespace esphome {
    namespace pzem6l24 {

        // Register Definitions
#define PZEM_VOLTAGE_REG                  0x0000
#define PZEM_CURRENT_REG                  0x0003
#define PZEM_FREQUENCY_REG                0x0006
#define PZEM_VOLTAGE_PHASE_REG            0x0009
#define PZEM_CURRENT_PHASE_REG            0x000B
#define PZEM_ACTIVE_POWER_REG             0x000E
#define PZEM_REACTIVE_POWER_REG           0x0014
#define PZEM_APPARENT_POWER_REG           0x001A
#define PZEM_ACTIVE_POWER_COMBINED_REG    0x0020
#define PZEM_REACTIVE_POWER_COMBINED_REG  0x0022
#define PZEM_APPARENT_POWER_COMBINED_REG  0x0024
#define PZEM_POWER_FACTOR_A_B_REG         0x0026
#define PZEM_POWER_FACTOR_C_COMBINED_REG  0x0027
#define PZEM_ACTIVE_ENERGY_REG            0x0028
#define PZEM_REACTIVE_ENERGY_REG          0x002E
#define PZEM_APPARENT_ENERGY_REG          0x0034
#define PZEM_ACTIVE_ENERGY_COMBINED_REG   0x003A
#define PZEM_REACTIVE_ENERGY_COMBINED_REG 0x003C
#define PZEM_APPARENT_ENERGY_COMBINED_REG 0x003E

// Resolutions
#define PZEM_VOLTAGE_RESOLUTION      0.01f
#define PZEM_CURRENT_RESOLUTION      0.01f
#define PZEM_FREQUENCY_RESOLUTION    0.01f
#define PZEM_POWER_RESOLUTION        0.1f
#define PZEM_POWER_FACTOR_RESOLUTION 0.01f
#define PZEM_ENERGY_RESOLUTION       0.1f
#define PZEM_PHASE_RESOLUTION        0.01f

// Enums
        enum PZEMType {
            PZEM_TYPE_VOLTAGE = 0,
            PZEM_TYPE_CURRENT,
            PZEM_TYPE_ACTIVE_POWER,
            PZEM_TYPE_ACTIVE_ENERGY,
            PZEM_TYPE_REACTIVE_POWER,
            PZEM_TYPE_REACTIVE_ENERGY,
            PZEM_TYPE_APPARENT_POWER,
            PZEM_TYPE_APPARENT_ENERGY,
            PZEM_TYPE_POWER_FACTOR,
            PZEM_TYPE_FREQUENCY,
            PZEM_TYPE_VOLTAGE_PHASE_ANGLE,
            PZEM_TYPE_CURRENT_PHASE_ANGLE,
        };

        enum PZEMPhase {
            PHASE_A = 0,
            PHASE_B = 1,
            PHASE_C = 2,
            PHASE_COMBINED = 3
        };

        // Sensor Child Class
        class PZEMSensor : public sensor::Sensor, public Component {
        public:
            void set_type(PZEMType type) { type_ = type; }
            void set_phase(PZEMPhase phase) { phase_ = phase; }
            PZEMType get_type() const { return type_; }
            PZEMPhase get_phase() const { return phase_; }

        protected:
            PZEMType type_;
            PZEMPhase phase_;
        };

        // Hub Component Class
        class PZEM6L24Component : public uart::UARTDevice, public Component {
        public:
            PZEM6L24Component() = default;

            void setup() override;
            void loop() override;
            void dump_config() override;
            float get_setup_priority() const override { return setup_priority::DATA; }

            void set_address(uint8_t address) { this->_slaveAddr = address; }
            void set_update_interval(uint32_t interval) { this->update_interval_ = interval; }

            void register_sensor(PZEMSensor* obj) { this->sensors_.push_back(obj); }

        protected:
            uint8_t _slaveAddr{ 0xF8 };
            uint32_t update_interval_{ 10000 };
            uint32_t last_update_{ 0 };
            std::vector<PZEMSensor*> sensors_;

            // Local storage for all values
            float vol_[3] = { NAN, NAN, NAN };
            float cur_[3] = { NAN, NAN, NAN };
            float freq_[3] = { NAN, NAN, NAN };
            float act_pwr_[3] = { NAN, NAN, NAN };
            float react_pwr_[3] = { NAN, NAN, NAN };
            float app_pwr_[3] = { NAN, NAN, NAN };
            float pf_[3] = { NAN, NAN, NAN };
            float act_nrg_[3] = { NAN, NAN, NAN };
            float react_nrg_[3] = { NAN, NAN, NAN };
            float app_nrg_[3] = { NAN, NAN, NAN };
            float vol_ang_[3] = { NAN, NAN, NAN };
            float cur_ang_[3] = { NAN, NAN, NAN };

            // Combined values
            float total_act_pwr_ = NAN;
            float total_react_pwr_ = NAN;
            float total_app_pwr_ = NAN;
            float total_pf_ = NAN;
            float total_act_nrg_ = NAN;
            float total_react_nrg_ = NAN;
            float total_app_nrg_ = NAN;

            // Modbus Helpers
            bool read_input_registers(uint8_t slaveAddr, uint16_t reg, uint8_t count, uint16_t* data);
            void refresh_values();
            void clearBuffer() { while (this->available()) this->read(); }

            int32_t combine_registers(uint16_t high, uint16_t low, bool is_signed) {
                if (is_signed) return (int32_t)((int16_t)high << 16 | (uint16_t)low);
                return (uint32_t)high << 16 | low;
            }
        };

    }  // namespace pzem6l24

}  // namespace esphome
