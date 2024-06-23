#pragma once
#include <array>
#include <cstddef>

namespace pluto {

template <size_t size>
constexpr auto typeName(const char (&data)[size]) {
  std::array<char, size> arr{};
  for (size_t i = 0; i < size; i++) {
    arr[i] = data[i];
  }
  return arr;
}

}  // namespace pluto
