/// AnalogInput.cpp
/// This file has methods for abstract STM32 HAL class for Analog Input.

// Copyright (c) 2020 Pez-Globo and the Pufferfish project contributors
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied.
//
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Pufferfish/HAL/STM32/AnalogInput.h>

namespace Pufferfish::HAL::STM32 {

/**
 * @brief  Calls HAL_ADC_Start
 * @param  None
 * @return ADCStatus returns error/ok
 */
ADCStatus AnalogInput::start() {
  /* Invoke HAL_ADC_Start to start the ADC */
  HAL_StatusTypeDef start_ret_val = HAL_ADC_Start(&adc_input_);

  /* Validate the HAL_ADC_Start return value to return ADCStatus */
  if (start_ret_val == HAL_OK) {
    /* return ADCStatus as ok */
    return ADCStatus::ok;
  }

  /* return ADCStatus as error */
  return ADCStatus::error;
}

/**
 * @brief  Invokes HAL_ADC_PollForConversion for Adc_Input with Timeout
 *         for the polling conversion of read data.
 * @brief  Invokes HAL_ADC_GetValue to read data of Adc_Input
 * @param  AnalogDataRead, Out parameter returns the read data
 * @retval ADCStatus returns error/ok
 */
ADCStatus AnalogInput::read(uint32_t &analog_data_read) {
  HAL_StatusTypeDef tmp_poll_cvr_rtn = HAL_OK;

  /**
   * @brief  Polling conversion of ADC3, based on EOCSelection
   */
  tmp_poll_cvr_rtn = HAL_ADC_PollForConversion(&adc_input_, timeout_);

  /* Validate the HAL_ADC_PollForConversion return value */
  if (tmp_poll_cvr_rtn == HAL_OK) {
    /**
     * @brief  invoking HAL_ADC_GetValue to read analog data
     *         returns analog read data of 4 bytes
     */
    analog_data_read = HAL_ADC_GetValue(&adc_input_);

    /* return ADCStatus as ok */
    return ADCStatus::ok;
  }

  /* return ADCStatus as error */
  return ADCStatus::error;
}

/**
 * @brief  Invokes HAL_ADC_Stop of Adc_Input
 * @param  None
 * @retval ADCStatus returns error/ok
 */
ADCStatus AnalogInput::stop() {
  /* Invoke HAL_ADC_Stop to stop the ADC */
  HAL_StatusTypeDef start_ret_val = HAL_ADC_Stop(&adc_input_);

  /* Validate the HAL_ADC_Stop return value to return ADCStatus */
  if (start_ret_val == HAL_OK) {
    /* return ADCStatus as ok */
    return ADCStatus::ok;
  }

  /* return ADCStatus as error */
  return ADCStatus::error;
}

}  // namespace Pufferfish::HAL::STM32
