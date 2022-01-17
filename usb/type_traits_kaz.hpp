#pragma once
#include <array>
#include <tuple>
#include <type_traits>

///@file std type_traits doesn't hit all the cases the way I'd like. e.g.
/// std::add_volatile<T> where T is a container
/// std::add_cv <T>, but it's conditional now.
/// std::get <>, but it's multi-dimensional, and handles legacy arrays.
/// std::tuple_element, but it handles legacy arrays.
/// tuple_element_offset, this isn't std.  But ought to be.

/*----------------------------------------------------------------------------*/
///@brief Type-storage struct for dissect down below.
template <typename M, typename S>
struct member_of_struct_types {
  using member_t = M;
  using struct_t = S;
};

///@brief Change a pointer-to-struct-member into type information.
template <typename M, typename S>
constexpr member_of_struct_types <M, S> dissect (M S::*ptr) noexcept
{ return {}; }

/*----------------------------------------------------------------------------*/
///@brief std::add_volatile_t, but for container's types.
template <typename T> struct weave_volatile { using type = volatile T; };

template <typename T, std::size_t N>
struct weave_volatile <T[N]>
{ using helper = weave_volatile<T>::type;
  using type = helper[N];
};

template <typename T, std::size_t N>
struct weave_volatile <std::array <T, N>>
{ using type = std::array <typename weave_volatile <T>::type, N>; };

template <typename...Ts>
struct weave_volatile <std::tuple <Ts...>>
{ using type = std::tuple <typename weave_volatile <Ts>::type ...>; };

template <typename T>
struct weave_volatile <T*>
{ using type = std::add_pointer_t <typename weave_volatile <T>::type>; };

template <typename T>
using weave_volatile_t = typename weave_volatile<T>::type;

/*----------------------------------------------------------------------------*/
///@brief Produce a type of dst_t, with the cv-qualifiers for src_t.
///@param src_t synthesizes ::type, s.t. is_const_v<::type> == is_const_v<src_t>
///             and likewise with is_volatile_v.
///@param dst_t synthesizes ::type, s.t. remove_cv_t<::type> == remove_cv_t<dst_t>
///@note src_t as a container is undefined.
template <typename src_t, typename dst_t>
struct copy_cv {
  template <typename D>
  using maybe_v_t = std::conditional_t
  <std::is_volatile_v <src_t>, weave_volatile_t <D>, std::remove_volatile_t<D>>;
  
  template <typename D>
  using maybe_c_t = std::conditional_t
  <std::is_const_v <src_t>, std::add_const_t <D>, std::remove_const_t<D>>;
  
  using type = maybe_c_t <maybe_v_t <dst_t>>;
};

template <typename src_t, typename dst_t>
using copy_cv_t = typename copy_cv <src_t, dst_t> ::type;

/*----------------------------------------------------------------------------*/
///@brief std::get, but with support for multi-index dimensions.
template <typename T>
constexpr auto && getm (T && t) noexcept { return std::forward <T> (t); }

///@brief getm specialization for legacy arrays.
template <std::size_t index, std::size_t...rest_idxs, typename T, std::size_t N>
constexpr auto & getm (T (&t) [N]) noexcept {
  static_assert ((0 <= index && index < N) || N == 0, "index out of range");
  return getm <rest_idxs...> (t[index]);
}

///@brief Forwarder to std::get for cases we don't handle.
template <std::size_t index, std::size_t...rest_idxs, typename T>
constexpr auto && getm (T && t) noexcept {
  return getm <rest_idxs...> (std::get <index> (std::forward <T> (t)));
}

/*----------------------------------------------------------------------------*/
///@brief std::tuple_element, but with support for legacy arrays.
template <std::size_t index, typename T>
struct tuple_element {
  using type = std::tuple_element_t <index, T>;
};

template <std::size_t index, typename T, std::size_t N>
struct tuple_element <index, T[N]> {
  static_assert (index < N || N == 0);
  using type = T;
};

template <std::size_t index, typename T>
using tuple_element_t = typename tuple_element <index, T> ::type;

/*----------------------------------------------------------------------------*/
///@brief Get the byte-offset of a tuple's element.  It also works for arrays.
template <std::size_t I, typename Tuple>
constexpr std::size_t element_offset() {
  using element_t = tuple_element_t <I, Tuple>;
  static_assert (!std::is_reference_v <element_t>);
  
  union {
    char a [sizeof (Tuple)];
    Tuple t{};
  };
  
  auto* p = std::addressof (getm <I> (t));

  for (std::size_t i = 0;; ++i) {
    if (static_cast <void*> (a + i) == p) {
      return i;
    }
  } /* ^ This is the dumbest constexpr work-around I've ever seen. */
}

///@brief Ensure element_offset() can be accessed as std::integral_constant.
template <std::size_t index, typename T>
struct tuple_element_offset
: public std::integral_constant <std::size_t, element_offset <index, T> ()>
{ };

