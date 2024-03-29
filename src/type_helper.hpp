#pragma once

/** Collection of functions for template type manipulation */

#include <optional>
#include <vector>

#include "type_traits"

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

// ===== is optional ===============
namespace is_optional_impl {
template <typename> constexpr bool is_optional_impl = false;
template <typename T> constexpr bool is_optional_impl<std::optional<T>> = true;
} // namespace is_optional_impl

template <typename T>
constexpr bool is_optional =
    is_optional_impl::is_optional_impl<std::remove_cvref_t<T>>;

} // namespace dynamic_paper
