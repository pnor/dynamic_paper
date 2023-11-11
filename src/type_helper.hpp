#pragma once

#include <vector>

#include "type_traits"

/** Collection of functions for template type manipulation */

namespace dynamic_paper {

// ===== is vector ===============
namespace is_vector_impl {
template <typename T> struct IsVector : std::false_type {};
template <typename T> struct IsVector<std::vector<T>> : std::true_type {};
} // namespace is_vector_impl

template <typename T> struct is_vector {
  static constexpr const bool value =
      is_vector_impl::IsVector<std::decay_t<T>>::value;
};

} // namespace dynamic_paper
