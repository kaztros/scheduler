#pragma once
#include <tuple>
#include "register_building.hpp"
#include "typeify_global.hpp"
#include "usb_standard.hpp"
#include "stm32l41xxx_usb_protocols.hpp"

namespace stm32l41xxx {
namespace usb {

enum class endpoint_type_e : uint8_t {
  BULK = 0b00,
  CONTROL = 0b01,
  ISOCHRONOUS = 0b10,
  INTERRUPT = 0b11,
};

::usb::Messages::Descriptor::transfer_type_e as_transfer_type (endpoint_type_e x) {
  // Hey C++?  This sucks.  The "good safe part" sucks to write, and the allowance of the last part sucks.
  switch (x) {
    case endpoint_type_e::BULK:        return ::usb::Messages::Descriptor::transfer_type_e::BULK;
    case endpoint_type_e::CONTROL:     return ::usb::Messages::Descriptor::transfer_type_e::CONTROL;
    case endpoint_type_e::ISOCHRONOUS: return ::usb::Messages::Descriptor::transfer_type_e::ISOCHRONOUS;
    case endpoint_type_e::INTERRUPT:   return ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT;
  }
  return ::usb::Messages::Descriptor::transfer_type_e (0b100); //Such type-safety.  Many compiles.
}

endpoint_type_e as_endpoint_type (::usb::Messages::Descriptor::transfer_type_e x) {
  switch (x) {
    case ::usb::Messages::Descriptor::transfer_type_e::BULK:        return endpoint_type_e::BULK;
    case ::usb::Messages::Descriptor::transfer_type_e::CONTROL:     return endpoint_type_e::CONTROL;
    case ::usb::Messages::Descriptor::transfer_type_e::ISOCHRONOUS: return endpoint_type_e::ISOCHRONOUS;
    case ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT:   return endpoint_type_e::INTERRUPT;
  }
  return endpoint_type_e (0b100);
}


enum class transaction_direction_e : uint8_t {
  CTR_TX_IN = 0,    //
  CTR_RX_OUT = 1,   //
};


/// @brief The main control register for the USB module.
struct control_register_t
: public volatile_assign_by_raw <control_register_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <bool,  0,  1>> fres;     //Force USB Reset
    BitfieldMember <uint16_t, BFE <bool,  1,  1>> pdwn;     //Power down
    BitfieldMember <uint16_t, BFE <bool,  2,  1>> lp_mode;  //Low-power mode
    BitfieldMember <uint16_t, BFE <bool,  3,  1>> fpsusp;   //Force Suspend
    BitfieldMember <uint16_t, BFE <bool,  4,  1>> resume;   //Resume request
    BitfieldMember <uint16_t, BFE <bool,  5,  1>> l1resume; //LPM L1 request
    BitfieldMember <uint16_t, BFE <bool,  6,  1>> reserved_bit_6;
    BitfieldMember <uint16_t, BFE <bool,  7,  1>> l1reqm;   //LPM L1 state request interrupt mask
    BitfieldMember <uint16_t, BFE <bool,  8,  1>> esofm;    //Expected start of frame interrupt mask
    BitfieldMember <uint16_t, BFE <bool,  9,  1>> sofm;     //Start of frame interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 10,  1>> resetm;   //USB reset interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 11,  1>> suspm;    //Suspend mode interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 12,  1>> wkupm;    //Wakeup interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 13,  1>> errm;     //Error interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 14,  1>> pmaovrm;  //Packet memory area over/under-run interrupt mask
    BitfieldMember <uint16_t, BFE <bool, 15,  1>> ctrm;     //Correct transfer interrupt mask
  };
};

struct interrupt_status_register_t
: public volatile_assign_by_raw <interrupt_status_register_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint8_t,  0,  4>> ep_id; //Endpoint generating the ctr interrupt.
    BitfieldMember <uint16_t, BFE <transaction_direction_e,  4,  1, transaction_direction_e::CTR_RX_OUT>> dir;     //Direction of transaction
    BitfieldMember <uint16_t, BFE <uint8_t,  5,  2>> reserved_bits_5_2;
    BitfieldMember <uint16_t, BFE <bool,  7,  1>> l1req;    //LPM L1 state request
    BitfieldMember <uint16_t, BFE <bool,  8,  1>> esof;     //Start of frame expected
    BitfieldMember <uint16_t, BFE <bool,  9,  1>> sof;      //Start of frame
    BitfieldMember <uint16_t, BFE <bool, 10,  1>> reset;    //USB reset requested
    BitfieldMember <uint16_t, BFE <bool, 11,  1>> susp;     //Suspend mode requested
    BitfieldMember <uint16_t, BFE <bool, 12,  1>> wkup;     //Wakeup detected
    BitfieldMember <uint16_t, BFE <bool, 13,  1>> err;      //Error detected (can be ignored :/)
    BitfieldMember <uint16_t, BFE <bool, 14,  1>> pmaovr;   //Packet Memory Area overrun/underrun
    BitfieldMember <uint16_t, BFE <bool, 15,  1>> suspm;    //Correct transfer (good things)
  };
};

