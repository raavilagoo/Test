/*
 * UARTSerial.h
 *
 *  Created on: May 20, 2020
 *      Author: Ethan Li
 */

#pragma once

#include <Pufferfish/HAL/STM32/BufferedUART.h>
#include "Pufferfish/Application/States.h"
#include "Pufferfish/Driver/Serial/Backend/Backend.h"
#include "Pufferfish/HAL/Interfaces/CRCChecker.h"

namespace Pufferfish::Driver::Serial::Backend {

class UARTBackend {
 public:
  using BufferedUART = HAL::STM32::LargeBufferedUART;

  UARTBackend(volatile BufferedUART &uart, HAL::Interfaces::CRC32 &crc32c, Application::States &states)
      : uart_(uart), backend_(crc32c, states) {}

  void setup_irq();
  void receive();
  void update_clock(uint32_t current_time);
  void send();

 private:
  volatile BufferedUART &uart_;
  Backend backend_;
  FrameProps::ChunkBuffer send_output_;
  HAL::AtomicSize sent_ = 0;
};

}  // namespace Pufferfish::Driver::Serial::Backend

#include "UART.tpp"
