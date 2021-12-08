#pragma once

#include <array>
#include <optional>

#include "CTM/AsNative.hpp"
#include "CTM/Instructions.hpp"

extern uregister_t trash;

template <size_t WIDTH>
struct BitSet {
  static constexpr size_t WIDTH_() { return WIDTH; }
  static constexpr size_t WHOLE_ARRAY_SIZE () { return WIDTH / REG_WIDTH(); }
  static constexpr size_t PARTIAL_WIDTH () { return WIDTH % REG_WIDTH(); }
  using partial_t = typename AsNative <PARTIAL_WIDTH()> ::type;
  static constexpr partial_t PARTIAL_MASK () { return (1U << WIDTH) - 1;  }

  std::array <uregister_t, WHOLE_ARRAY_SIZE()> values_whole;
  partial_t value_partial;

public:
  constexpr BitSet ()
  : value_partial (0)
  {
    for (auto & ele : values_whole) {
      ele = 0;
    }
  }
  ~BitSet() = default;

  template <typename OP>
  constexpr auto operateByWholeIndex (size_t whole_index, OP const & op) {
    if (whole_index < 0 || WHOLE_ARRAY_SIZE() < whole_index) {
      // ^ Not off by one, because of the partial value.
      return op (trash);
    } else if (whole_index < WHOLE_ARRAY_SIZE()) {
      return op (values_whole[whole_index]);
    } else if constexpr (PARTIAL_WIDTH() != 0) {
      return op (value_partial);
    }
  }

  template <typename OP>
  constexpr auto operateByWholeIndex (size_t whole_index, OP const & op) const {
    if (whole_index < 0 || WHOLE_ARRAY_SIZE() < whole_index) {
      // ^ Not off by one, because of the partial value.
      return op (trash);
    } else if (whole_index < WHOLE_ARRAY_SIZE()) {
      return op (values_whole[whole_index]);
    } else if constexpr (PARTIAL_WIDTH() != 0) {
      return op (value_partial);
    }
  }

  template <typename OP>
  void apply (OP const & op) {
    int idx_beg = 0;
    for (auto & v : values_whole) {
      op (idx_beg, REG_WIDTH(), v);
      idx_beg += REG_WIDTH();
    }
    if constexpr (0 != PARTIAL_WIDTH()) {
      op (idx_beg, PARTIAL_WIDTH(), value_partial);
      idx_beg += PARTIAL_WIDTH();
    }
  }

  template <typename OP>
  void apply (OP const & op) const {
    int idx_beg = 0;
    for (auto v : values_whole) {
      op (idx_beg, REG_WIDTH(), v);
      idx_beg += REG_WIDTH();
    }
    if constexpr (0 != PARTIAL_WIDTH()) {
      op (idx_beg, PARTIAL_WIDTH(), value_partial);
      idx_beg += PARTIAL_WIDTH();
    }
  }

  template <typename OP>
  void on_every_true (OP const & op) const {
    apply
      ( [&op](auto const idx_beg, auto const word_size, uregister_t x) {
          x = reverseBits (x);
          while (0 != x) {
            auto count_of_leading_zeroes = __builtin_clz (x);
            op (idx_beg + count_of_leading_zeroes);
            x &= ~(0x8000'0000UL >> count_of_leading_zeroes);
          }
        }
      );
  }

  template <typename OP>
  void on_every_false (OP const & op) const {
    apply
      ( [&op](auto const idx_beg, auto const word_size, uregister_t x) {
          x = (~x) & ((1UL << word_size) - 1);
          x = reverseBits (x);

          while (0 != x) {
            auto count_of_leading_zeroes = __builtin_clz (x);
            op (idx_beg + count_of_leading_zeroes);
            x &= ~(0x8000'0000UL >> count_of_leading_zeroes);
          }
        }
      );
  }
};


template <size_t WIDTH>
constexpr bool GetIndex (BitSet<WIDTH> const & x, size_t pos) {
  if (pos < 0 || WIDTH < pos) { return false; }

  auto const mask = uregister_t (1UL << (pos % REG_WIDTH()));
  auto const whole_index = pos / REG_WIDTH();
  
  return x.operateByWholeIndex (whole_index, [mask](auto y){ return 0 != (y & mask); });
}

template <size_t WIDTH>
constexpr void SetIndex (BitSet<WIDTH> & x, size_t pos) {
  if (pos < 0 || WIDTH < pos) { return; }

  auto const mask = uregister_t (1UL << (pos % REG_WIDTH()));
  auto const whole_index = pos / REG_WIDTH();
  
  x.operateByWholeIndex (whole_index, [mask](auto & y) { y |= mask; });
}

template <size_t WIDTH>
constexpr void ClearIndex (BitSet<WIDTH> & x, size_t pos) {
  if (pos < 0 || WIDTH < pos) { return; }

  auto const mask = uregister_t (1UL << (pos % REG_WIDTH()));
  auto const whole_index = pos / REG_WIDTH();
  
  x.operateByWholeIndex (whole_index, [mask](auto & y) { y &= ~mask; });
}

template <size_t WIDTH>
constexpr std::optional<size_t> GetFirstFalseIndex (BitSet<WIDTH> const & x) {
  std::optional <size_t> maybe_index;
  
  x.on_every_false( [&maybe_index] (auto index) {
    if (!maybe_index) { maybe_index = index; }
  } );
  return maybe_index;
}

template <size_t WIDTH>
constexpr std::optional<size_t> GetFirstTrueIndex (BitSet<WIDTH> const & x) {
  std::optional <size_t> maybe_index;
  
  x.on_every_true( [&maybe_index] (auto index) {
    if (!maybe_index) { maybe_index = index; }
  } );
  return maybe_index;
}

template <size_t WIDTH>
constexpr BitSet<WIDTH> operator~(BitSet<WIDTH> x) {
  x.apply (
    [] (auto const idx_beg, auto const word_size, uregister_t & ele) {
      auto flip_mask = (1UL << word_size) - 1;
      ele ^= flip_mask;
    }
  );
}

template <size_t WIDTH, typename LAMBDA_T>
void apply_binary_op_in_place(BitSet<WIDTH> & lhs, BitSet<WIDTH> rhs, LAMBDA_T const & op) {
  for (int idx=0; idx < BitSet<WIDTH>::WHOLE_ARRAY_SIZE(); ++idx) {
    op (lhs.values_whole[idx], rhs.values_whole[idx]);
  }
  if constexpr (BitSet<WIDTH>::PARTIAL_WIDTH()) {
    op (lhs.value_partial, rhs.value_partial);
  }
}

template <size_t WIDTH>
constexpr BitSet<WIDTH> operator& (BitSet<WIDTH> lhs, BitSet<WIDTH> rhs) {
  apply_binary_op_in_place (lhs, rhs, [](auto & l, auto r) {
    l &= r;
  });
  return lhs;
}

template <size_t WIDTH>
constexpr bool any (BitSet<WIDTH> const & lhs) {
  uint32_t check_nonzero = 0;
  lhs.apply (
    [&check_nonzero] (auto const idx_beg, auto const word_size, uregister_t ele) {
      check_nonzero |= ele;
    }
  );

  return check_nonzero != 0;
}

template <size_t WIDTH>
constexpr bool all (BitSet<WIDTH> const & lhs) {
  uregister_t check_zero = ~(uregister_t (0));
  lhs.apply (
    [&check_zero] (auto const idx_beg, auto const word_size, uregister_t ele) {
			ele |= ~((1UL << word_size) - 1);		//Set any bits unused to '1'.
    	check_zero &= ele;
    }
  );

	return uregister_t (0) == ~check_zero;
}


