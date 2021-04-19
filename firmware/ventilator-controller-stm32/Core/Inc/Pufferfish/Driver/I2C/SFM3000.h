/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 * SFM3000.h
 *
 *      Author: March Boonyapaluk
 *
 *  A driver for the SFM3000 Low Pressure Drop Digital Flow Meter
 */

#pragma once

#include "Pufferfish/Driver/Testable.h"
#include "Pufferfish/HAL/CRCChecker.h"
#include "Pufferfish/HAL/Interfaces/Time.h"
#include "SensirionDevice.h"

namespace Pufferfish {
namespace Driver {
namespace I2C {

/**
 * All data in a reading from the Sensirion SFM3000 mass flow meter.
 */
struct SFM3000Sample {
  uint16_t raw_flow;
  float flow;
};

/**
 * Driver of Sensirion SFM3000 flow sensor
 */
class SFM3000 : public Testable {
 public:
  static constexpr uint16_t default_i2c_addr = 0x40;

  static const int offset_flow = 32000;
  static constexpr float scale_factor_air = 140.0F;
  static constexpr float scale_factor_o2 = 142.8F;

  explicit SFM3000(HAL::Interfaces::I2CDevice &dev, HAL::Interfaces::Time &time, float scale_factor = scale_factor_air)
      : crc8_(crc_params), sensirion_(dev, crc8_), time_(time), scale_factor_(scale_factor) {}

  /**
   * Starts a flow measurement
   * @return ok on success, error code otherwise
   */
  I2CDeviceStatus start_measure();

  /**
   * Reads out the serial number
   * @param sn[out] the unique serial number
   * @return ok on success, error code otherwise
   */
  I2CDeviceStatus serial_number(uint32_t &sn);

  /**
   * Reads out the flow rate from the sensor
   * @param sample[out] the sensor reading; only valid on success
   * @return ok on success, error code otherwise
   */
  I2CDeviceStatus read_sample(SFM3000Sample &sample);

  I2CDeviceStatus reset() override;
  I2CDeviceStatus test() override;

 private:
  static constexpr HAL::CRC8Parameters crc_params = {0x31, 0x00, false, false, 0x00};

  HAL::SoftCRC8 crc8_;
  SensirionDevice sensirion_;
  bool measuring_ = false;
  HAL::Interfaces::Time &time_;
  float scale_factor_;
};

}  // namespace I2C
}  // namespace Driver
}  // namespace Pufferfish
