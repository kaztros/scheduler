#include <tuple>
#include <utility>
#include <Tricks/type_traits_more.hpp>


constexpr bool TUPLE_IS_ALLOCATED_BACKWARDS () {
    std::tuple <int, int> x {1, 2};
    return (& std::get<0>(x)) > (& std::get<1>(x));
}


template <typename T, typename TT = typename std::remove_reference <T> ::type, size_t... I>
constexpr auto reverse_impl (T & t, std::index_sequence<I...>)
//-> std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, TT>::type...>
{
  constexpr auto N = sizeof...(I);
  return
    std::tuple
      <std::tuple_element_t <N-I-1, TT>...>
      (std::get <N-I-1>(t)...);
}


template <typename T, typename TT = std::remove_reference_t<T>, size_t... I>
constexpr auto reverse_impl (T && t, std::index_sequence<I...>)
//-> std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, TT>::type...>
{
  constexpr auto N = sizeof...(I);
  return
    std::tuple
      <std::tuple_element_t <N-I-1, TT>...>
      (std::get <N-I-1> (std::move (t))...);
}


template <typename T, typename TT = typename std::remove_reference<T>::type>
constexpr auto reverse (T && t) {
  return reverse_impl
    (std::forward <T> (t)
    ,std::make_index_sequence <std::tuple_size_v <TT>> ()
    );
}


template <typename TUPLE_T, size_t...IDXS>
auto stop_forwarding_rvalues_helper (TUPLE_T && t, std::index_sequence<IDXS...>) {
  using TT = typename std::remove_reference <TUPLE_T> ::type;
  return std::tuple
    <remove_rvalue_reference_t <std::tuple_element_t <IDXS, TT>>...>
    (std::get <IDXS> (std::forward <TUPLE_T> (t))...);
}

template <typename TUPLE_T>
auto stop_forwarding_rvalues (TUPLE_T && t) {
  using TT = remove_rvalue_reference_t <TUPLE_T>;
  return stop_forwarding_rvalues_helper
    (std::forward <TUPLE_T> (t)
    ,std::make_index_sequence<std::tuple_size_v<TT>> ()
    );
}


/* DO NOT CALL */
template <typename RET_T, typename ... T>
auto tuple_type_helper (RET_T (*) (T...)) {
  std::tuple <T...>* x;
  return x;
}

template <auto CALLABLE>
using TupleTypeFromFunction_t = std::remove_pointer_t <decltype (tuple_type_helper (CALLABLE))>;

