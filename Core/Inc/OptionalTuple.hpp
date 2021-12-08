#include <optional>
#include <tuple>
#include "BitField.hpp"

template <typename ... TYPES>
struct OptionalTuple {
  static constexpr auto SIZE = sizeof...(TYPES);

  std::tuple <TYPES...> values;
  BitSet<SIZE> is_values; 
};


template <size_t IDX, typename ... TYPES>
auto get (OptionalTuple <TYPES...> & ot) noexcept -> std::optional <std::tuple_element_t <IDX, decltype(ot)> & > {
  if (GetIndex(ot.is_values, IDX)) {
    return std::make_optional (std::get <IDX> (ot.values));
  } else {
    return {};
  }
}


template <size_t IDX, typename ... TYPES>
auto get (OptionalTuple <TYPES...> && ot) noexcept -> std::optional <std::tuple_element_t <IDX, decltype(ot)>> {
  if (GetIndex (ot.is_values, IDX)) {
    return std::make_optional (std::get <IDX> (std::forward <std::tuple <TYPES...>> (ot.values)));
  } else {
    return {};
  }
}


