#include <span>
#include "type_traits_kaz.hpp"
#include "stm32l41xxx_usb.hpp"
/// @brief This file contains register-type-sequences to mimic the actual layout in memory.
/// @brief This file contains disambiguated register-types, for multi-mode registers.

namespace stm32l41xxx {
namespace usb {

/* Make sure these sequences are packed correctly. */
static_assert(offsetof(buffer_span_t, byte_count) == sizeof(buffer_offset_t), "Compiler generates incorrect memory layout.");

template <typename TAGGED>
auto as_native_buffers_helper (TAGGED x) {
    if constexpr (std::is_base_of_v <endpoint_register_bidir_t, TAGGED>) {
    return reinterpret_cast
    < std::tuple
      < pma_ram <TAGGED::TX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>>
      , pma_ram <TAGGED::RX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>>
      > *
    > (0);
  } else if constexpr (std::is_base_of_v <endpoint_register_out_only_t, TAGGED>) {
    return reinterpret_cast
    < pma_ram <TAGGED::RX_BUFFER_SIZE, copy_cv_t <TAGGED, uint16_t>> (*) [2]>
    (0);
  } else if constexpr (std::is_base_of_v<endpoint_register_in_only_t, TAGGED>) {
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
  
private:
  uint8_t reserved_space [btable_offset];
  //^ Not a std::array, becuase 0-length std::array takes up one byte >:[.
public:
  volatile_but_raw_c <buffer_spans_t> ctlses [sizeof...(EP_MODED_CTL_Ts)];
  std::tuple <as_native_buffers_t <EP_MODED_CTL_Ts> ...> datases;
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
  
  static_assert (btable_offset + sizeof(ctlses) + sizeof(datases) <= n_bytes);
  
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
  if constexpr (0 == size) {  // C++ _hates_ zero-length arrays.
    return std::span <uint8_t, 0> (& data.b [0], & data.b [0]);
  } else {
    return std::span (data.b);
  }
  
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


}//end namespace usb
}//end namespace stm32l41xxx