struct device_address_register_t
: public volatile_assign_by_raw <device_address_register_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint8_t, 0,  7>> address; //Endpoint generating the ctr interrupt.
    BitfieldMember <uint16_t, BFE <bool,    7,  1>> ef;      //Address enabled.
  };
};

struct endpoint_register_t
: public volatile_assign_by_raw <endpoint_register_t>
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
    BitfieldMember <uint16_t, BFE <uint16_t, 12,  2>> stat_rx;
    BitfieldMember <uint16_t, BFE <uint16_t, 14,  1>> dtog_rx;
    BitfieldMember <uint16_t, BFE <uint16_t, 15,  1>> ctr_rx;
  };
};

/// @brief Describes the offset of the USB packet buffers in packet-memory.
struct btable_register_t
: public volatile_assign_by_raw <btable_register_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint16_t,  0, 16>> addr; //Must be 8-byte aligned.
  };
};

/// @brief Low-Power-Mode control and status
struct lpm_control_and_status_register_t
: public volatile_assign_by_raw <lpm_control_and_status_register_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <bool,  0, 1>> lpmen;   //LPM support enabled?
    BitfieldMember <uint16_t, BFE <bool,  1, 1>> lpmack;  //LPM Token acknowledge enabled?
    BitfieldMember <uint16_t, BFE <bool,  2, 1>> reserved_bit_2;
    BitfieldMember <uint16_t, BFE <bool,  3, 1>> remwake; //bRemoteWake value of last ACKed LPM token
    BitfieldMember <uint16_t, BFE <bool,  4, 4>> besl;    //besl value of last ACKed LPM token
    BitfieldMember <uint16_t, BFE <uint8_t,  8, 8>> reserved_bits_8_8;
  };
};

/// @brief Packet buffer address (relative to BTABLE register)
struct buffer_offset_t
: public volatile_assign_by_raw <buffer_offset_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint16_t,  0, 16>> addr;
  };
};

/// @brief Packet buffer length, max receivable length.
struct buffer_count_t
: public volatile_assign_by_raw <buffer_count_t>
{
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint16_t,  0, 10>> byte_count;
    BitfieldMember <uint16_t, BFE <uint16_t, 10,  5>> num_block;
    BitfieldMember <uint16_t, BFE <uint16_t, 15,  1>> bl_size;
  };
};

template <typename T>
struct dw_aligned
: public T
{
  static_assert(2 == sizeof(T));  //fragile implementation.  BE NICE TO IT.
private:
  uint16_t _dw_aligned_space;
};

static_assert (4 == sizeof(dw_aligned<endpoint_register_t>));

struct device_registers_t {
  dw_aligned <endpoint_register_t> ep [8];
  uint8_t _reserved_20_20 [0x20];
  dw_aligned <control_register_t> cnt;
  dw_aligned <interrupt_status_register_t> ist;
  uint8_t _reserved_48_04 [0x04]; //Frame number, unused for now.
  dw_aligned <device_address_register_t> dadd;
  dw_aligned <btable_register_t> btable;
  uint8_t _reserved_54_08 [0x08]; //Low-power control, battery charging detector.
};


struct buffer_span_t
: public buffer_offset_t
, public buffer_count_t
{
  buffer_span_t () = default;
  buffer_span_t (buffer_span_t const & rhs) = default;

  /// @brief Force volatile copies to segment into halfword copies.
  buffer_span_t (buffer_span_t const volatile & rhs);
};

using buffer_spans_t = std::array <buffer_span_t, 2>;

/* Make sure these types are nothing more than their raw-equivalents. */
static_assert (sizeof(control_register_t) == sizeof(control_register_t::_raw));
static_assert (sizeof(interrupt_status_register_t) == sizeof(interrupt_status_register_t::_raw));
static_assert (sizeof(device_address_register_t) == sizeof(device_address_register_t::_raw));
static_assert (sizeof(endpoint_register_t) == sizeof(endpoint_register_t::_raw));
static_assert (sizeof(btable_register_t) == sizeof(btable_register_t::_raw));
static_assert (sizeof(lpm_control_and_status_register_t) == sizeof(lpm_control_and_status_register_t::_raw));
static_assert (sizeof(buffer_offset_t) == sizeof(buffer_offset_t::_raw));
static_assert (sizeof(buffer_count_t) == sizeof(buffer_count_t::_raw));
static_assert (8 == sizeof(buffer_spans_t));

/* Make sure that these types match their data-sheet sizes: */
static_assert (0x5C == sizeof(device_registers_t));

template <std::size_t N = 1024, std::size_t BTABLE_OFFSET = 0x0000>
struct h_array {
  static_assert (BTABLE_OFFSET % 8 == 0, "BTABLE offset must be 8-byte aligned.");

  union {
    std::array <uint8_t volatile,  N  > b;
    std::array <uint16_t volatile, N/2> h;
    struct {
      uint8_t _btable_spacing [BTABLE_OFFSET];  //Can't make this private.
      weave_volatile_t <buffer_spans_t[8]> buffer_spanses;
    };
  };
};


extern device_registers_t volatile usb1;     // @ 0x4000'6800, APB1
extern h_array <1024> usb1_sram;             // @ 0x4000'6C00, APB1

}//end namespace usb
}//end namespace stm32l41xxx
