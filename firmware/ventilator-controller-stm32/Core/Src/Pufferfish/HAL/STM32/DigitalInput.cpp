/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 *      Author: March Boonyapaluk
 */

#include <Pufferfish/HAL/STM32/DigitalInput.h>

namespace Pufferfish::HAL::STM32 {

bool DigitalInput::read() {
  return inverted ? HAL_GPIO_ReadPin(&port_, pin) == GPIO_PIN_RESET
                  : HAL_GPIO_ReadPin(&port_, pin) == GPIO_PIN_SET;
}

}  // namespace Pufferfish::HAL::STM32
