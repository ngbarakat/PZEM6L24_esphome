#include "pzem6l24.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace pzem6l24 {

static const char *TAG = "pzem6l24";

// CRC Calculation
uint16_t calc_crc(const uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001; else crc >>= 1;
    }
  }
  return crc;
}

bool PZEM6L24Component::read_input_registers(uint8_t slaveAddr, uint16_t reg, uint8_t count, uint16_t *data) {
  uint8_t frame[8];
  frame[0] = slaveAddr; frame[1] = 0x04;
  frame[2] = (reg >> 8) & 0xFF; frame[3] = reg & 0xFF;
  frame[4] = (count >> 8) & 0xFF; frame[5] = count & 0xFF;
  uint16_t crc = calc_crc(frame, 6);
  frame[6] = crc & 0xFF; frame[7] = (crc >> 8) & 0xFF;

  this->flush();
  this->write_array(frame, 8);
  this->flush();

  uint16_t response_len = 3 + (count * 2) + 2;
  uint8_t buf[256];
  
  uint32_t start = millis();
  while (this->available() < response_len) {
    if (millis() - start > 200) return false;
    yield();
  }

  if (this->read_array(buf, response_len)) {
    if (calc_crc(buf, response_len - 2) != ((buf[response_len-1] << 8) | buf[response_len-2])) return false;
    for (int i = 0; i < count; i++) data[i] = (buf[3 + (i*2)] << 8) | buf[3 + (i*2) + 1];
    return true;
  }
  return false;
}

void PZEM6L24Component::setup() {
  this->clearBuffer();
  ESP_LOGI(TAG, "PZEM-6L24 Component Initialized");
}

void PZEM6L24Component::loop() {
  if (millis() - this->last_update_ >= this->update_interval_) {
    this->last_update_ = millis();
    this->refresh_values();
  }
}

