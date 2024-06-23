#pragma once
#include <cxxabi.h>

#include "spdlog/spdlog.h"

// #include "boost/"
#define INFO(fmt, args...) spdlog::info(fmt, ##args)
#define ERROR(fmt, args...) spdlog::error(fmt, ##args)
#define WARN(fmt, args...) spdlog::warn(fmt, ##args)
#define FATAL(fmt, args...) spdlog::critical(fmt, ##args)

template <typename T>
inline std::string Demangle() {
  int status;
  std::unique_ptr<char, void (*)(void*)> demangled(
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
      std::free);
  return (status == 0) ? std::string(demangled.get()) : "Error demangling";
}
