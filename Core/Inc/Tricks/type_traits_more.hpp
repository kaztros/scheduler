#include <type_traits>


template <typename T, typename U>
struct is_same_decayed
: std::is_same <std::decay_t<T>, std::decay_t<U>>::type
{ };


template <typename T> struct remove_rvalue_reference      { using type = T;  };
template <typename T> struct remove_rvalue_reference<T&>  { using type = T&; };
template <typename T> struct remove_rvalue_reference<T&&> { using type = T;  };
template <typename T>
using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;




