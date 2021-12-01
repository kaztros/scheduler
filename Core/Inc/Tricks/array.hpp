#include <array>
#include <utility>

template <typename T, size_t N, size_t M, size_t ... N_INDICES, size_t ... M_INDICES>
constexpr std::array <T, N+M> concatenate_helper
  ( std::array <T, N> && x
  , std::array <T, M> && y
  , std::index_sequence <N_INDICES...>
  , std::index_sequence <M_INDICES...>
  )
{ return std::array <T, N+M> { x[N_INDICES]... , y[M_INDICES]... }; }


template <typename T, size_t N, size_t M, typename ... REST_T>
constexpr auto concatenate (const std::array <T, N> && x, const std::array <T, M> && y, REST_T && ... rest) {
  return concatenate
    ( concatenate_helper (x, y, std::make_index_sequence<N>(), std::make_index_sequence<M>())
    , rest...
    );
}
