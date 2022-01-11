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



template <device_registers_t volatile & USB, std::size_t PMA_SIZE, h_array <PMA_SIZE> & PMA_SPACE, std::size_t btable_offset = 0x0000>
struct device_registers_with_hardcoded_btable {
  static constexpr auto volatile * usb = USB;
  //gotta find a way to provide some pointers to buffer descriptions.
  static constexpr auto volatile * buffer_spans_table = reinterpret_cast <buffer_spans_table_t volatile *> (&(PMA_SPACE.b[btable_offset])); //constexpr doesn't like reinterpret_cast
};

/* Make sure these types are nothing more than their raw-equivalents. */
static_assert (sizeof(endpoint_register_bidirectional_t) == sizeof(endpoint_register_bidirectional_t::_raw));
static_assert (sizeof(endpoint_register_unidirectional_rx_t) == sizeof(endpoint_register_unidirectional_rx_t::_raw));
static_assert (sizeof(endpoint_register_unidirectional_tx_t) == sizeof(endpoint_register_unidirectional_tx_t::_raw));

/* Make sure these sequences are packed correctly. */
static_assert(offsetof(usb_buffer_registers_t, byte_count) == sizeof(buffer_offset_t), "Compiler generates incorrect memory layout.");

#if 0 //This really ought to be two different functions.  But also like:
//  pma_buffer_t is better suited to what I want to do, with less cycles.
std::tuple <uint16_t*, decltype(buffer_count_t::byte_count) &> as_native_memory_address (btable_register_t btable /* const-out */, usb_buffer_registers_t buffer_desc);
#endif//0


}//end namespace usb
}//end namespace stm32l41xxx
