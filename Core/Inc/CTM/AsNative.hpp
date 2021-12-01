#include <cstddef>
#include <cstdint>
#include <type_traits>

template <int WIDTH>
struct AsNative {
  using type = typename std::enable_if
    < (0 <= WIDTH && WIDTH <= 32)
    , typename AsNative<WIDTH+1>::type
    > ::type;
};

struct EmptyType { };

using uregister_t = unsigned int;

template <> struct AsNative <32> { using type = uint32_t; };
template <> struct AsNative <16> { using type = uint16_t; };
template <> struct AsNative  <8> { using type = uint8_t; };
template <> struct AsNative  <1> { using type = bool; };
template <> struct AsNative  <0> { using type = EmptyType; };

constexpr size_t REG_WIDTH () {  return 8 * sizeof(uregister_t);  }