void PZEM6L24Component::dump_config() {
  ESP_LOGCONFIG(TAG, "PZEM-6L24:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->_slaveAddr);
  ESP_LOGCONFIG(TAG, "  Registered Sensors: %d", this->sensors_.size());
}

// Efficient Bulk Read Strategy
void PZEM6L24Component::refresh_values() {
  // 1. Read Voltages (3 regs)
  uint16_t data[6];
  if (read_input_registers(this->_slaveAddr, PZEM_VOLTAGE_REG, 3, data)) {
    this->vol_[0] = data[0] * PZEM_VOLTAGE_RESOLUTION;
    this->vol_[1] = data[1] * PZEM_VOLTAGE_RESOLUTION;
    this->vol_[2] = data[2] * PZEM_VOLTAGE_RESOLUTION;
  } else { this->vol_[0] = this->vol_[1] = this->vol_[2] = NAN; }

  // 2. Read Currents (3 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_CURRENT_REG, 3, data)) {
    this->cur_[0] = data[0] * PZEM_CURRENT_RESOLUTION;
    this->cur_[1] = data[1] * PZEM_CURRENT_RESOLUTION;
    this->cur_[2] = data[2] * PZEM_CURRENT_RESOLUTION;
  } else { this->cur_[0] = this->cur_[1] = this->cur_[2] = NAN; }

  // 3. Read Frequencies (3 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_FREQUENCY_REG, 3, data)) {
    this->freq_[0] = data[0] * PZEM_FREQUENCY_RESOLUTION;
    this->freq_[1] = data[1] * PZEM_FREQUENCY_RESOLUTION;
    this->freq_[2] = data[2] * PZEM_FREQUENCY_RESOLUTION;
  } else { this->freq_[0] = this->freq_[1] = this->freq_[2] = NAN; }

  // 4. Read Active Power (3 phases * 2 regs = 6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_ACTIVE_POWER_REG, 6, data)) {
    this->act_pwr_[0] = combine_registers(data[0], data[1], true) * PZEM_POWER_RESOLUTION;
    this->act_pwr_[1] = combine_registers(data[2], data[3], true) * PZEM_POWER_RESOLUTION;
    this->act_pwr_[2] = combine_registers(data[4], data[5], true) * PZEM_POWER_RESOLUTION;
  } else { this->act_pwr_[0] = this->act_pwr_[1] = this->act_pwr_[2] = NAN; }

  // 5. Read Reactive Power (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_REACTIVE_POWER_REG, 6, data)) {
    this->react_pwr_[0] = combine_registers(data[0], data[1], true) * PZEM_POWER_RESOLUTION;
    this->react_pwr_[1] = combine_registers(data[2], data[3], true) * PZEM_POWER_RESOLUTION;
    this->react_pwr_[2] = combine_registers(data[4], data[5], true) * PZEM_POWER_RESOLUTION;
  } else { this->react_pwr_[0] = this->react_pwr_[1] = this->react_pwr_[2] = NAN; }

  // 6. Read Apparent Power (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_APPARENT_POWER_REG, 6, data)) {
    this->app_pwr_[0] = combine_registers(data[0], data[1], true) * PZEM_POWER_RESOLUTION;
    this->app_pwr_[1] = combine_registers(data[2], data[3], true) * PZEM_POWER_RESOLUTION;
    this->app_pwr_[2] = combine_registers(data[4], data[5], true) * PZEM_POWER_RESOLUTION;
  } else { this->app_pwr_[0] = this->app_pwr_[1] = this->app_pwr_[2] = NAN; }

  // 7. Read Power Factor (2 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_POWER_FACTOR_A_B_REG, 2, data)) {
    this->pf_[0] = ((data[0] >> 8) & 0xFF) * PZEM_POWER_FACTOR_RESOLUTION;
    this->pf_[1] = (data[0] & 0xFF) * PZEM_POWER_FACTOR_RESOLUTION;
    this->pf_[2] = ((data[1] >> 8) & 0xFF) * PZEM_POWER_FACTOR_RESOLUTION;
    this->total_pf_ = (data[1] & 0xFF) * PZEM_POWER_FACTOR_RESOLUTION;
  } else { this->pf_[0] = this->pf_[1] = this->pf_[2] = this->total_pf_ = NAN; }

  // 8. Read Active Energy (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_ACTIVE_ENERGY_REG, 6, data)) {
    this->act_nrg_[0] = combine_registers(data[0], data[1], true) * PZEM_ENERGY_RESOLUTION;
    this->act_nrg_[1] = combine_registers(data[2], data[3], true) * PZEM_ENERGY_RESOLUTION;
    this->act_nrg_[2] = combine_registers(data[4], data[5], true) * PZEM_ENERGY_RESOLUTION;
  } else { this->act_nrg_[0] = this->act_nrg_[1] = this->act_nrg_[2] = NAN; }

  // 9. Read Reactive Energy (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_REACTIVE_ENERGY_REG, 6, data)) {
    this->react_nrg_[0] = combine_registers(data[0], data[1], true) * PZEM_ENERGY_RESOLUTION;
    this->react_nrg_[1] = combine_registers(data[2], data[3], true) * PZEM_ENERGY_RESOLUTION;
    this->react_nrg_[2] = combine_registers(data[4], data[5], true) * PZEM_ENERGY_RESOLUTION;
  } else { this->react_nrg_[0] = this->react_nrg_[1] = this->react_nrg_[2] = NAN; }

  // 10. Read Apparent Energy (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_APPARENT_ENERGY_REG, 6, data)) {
    this->app_nrg_[0] = combine_registers(data[0], data[1], true) * PZEM_ENERGY_RESOLUTION;
    this->app_nrg_[1] = combine_registers(data[2], data[3], true) * PZEM_ENERGY_RESOLUTION;
    this->app_nrg_[2] = combine_registers(data[4], data[5], true) * PZEM_ENERGY_RESOLUTION;
  } else { this->app_nrg_[0] = this->app_nrg_[1] = this->app_nrg_[2] = NAN; }

  // 11. Read Combined Power (6 regs total for all 3 combined types)
  if (read_input_registers(this->_slaveAddr, PZEM_ACTIVE_POWER_COMBINED_REG, 6, data)) {
    this->total_act_pwr_ = combine_registers(data[0], data[1], true) * PZEM_POWER_RESOLUTION;
    this->total_react_pwr_ = combine_registers(data[2], data[3], true) * PZEM_POWER_RESOLUTION;
    this->total_app_pwr_ = combine_registers(data[4], data[5], true) * PZEM_POWER_RESOLUTION;
  } else { this->total_act_pwr_ = this->total_react_pwr_ = this->total_app_pwr_ = NAN; }

  // 12. Read Combined Energy (6 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_ACTIVE_ENERGY_COMBINED_REG, 6, data)) {
    this->total_act_nrg_ = combine_registers(data[0], data[1], true) * PZEM_ENERGY_RESOLUTION;
    this->total_react_nrg_ = combine_registers(data[2], data[3], true) * PZEM_ENERGY_RESOLUTION;
    this->total_app_nrg_ = combine_registers(data[4], data[5], true) * PZEM_ENERGY_RESOLUTION;
  } else { this->total_act_nrg_ = this->total_react_nrg_ = this->total_app_nrg_ = NAN; }

  // 13. Read Angles (5 regs)
  if (read_input_registers(this->_slaveAddr, PZEM_VOLTAGE_PHASE_REG, 5, data)) {
    this->vol_ang_[0] = 0.0f; // Ref
    this->vol_ang_[1] = data[0] * PZEM_PHASE_RESOLUTION;
    this->vol_ang_[2] = data[1] * PZEM_PHASE_RESOLUTION;
    this->cur_ang_[0] = data[2] * PZEM_PHASE_RESOLUTION;
    this->cur_ang_[1] = data[3] * PZEM_PHASE_RESOLUTION;
    this->cur_ang_[2] = data[4] * PZEM_PHASE_RESOLUTION;
  } else {
    this->vol_ang_[0] = this->vol_ang_[1] = this->vol_ang_[2] = NAN;
    this->cur_ang_[0] = this->cur_ang_[1] = this->cur_ang_[2] = NAN;
  }

  // Update all registered sensors
  for (auto *sens : this->sensors_) {
    float val = NAN;
    PZEMType t = sens->get_type();
    PZEMPhase p = sens->get_phase();
    int idx = (int)p;

    if (p == PHASE_COMBINED) {
      if (t == PZEM_TYPE_VOLTAGE) {
        // Calculate Average Voltage
        // If any phase is NAN, the result will be NaN (C++ standard for math with NaNs)
        val = (this->vol_[0] + this->vol_[1] + this->vol_[2]) / 3.0f;
      }
      else if (t == PZEM_TYPE_ACTIVE_POWER) val = this->total_act_pwr_;
      else if (t == PZEM_TYPE_REACTIVE_POWER) val = this->total_react_pwr_;
      else if (t == PZEM_TYPE_APPARENT_POWER) val = this->total_app_pwr_;
      else if (t == PZEM_TYPE_ACTIVE_ENERGY) val = this->total_act_nrg_;
      else if (t == PZEM_TYPE_REACTIVE_ENERGY) val = this->total_react_nrg_;
      else if (t == PZEM_TYPE_APPARENT_ENERGY) val = this->total_app_nrg_;
      else if (t == PZEM_TYPE_POWER_FACTOR) val = this->total_pf_;
    } else if (idx < 3) {
      switch (t) {
        case PZEM_TYPE_VOLTAGE: val = this->vol_[idx]; break;
        case PZEM_TYPE_CURRENT: val = this->cur_[idx]; break;
        case PZEM_TYPE_FREQUENCY: val = this->freq_[idx]; break;
        case PZEM_TYPE_ACTIVE_POWER: val = this->act_pwr_[idx]; break;
        case PZEM_TYPE_REACTIVE_POWER: val = this->react_pwr_[idx]; break;
        case PZEM_TYPE_APPARENT_POWER: val = this->app_pwr_[idx]; break;
        case PZEM_TYPE_POWER_FACTOR: val = this->pf_[idx]; break;
        case PZEM_TYPE_ACTIVE_ENERGY: val = this->act_nrg_[idx]; break;
        case PZEM_TYPE_REACTIVE_ENERGY: val = this->react_nrg_[idx]; break;
        case PZEM_TYPE_APPARENT_ENERGY: val = this->app_nrg_[idx]; break;
        case PZEM_TYPE_VOLTAGE_PHASE_ANGLE: val = this->vol_ang_[idx]; break;
        case PZEM_TYPE_CURRENT_PHASE_ANGLE: val = this->cur_ang_[idx]; break;
        default: val = NAN;
      }
    }
    
    if (!std::isnan(val)) {
      sens->publish_state(val);
    }
  }
}

}  // namespace pzem6l24
}  // namespace esphome