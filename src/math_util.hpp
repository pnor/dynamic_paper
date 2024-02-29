#pragma once

/** Helper Math functions */

/** Computes `number % modulo` */
template <typename T> constexpr T mod(const T number, const T modulo) {
  return ((number % modulo) + modulo) % modulo;
}
