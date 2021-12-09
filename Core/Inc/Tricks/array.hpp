#include <array>
#include <utility>

template <typename T, size_t N, size_t M, size_t ... N_INDICES, size_t ... M_INDICES>
constexpr std::array <T, N+M> concatenate_helper
  ( std::array <T, N> && x
  , std::array <T, M> && y
  , std::index_sequence <N_INDICES...>
  , std::index_sequence <M_INDICES...>
  )
{ return std::array <T, N+M> { std::forward<T>(x[N_INDICES])... , std::forward<T>(y[M_INDICES])... }; }


template <typename T, size_t N>
constexpr auto concatenate (std::array <T, N> && x)
{   return x;   }


template <typename T, size_t N, size_t M, typename ... REST_T>
constexpr auto concatenate (std::array <T, N> && x, std::array <T, M> && y, REST_T && ... rest) {
  return concatenate
    ( concatenate_helper (std::forward<std::array <T, N>>(x), std::forward<std::array <T, M>>(y), std::make_index_sequence<N>(), std::make_index_sequence<M>())
    , std::forward <REST_T> (rest)...
    );
}

