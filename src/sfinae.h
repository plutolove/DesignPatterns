#pragma once

#include <iostream>

template <typename... Args>
struct Overload : Args... {
  using Args::operator()...;
};

template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

template <typename T>
void f(typename T::i* a) {
  std::cout << "f(T) " << sizeof(T::i) << std::endl;
}

void f(...) { std::cout << "f(...)" << std::endl; }

template <typename T>
struct IsClass {
  template <typename X>
  static char test(int X::*);
  template <typename X>
  static int test(...);

  auto static constexpr value = sizeof(test<T>(NULL)) == 1;
};

class Test {
  int x;
};

template <typename T>
struct HasMemberFunction {
  template <typename U>
  static auto check(double)
      -> decltype(std::declval<U>().func(double()), std::true_type{});
  template <typename U>
  static std::false_type check(...);
  auto static constexpr value = decltype(check<T>(0.0))::value;
};

class TestClass {
 public:
  void func(double) {}
};
