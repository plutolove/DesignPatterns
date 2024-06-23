#pragma once
#include <array>
#include <type_traits>

#include "log.h"

namespace pluto {

template <typename... Args>
struct TypeList {
  using type = TypeList<Args...>;
  constexpr static auto size = sizeof...(Args);
  template <typename... T>
  using append = TypeList<Args..., T...>;

  template <template <typename...> class T>
  using exportTo = T<Args...>;
};

template <typename... Ts>
void printTypeName() {
  (INFO("{}", typeid(Ts).name()), ...);
}

template <typename... Ts>
void printTypeName(TypeList<Ts...>) {
  (INFO("{}", Demangle<Ts>()), ...);
}

// 高阶函数
template <typename Ts, template <typename> class F>
struct Map;

template <typename... Ts, template <typename> class F>
struct Map<TypeList<Ts...>, F> {
  using type = TypeList<typename F<Ts>::type...>;
};

template <typename Ts, template <typename> class F>
using Map_t = typename Map<Ts, F>::type;

template <typename In, template <typename> class F, typename Out = TypeList<>>
struct Filter {
  using type = Out;
};

template <typename T, typename... Ts, template <typename> class F, typename Out>
struct Filter<TypeList<T, Ts...>, F, Out>
    : std::conditional_t<
          F<T>::value,
          Filter<TypeList<Ts...>, F, typename Out::template append<T>>,
          Filter<TypeList<Ts...>, F, Out>> {};

template <typename In, template <typename> class F>
using Filter_t = typename Filter<In, F>::type;

template <typename Ts, template <typename, typename> class F, typename Init>
struct Fold {
  using type = Init;
};

template <typename Ts, template <typename, typename> class F, typename Init>
using Fold_t = typename Fold<Ts, F, Init>::type;

template <typename T, typename... Ts, template <typename, typename> class F,
          typename Init>
struct Fold<TypeList<T, Ts...>, F, Init>
    : Fold<TypeList<Ts...>, F, typename F<Init, T>::type> {};

template <typename L, typename R>
struct Concat {
  template <typename Init, typename T>
  struct Append : Init::template append<T> {};
  using type = Fold_t<R, Append, L>;
};

template <typename L, typename R>
using Concat_t = typename Concat<L, R>::type;

template <typename Ts, typename E>
struct TypeIn {
  template <typename Init, typename T>
  struct In : std::conditional_t<Init::value, Init, std::is_same<E, T>> {};
  using type = Fold_t<Ts, In, std::false_type>;
  constexpr static bool value = type::value;
};

template <typename Ts, typename E>
constexpr bool TypeIn_v = TypeIn<Ts, E>::value;

template <typename Ts>
struct Unique {
  template <typename Init, typename T>
  struct Append : std::conditional_t<TypeIn_v<Init, T>, Init,
                                     typename Init::template append<T>> {};
  using type = Fold_t<Ts, Append, TypeList<>>;
};

template <typename Ts>
using Unique_t = typename Unique<Ts>::type;

template <typename Ts, template <typename, typename> class Cmp>
struct Sort {
  using type = TypeList<>;
};

template <template <typename, typename> class Cmp, typename Cur, typename... Ts>
struct Sort<TypeList<Cur, Ts...>, Cmp> {
  using tails = TypeList<Ts...>;
  template <typename E>
  struct LT {
    constexpr static bool value = Cmp<E, Cur>::value;
  };
  template <typename E>
  struct GT {
    constexpr static bool value = !Cmp<E, Cur>::value;
  };

  using SmallPart = typename Sort<Filter_t<tails, LT>, Cmp>::type;
  using BigPart = typename Sort<Filter_t<tails, GT>, Cmp>::type;
  using type = Concat_t<typename SmallPart::template append<Cur>, BigPart>;
};

template <typename F, typename T>
struct Edge {
  using From = F;
  using To = T;
};

template <typename T, typename Out = TypeList<>>
struct Chain {
  using type = Out;
};

template <typename F, typename Out>
struct Chain<auto (*)(F)->void, Out> {
  using From = F;
  using type = Out;
};

template <typename F, typename T, typename Out>
struct Chain<auto (*)(F)->T, Out> {
  using From = F;
  using To = typename Chain<T>::From;

  using type =
      typename Chain<T, typename Out::template append<Edge<From, To>>>::type;
};

template <typename... Ts>
struct Graph {
  using type =
      Fold_t<TypeList<typename Chain<Ts>::type...>, Concat, TypeList<>>;
};

template <typename Node = void>
struct EdgeTrait {
  template <typename Edge>
  struct IsFrom {
    static constexpr auto value = std::is_same_v<Node, typename Edge::From>;
  };

  template <typename Edge>
  struct IsTo {
    static constexpr auto value = std::is_same_v<Node, typename Edge::To>;
  };

  template <typename Edge>
  struct GetFrom {
    using type = typename Edge::From;
  };

  template <typename Edge>
  struct GetTo {
    using type = typename Edge::To;
  };
};

template <typename Edges, typename From, typename Target,
          typename Path = TypeList<>, typename = void>
struct FindShortestPath;

// template <typename Edges, typename From, typename Target>
// using FindShortestPath_t = FindShortestPath<Edges, From, Target>::type;

template <typename Edges, typename Target, typename Path>
struct FindShortestPath<Edges, Target, Target, Path>
    : Path::template append<Target> {};

template <typename Edges, typename From, typename Target, typename Path>
struct FindShortestPath<Edges, From, Target, Path,
                        std::enable_if_t<TypeIn_v<Path, From>>> : TypeList<> {};

template <typename Edges, typename From, typename Target, typename Path>
struct FindShortestPath<
    Edges, From, Target, Path,
    std::enable_if_t<!TypeIn_v<Path, From> && !std::is_same_v<From, Target>>> {
  using CEdges =
      Filter_t<Edges, EdgeTrait<From>::template IsFrom>;  // 以from为起点的边

  using CNodes = Map_t<CEdges, EdgeTrait<>::GetTo>;  // 获取所有to节点

  template <typename NEXT_NODE>
  struct FPath : FindShortestPath<Edges, NEXT_NODE, Target,
                                  typename Path::template append<From>> {};

  using AllPath = Map_t<CNodes, FPath>;

  template <typename ACC, typename CP>
  struct MiniPath
      : std::conditional_t<(ACC::size == 0 ||
                            ((ACC::size > CP::size) && CP::size > 0)),
                           CP, ACC> {};
  using type = Fold_t<AllPath, MiniPath, TypeList<>>;
};

}  // namespace pluto
