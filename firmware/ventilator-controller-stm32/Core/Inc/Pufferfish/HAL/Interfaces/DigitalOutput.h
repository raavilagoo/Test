/// DigitalOutput.h
/// This file has interface class and methods for Digital Output.

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

#pragma once

namespace Pufferfish {
namespace HAL {
namespace Interfaces {

/**
 * An abstract class which represents a GPIO output
 */
class DigitalOutput {
 public:
  /**
   * Writes a digital output to the GPIO pin
   * @param output  true if desired output is active, false if non-active
   */
  virtual void write(bool output) = 0;
};

} //namespace Interfaces
}  // namespace HAL
}  // namespace Pufferfish


