/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 * ShiftRegister.h
 *
 *  Created on: Jun 31, 2020
 *      Author: March Boonyapaluk
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "Pufferfish/HAL/Interfaces/DigitalOutput.h"
#include "Pufferfish/HAL/Interfaces/Time.h"

namespace Pufferfish {
namespace Driver {

class ShiftRegister {
 public:
  static const int baud_rate = 20;
  static const uint8_t num_channels = 8;

  ShiftRegister(
      HAL::Interfaces::DigitalOutput &serial_in,
      HAL::Interfaces::DigitalOutput &serial_clock,
      HAL::Interfaces::DigitalOutput &r_clock,
      HAL::Interfaces::DigitalOutput &serial_clear,
      HAL::Interfaces::Time &time)
      : serial_in_(serial_in),
        serial_clock_(serial_clock),
        r_clock_(r_clock),
        serial_clear_(serial_clear),
        time_(time) {}

  void set_channel(uint8_t chan, bool out);
  [[nodiscard]] bool get_channel(uint8_t chan) const;
  void update();
  void clear();

 private:
  HAL::Interfaces::DigitalOutput &serial_in_;
  HAL::Interfaces::DigitalOutput &serial_clock_;
  HAL::Interfaces::DigitalOutput &r_clock_;
  HAL::Interfaces::DigitalOutput &serial_clear_;
  HAL::Interfaces::Time &time_;

  uint8_t output_reg_ = 0;
};

}  // namespace Driver
}  // namespace Pufferfish
