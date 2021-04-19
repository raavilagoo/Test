/*
 * Enums.h
 *
 *  Created on: May 16, 2020
 *      Author: Ethan Li
 *
 *  Utilities for working with enums.
 */

#pragma once

#include <array>

namespace Pufferfish::Util {

// EnumValues is inspired by https://stackoverflow.com/a/33091821 by user janm,
// but the implementation of the variadic template recursion is different

template <typename Enum, Enum... values>
class EnumValues;

template <typename Enum>
class EnumValues<Enum> {
 public:
  template <typename Underlying>
  static bool constexpr includes(Underlying /*test_value*/) {
    return false;
  }
};

template <typename Enum, Enum first_value, Enum... remaining_values>
class EnumValues<Enum, first_value, remaining_values...> {
 public:
  template <typename Underlying>
  static bool constexpr includes(Underlying test_value) {
    return test_value == static_cast<Underlying>(first_value) ||
           EnumValues<Enum, remaining_values...>::includes(test_value);
  }
};

}  // namespace Pufferfish::Util
