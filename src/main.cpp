#include <any>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "log.h"
#include "sfinae.h"
#include "type_traits.h"

template <typename... Ts>
struct Const {
  auto static constexpr Arr = []() {
    std::array<int, sizeof...(Ts)> arr{};
    auto func = [&arr](auto* ptr, size_t idx) {
      using Type = std::decay_t<std::remove_pointer_t<decltype(ptr)>>;
      if (std::is_same_v<Type, int>) {
        arr[idx] = 0;
      } else {
        arr[idx] = 1;
      }
    };
    size_t i = 0;
    (func(static_cast<Ts*>(nullptr), i++), ...);
    return arr;
  }();
};

template <typename T>
struct InputImpl1 {
  void getInput(const std::vector<std::any>& args, size_t idx) {
    // input = std::any_cast<T>(args[idx]);
    INFO("type name: {}", typeid(*this).name());
  }
  T input;
};

template <typename... Ts>
struct ArgImpl : public InputImpl1<Ts>... {
  template <size_t... I>
  void InitInput(const std::vector<std::any>& args, std::index_sequence<I...>) {
    ((InputImpl1<Ts>::getInput(args, I)), ...);
  }
  void apply(const std::vector<std::any>& args) {
    InitInput(args, std::make_index_sequence<sizeof...(Ts)>{});
  }
};

template <typename... Ts>
struct Operator : public ArgImpl<Ts...> {
  void Init(const std::vector<std::any>& args) { ArgImpl<Ts...>::apply(args); }
};

using namespace pluto;

template <typename T>
struct MapF {
  using type = std::vector<T>;
};

template <typename L, typename R>
struct Common {
  using type = std::common_type_t<L, R>;
};

template <typename L, typename R>
struct Cmp {
  constexpr static auto value = sizeof(L) < sizeof(R);
};

template <int ID>
struct Node {
  static constexpr auto id = ID;
};

int main(int argc, char** argv) {
  // for (auto& val : Const<double, int, float, char>::Arr) {
  //   INFO("val: {}", val);
  // }
  // Operator<int, double, std::vector<int>, float> tmp;
  // tmp.Init(std::vector<std::any>{});
  using st = TypeList<int, double, std::string>;
  INFO("type size: {}", st::size);
  printTypeName(st{});
  using map_t = Map_t<st, MapF>;
  printTypeName(map_t{});
  using number_t = Filter_t<st, std::is_arithmetic>;
  printTypeName(number_t{});
  using SS = TypeList<int, double, char>;
  using common_type = Fold_t<SS, Common, char>;
  INFO("_______________________");
  printTypeName<common_type>();
  using cc = Concat_t<st, SS>;
  printTypeName(cc{});
  INFO("------------{}", TypeIn<cc, int>::value);
  using ts = TypeList<int, int, double, double, std::string, float>;
  using uniq = Unique_t<ts>;
  printTypeName(uniq{});
  using sort_t = Sort<cc, Cmp>::type;
  INFO("---------------------------------------------");
  printTypeName(sort_t{});

  using g = Graph<auto (*)(Node<0>)->auto (*)(Node<1>)->auto (*)(Node<3>)->void,
                  auto (*)(Node<1>)->auto (*)(Node<2>)->auto (*)(Node<3>)->void,
                  auto (*)(Node<3>)->auto (*)(Node<1>)->void,
                  auto (*)(Node<1>)->auto (*)(Node<4>)->void>::type;
  printTypeName(g{});
  using from = Filter_t<g, EdgeTrait<Node<0>>::IsFrom>;
  printTypeName(from{});
  using to = Map_t<from, EdgeTrait<>::GetTo>::type;
  INFO("----------------------------------");
  printTypeName(to{});
  using path = FindShortestPath<g::type, Node<0>, Node<4>>::type;
  INFO("--------------------------------");
  printTypeName(path{});
  return 0;
}
