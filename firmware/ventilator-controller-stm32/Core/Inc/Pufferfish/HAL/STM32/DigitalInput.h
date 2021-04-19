/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 *      Author: March Boonyapaluk
 */

#pragma once

#include "Pufferfish/HAL/Interfaces/DigitalInput.h"
#include "stm32h7xx_hal.h"

namespace Pufferfish {
namespace HAL {
namespace STM32 {

/**
 * Represents a GPIO input in STM32
 */
class DigitalInput : public Interfaces::DigitalInput {
 public:
  /**
   * Constructs a new Digital Input
   * @param port  GPIO port of the MCU (A, B, ...)
   * @param pin   GPIO pin of the MCU (1, 2, ...)
   * @param inverted   true if the input is active-low, false for active-high
   */
  DigitalInput(GPIO_TypeDef &port, const uint16_t pin, const bool inverted = false)
      : port_(port), pin(pin), inverted(inverted) {}

  /**
   * Reads a digital input from the GPIO pin
   * @return true if the pin is active (HIGH when inverted=false,
   *  and LOW when inverted=true), false otherwise
   */
  bool read() override;

 private:
  GPIO_TypeDef &port_;
  const uint16_t pin;
  const bool inverted;
};

} //namespace STM32
}  // namespace HAL
}  // namespace Pufferfish
