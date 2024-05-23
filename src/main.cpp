#include <type_traits>

#include "log.h"
#include "sfinae.h"

namespace design {

template <typename Lambda>
class ValidHelper {
 private:
  template <typename... Args>
  constexpr auto test(int)
      -> decltype(std::declval<Lambda>()(std::declval<Args>()...),
                  std::true_type{}) {
    return std::true_type{};
  }

  template <typename... Args>
  constexpr auto test(...) -> std::false_type {
    return std::false_type{};
  }

 public:
  template <typename... Args>
  constexpr bool operator()(const Args&... args) {
    return decltype(test<Args...>(0))::value;
  }
};

template <typename Lambda>
constexpr auto valid(Lambda&& f) {
  return ValidHelper<Lambda>{};
}

}  // namespace design

int main(int argc, char** argv) {
  auto ov = Overload{[](int i) { std::cout << "i=" << i << std::endl; },
                     [](double d) { std::cout << "d=" << d << std::endl; }};
  ov(23);
  f(23);
  INFO("{}", IsClass<Test>::value);
  INFO("{}", IsClass<int>::value);
  INFO("{}", HasMemberFunction<TestClass>::value);
  auto is_assignable =
      design::valid([](auto&& x, auto&& y) -> decltype(x + y) {});
  // INFO("is assig: {}", is_assignable);
  int a = 0;

  INFO("is_assig: {}", is_assignable(a, double{}));
  INFO("is_assig: {}", is_assignable(a, std::string{}));

  auto check_member_func =
      design::valid([](auto&& x) -> decltype(x.func(double{})) {});
  INFO("check member func: {}", check_member_func(a));
  INFO("check TestClass member func: {} ", check_member_func(TestClass{}));

  return 0;
}
