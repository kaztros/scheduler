#include <span>
#include "stm32l41xxx_usb.hpp"
/// @brief This file contains register-type-sequences to mimic the actual layout in memory.
/// @brief This file contains disambiguated register-types, for multi-mode registers.

namespace stm32l41xxx {
namespace usb {

/// @brief Restricted endpoint_register_t, that works as a single-buffered bi-directional register.
struct endpoint_register_bidirectional_t
: public endpoint_register_t
, public volatile_assign_by_raw <endpoint_register_bidirectional_t>
{
  using volatile_assign_by_raw <endpoint_register_bidirectional_t> ::operator=;
};

/// @brief Restricted endpoint_register_t, that works as a double-buffered RX register.
struct endpoint_register_unidirectional_rx_t
: public volatile_assign_by_raw <endpoint_register_unidirectional_rx_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint16_t,  0,  4>> address;
    BitfieldMember <uint16_t, BFE <uint16_t,  6,  1>> sw_buf;
    BitfieldMember <uint16_t, BFE <uint16_t,  8,  1>> ep_kind;
    BitfieldMember <uint16_t, BFE <endpoint_type_e,  9,  2, endpoint_type_e::INTERRUPT>> ep_type;
    BitfieldMember <uint16_t, BFE <uint16_t, 11,  1>> setup;
    BitfieldMember <uint16_t, BFE <uint16_t, 12,  2>> stat_rx;
    BitfieldMember <uint16_t, BFE <uint16_t, 14,  1>> dtog_rx;
    BitfieldMember <uint16_t, BFE <uint16_t, 15,  1>> ctr_rx;
  };
  
  /// @brief Pretend to be our base-register-type.
  operator endpoint_register_t & ();
};

/// @brief Restricted endpoint_register_t, that works as a double-buffered RX register.
struct endpoint_register_unidirectional_tx_t
: public volatile_assign_by_raw <endpoint_register_unidirectional_tx_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint16_t,  0,  4>> address;
    BitfieldMember <uint16_t, BFE <uint16_t,  4,  2>> stat_tx;
    BitfieldMember <uint16_t, BFE <uint16_t,  6,  1>> dtog_tx;
    BitfieldMember <uint16_t, BFE <uint16_t,  7,  1>> ctr_tx;
    BitfieldMember <uint16_t, BFE <uint16_t,  8,  1>> ep_kind;
    BitfieldMember <uint16_t, BFE <endpoint_type_e,  9,  2, endpoint_type_e::INTERRUPT>> ep_type;
    BitfieldMember <uint16_t, BFE <uint16_t, 11,  1>> setup;
    BitfieldMember <uint16_t, BFE <uint16_t, 14,  1>> sw_buf;
  };
  
  /// @brief Pretend to be our base-register-type.
  operator endpoint_register_t & ();
};


/* Make sure these types are nothing more than their raw-equivalents. */
static_assert (sizeof(endpoint_register_bidirectional_t) == sizeof(endpoint_register_bidirectional_t::_raw));
static_assert (sizeof(endpoint_register_unidirectional_rx_t) == sizeof(endpoint_register_unidirectional_rx_t::_raw));
static_assert (sizeof(endpoint_register_unidirectional_tx_t) == sizeof(endpoint_register_unidirectional_tx_t::_raw));

/* Make sure these sequences are packed correctly. */
static_assert(offsetof(buffer_span_t, byte_count) == sizeof(buffer_offset_t), "Compiler generates incorrect memory layout.");

template <typename TAGGED>
auto as_native_buffers_helper (TAGGED x) {
    if constexpr (std::is_base_of_v <endpoint_register_bidirectional_t, TAGGED>) {
    return reinterpret_cast
    < std::tuple
      < pma_ram <TAGGED::TX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>>
      , pma_ram <TAGGED::RX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>>
      > *
    > (0);
  } else if constexpr (std::is_base_of_v <endpoint_register_unidirectional_rx_t, TAGGED>) {
    return reinterpret_cast
    < pma_ram <TAGGED::RX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>> (*) [2]>
    (0);
  } else if constexpr (std::is_base_of_v<endpoint_register_unidirectional_tx_t, TAGGED>) {
    return reinterpret_cast
    < pma_ram <TAGGED::TX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>> (*) [2]>
    (0);
  } else {
    //static_assert (false, "Unhandled case.");
    return reinterpret_cast <void*> (0);
  }
};

template <typename TAGGED>
using as_native_buffers_t = std::remove_pointer_t <decltype (as_native_buffers_helper (TAGGED ()))>;

template
<std::size_t n_bytes, std::size_t btable_offset, typename EP_MODED_CTL_Ts>
struct btable_space_t;


template
< std::size_t n_bytes
, std::size_t btable_offset
, typename ... EP_MODED_CTL_Ts
>
struct btable_space_t <n_bytes, btable_offset, std::tuple <EP_MODED_CTL_Ts...>> {
  static_assert (btable_offset % 8 == 0, "BTABLE offset must be 8-byte aligned.");
  
  template <typename T>
  using map_buffer_t = as_native_buffers_t <T>;
  
private:
  uint8_t reserved_space [btable_offset];
  //^ Not a std::array, becuase 0-length std::array takes up one byte >:[.
public:
  buffer_spans_t ctlses [sizeof...(EP_MODED_CTL_Ts)];
  std::tuple <map_buffer_t <EP_MODED_CTL_Ts> ...> datases;
  //Not sure if datases are offset by BTABLE.offset.  Like it might be cool to
  //allocate the ctlses at the end of the USB SRAM.
  
