#include <array>
#include <tuple>
#include <type_traits>

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
{ using type = T volatile *; };

template <typename T>
using weave_volatile_t = typename weave_volatile<T>::type;

/*----------------------------------------------------------------------------*/
///@brief Produce a type of dst_t, with the cv-qualifiers for src_t.
///@param src_t synthesizes ::type, s.t. is_const_v<::type> == is_const_v<src_t>, and likewise with volatile.
///@param dst_t synthesizes ::type, s.t. remove_cv_t<::type> == remove_cv_t<dst_t>
///@note Behavior undefined if src_t is a container.
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

//an array within a tuple breaks g++, unless getm is forward declared.
//Probably...
template <std::size_t index, std::size_t...rest_idxs, typename T, std::size_t N>
constexpr auto & getm (T (&t) [N]) noexcept;

///@brief getm specialization for legacy arrays.
template <std::size_t index, std::size_t...rest_idxs, typename T, std::size_t N>
constexpr auto & getm (T (&t) [N]) noexcept {
  static_assert ((0 < index && index < N) || N == 0, "index out of range");
  return getm <rest_idxs...> (t[index]);
}

///@brief Forwarder to std::get for cases we don't handle.
template <std::size_t index, std::size_t...rest_idxs, typename T>
constexpr auto && getm (T && t) noexcept {
  return getm <rest_idxs...> (std::get <index> (std::forward <T> (t)));
}
