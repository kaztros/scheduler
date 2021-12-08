#include "BitField.hpp"

template <typename T, size_t SIZE>
struct OptionalPool {
  using handle_t = unsigned int;
  std::array <T, SIZE> values;
  BitSet<SIZE> is_values;
};