  template <std::size_t ep_ctl_idx, std::size_t buffer_idx>
  constexpr static buffer_span_t default_ctl () {
    using T = decltype (btable_space_t::datases);
    using T_bufs = tuple_element_t <ep_ctl_idx, T>;
    using T_buf = tuple_element_t <buffer_idx, T_bufs>;
    
    constexpr std::size_t max_size = T_buf::SIZE_IN_BYTES;
    std::size_t struct_offset = offsetof (btable_space_t, datases);
    std::size_t tuple_offset = ::tuple_element_offset <ep_ctl_idx, T> ::value;
    std::size_t buffer_offset = ::tuple_element_offset <buffer_idx, T_bufs> ::value;
    
    buffer_span_t result;
    result.byte_count = 0;
    result.addr = struct_offset + tuple_offset + buffer_offset;

    if constexpr (0 == (max_size % 2) && max_size < 64) {
      result.bl_size = 0;
      result.num_block = max_size / 2;
    } else if constexpr (0 == (max_size % 32) && max_size <= 1024) {
      result.bl_size = 1;
      result.num_block = (max_size / 32) - 1;
    } else {
      static_assert ("Unhandled length type.");
    }
    
    return result;
  }
  
  static_assert (sizeof(ctlses) + sizeof(datases) < n_bytes);
  
  void init_hw () {
    auto write_default_buffer_ctls =
    [&] <std::size_t ep_ctl_idx>
    (std::integral_constant <std::size_t, ep_ctl_idx>) {
      ctlses [ep_ctl_idx][0] = default_ctl <ep_ctl_idx, 0> ();
      ctlses [ep_ctl_idx][1] = default_ctl <ep_ctl_idx, 1> ();
    };
    
    [&] <std::size_t...idx> (std::index_sequence <idx...>) {
      ((write_default_buffer_ctls (std::integral_constant <std::size_t, idx> ())), ...);
    } (std::index_sequence_for <EP_MODED_CTL_Ts...> ());
  }
  

  
};



template <std::size_t size, typename T>
auto span ( pma_ram <size, T> & data
          , buffer_count_t count) {
  return std::span (data.b, count.byte_count);
}

template <std::size_t size, typename T>
auto max_span ( pma_ram <size, T> & data) {
  return std::span (data.b);
}

template
< std::size_t ep_ctl_idx, std::size_t buffer_idx
, std::size_t n_bytes, std::size_t btable_offset, typename ... EP_MODED_CTL_Ts
>
auto span (btable_space_t <n_bytes, btable_offset, std::tuple<EP_MODED_CTL_Ts...> > & _this) {
  return span
  ( getm <ep_ctl_idx, buffer_idx> (_this.datases)
  , getm <ep_ctl_idx, buffer_idx> (_this.ctlses)
  );
}

template
< std::size_t ep_ctl_idx, std::size_t buffer_idx
, std::size_t n_bytes, std::size_t btable_offset, typename ... EP_MODED_CTL_Ts
>
auto max_span (btable_space_t <n_bytes, btable_offset, std::tuple<EP_MODED_CTL_Ts...> > & _this) {
  return max_span (getm <ep_ctl_idx, buffer_idx> (_this.datases));
}


template <typename BUFFER_SPANS_REF_T, std::size_t offset, std::size_t max_size>
struct sram_range {
  static constexpr auto MAX_SIZE = max_size;
  
  static constexpr buffer_span_t default_value () {
    buffer_span_t result;

    result.byte_count = 0;
    result.addr = offset;

    if constexpr (0 == (max_size % 2) && max_size < 64) {
      result.bl_size = 0;
      result.num_block = max_size / 2;
    } else if constexpr (0 == (max_size % 32) && max_size <= 1024) {
      result.bl_size = 1;
      result.num_block = (max_size / 32) - 1;
    } else {
      static_assert ("Unhandled length type.");
    }
    
    return result;
  }
  
  void init_hw() {
    *BUFFER_SPANS_REF_T() = default_value ();
  }
};
/*
template
< std::size_t ram_size
, std::size_t btable_offset
, typename BUFFER_SPANS_REF_T
, std::size_t sram_offset
, std::size_t max_size
>
std::span <volatile uint8_t> span
( h_array <ram_size, btable_offset> & usb_sram
, sram_range <BUFFER_SPANS_REF_T, sram_offset, max_size> range
) {
  buffer_count_t regs_copy = raw_snapshot_of <buffer_count_t> (* BUFFER_SPANS_REF_T ());
                          // ^ avoid copying extra registers.
  return { & (usb_sram.b) [btable_offset + sram_offset], regs_copy.byte_count };
}

template
< std::size_t ram_size
, std::size_t btable_offset
, typename BUFFER_SPANS_REF_T
, std::size_t sram_offset
, std::size_t max_size
>
auto max_span
( h_array <ram_size, btable_offset> & usb_sram
, sram_range <BUFFER_SPANS_REF_T, sram_offset, max_size> range
) {
  volatile uint8_t * start = & (usb_sram.b) [btable_offset + sram_offset];
  return std::span <volatile uint8_t, max_size> (start, start + max_size);
}
 */

}//end namespace usb
}//end namespace stm32l41xxx
