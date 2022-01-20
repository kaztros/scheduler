#pragma once
#include <tuple>
#include "stm32l41xxx_usb_adaptors_tagged.hpp"

namespace stm32l41xxx {
namespace usb {

///The endpoint-control-registers aren't trivially writeable.  They're designed
///for transaction-based modifications based on write.
///e.g. writing bits to some portion of the register results in a &= operation.
///  or as a ^= operation.
///
///Naively, one would try to observe the current register, and build the desired
///transaction-word through computation.
///
///I try to build the transaction word based on type-data (constexpr template
///parameters) and application state.
///  e.g. if we're in the routine to read buffers, then we're
///  1) Likely operating, so stat_rx must be NAK (0b10) right now.  And using a
///     ^= (0b01) transaction will move us to VALID (0b11) so that the device
///     can receive another USB data packet.
///
///  2) Unlikely disabled, so stat_rx is DISABLED (0b00) right now.  And using a
///     ^= (0b01) transaction will move us to STALL (0b01).  Which is inert.
///
///Ideally, we always hit #1, and the codepath that generates these transactions
///is only executed based on one-read of the endpoint-control-register, with USB
///HW specs guaranteeing invariance until our transaction completes (until the
///rx-buffer is released).

///Some of these verbs are called "force", and they're The Naive Approach.
///They're intended for initialization/deinitialization routines, which engage
///with more unknowns / slop.

// nop          - Needs type info, or the current register state.
// clear_ctr_rx - Needs nop's output.
// clear_ctr_tx - Needs nop's output.
// release_rx   - Needs nop's output, potentially inert if app-state is misread.
// commit_tx    - Needs nop's output, potentially inert if app-state is misread.

// force_stat_rx - Needs the current register's state.
// force_stat_tx - Needs the current register's state.

//The ops are made to be composed (although, whether that's left-to-right
//, or inner-to-outer is

//namespace verbs {

template <typename T>
using trans_t = ::transaction_t<T>;

template <typename...TAGS>
using trans_bi_t =
  ::transaction_t <endpoint_register_bidir_tagged_t <TAGS...>>;

template <typename...TAGS>
using trans_out_t =
  ::transaction_t <endpoint_register_out_only_tagged_t <TAGS...>>;

template <typename...TAGS>
using trans_in_t =
  ::transaction_t <endpoint_register_in_only_tagged_t <TAGS...>>;

///@brief Naively reverse-calculate transaction for desired stat_rx value.
///@param T Endpoint Control Register Tagged Type
template <typename T>
trans_t <T> force_stat_rx (trans_t <T> x, endpoint_status_e stat) {
  x.transaction.stat_rx = x.original.stat_rx ^ stat;
  return x;
}

///@brief Naively reverse-calculate transaction for desired stat_tx value.
///@param T Endpoint Control Register Tagged Type
template <typename T>
trans_t <T> force_stat_tx (trans_t <T> x, endpoint_status_e stat) {
  x.transaction.stat_tx = x.original.stat_tx ^ stat;
  return x;
}

template <typename EP_CTL_TAGGED_T>
constexpr auto _nop_tagged (EP_CTL_TAGGED_T result) {
  endpoint_register_setup_t reg;  //Throw-away reg used for type-derived data.
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  result._raw = (m(reg.ctr_rx) | m(reg.ctr_tx)) & ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
  result.address = EP_CTL_TAGGED_T::ADDRESS;
  result.ep_type = as_endpoint_type (EP_CTL_TAGGED_T::TRANSFER_TYPE);
  result.ep_kind = std::is_base_of_v<endpoint_register_out_only_t, EP_CTL_TAGGED_T>
   || std::is_base_of_v<endpoint_register_in_only_t, EP_CTL_TAGGED_T>;
  //^ Set DBLBUF accordingly.
  return result;
}

template <typename...TAGS>
constexpr auto nop (endpoint_register_bidir_tagged_t <TAGS...> dummy) {
  return _nop_tagged (dummy);
}

template <typename...TAGS>
constexpr auto nop (endpoint_register_out_only_tagged_t <TAGS...> dummy) {
  return _nop_tagged (dummy);
}

template <typename...TAGS>
constexpr auto nop (endpoint_register_in_only_tagged_t <TAGS...> dummy) {
  return _nop_tagged (dummy);
}

/*
constexpr auto nop (auto reg_moded) {
  endpoint_register_setup_t & reg = reg_moded;
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  reg._raw |= m(reg.ctr_rx) | m(reg.ctr_tx);
            //^ These fields don't respond to 1's.
  reg._raw &= ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
            //^ These fields don't respond to 0's.
  return reg_moded;
}*/

template <typename...TAGS>
constexpr auto clear_ctr_rx (trans_bi_t <TAGS...> reg)
{ reg.transaction.ctr_rx = 0; return reg; }

template <typename...TAGS>
constexpr auto clear_ctr_rx (trans_out_t <TAGS...> reg)
{ reg.transaction.ctr_rx = 0; return reg; }

template <typename...TAGS>
constexpr auto clear_ctr_tx (trans_bi_t <TAGS...> reg)
{ reg.transaction.ctr_tx = 0; return reg; }

template <typename...TAGS>
constexpr auto clear_ctr_tx (trans_in_t <TAGS...> reg)
{ reg.transaction.ctr_tx = 0; return reg; }

//constexpr auto clear_ctr_tx (auto reg) { reg.ctr_tx = 0; return reg; }

template <typename...TAGS>
constexpr auto release_rx_buffer (trans_bi_t <TAGS...> reg) {
  reg.transaction.stat_rx = endpoint_status_e::NAK ^ endpoint_status_e::VALID;
  return reg;
}

template <typename...TAGS>
constexpr auto release_rx_buffer (trans_out_t <TAGS...> reg)
{ reg.transaction.sw_buf = 1; return reg; }

template <typename...TAGS>
constexpr auto commit_tx_buffer (trans_bi_t <TAGS...> reg) {
  reg.transaction.stat_tx = endpoint_status_e::NAK ^ endpoint_status_e::VALID;
  return reg;
}

template <typename...TAGS>
constexpr auto commit_tx_buffer (trans_in_t <TAGS...> reg)
{ reg.transaction.sw_buf = 1; return reg; }

/*

constexpr auto release_rx_buffer (auto reg) {
  if constexpr (std::is_base_of_v <endpoint_register_out_only_t, decltype(reg)>) {
    reg.sw_buf = 1;   // Toggle sw_buf.
  } else if constexpr (std::is_base_of_v <endpoint_register_bidir_t, decltype(reg)>) {
    reg.stat_rx = static_cast <endpoint_status_e> (0b01);
    // Toggles 0b10 (NAK) into 0b11 (VALID)
    // or 0b00 (DISABLED) into 0b01 (STALL)
  } else {
    static_assert ("Unhandled case.");
  }
  return reg;
}
 
*/

/*
constexpr auto commit_tx_buffer (auto reg) {
  if constexpr (std::is_base_of_v <endpoint_register_in_only_t, decltype(reg)>) {
    reg.sw_buf = 1; //
  } else if constexpr (std::is_base_of_v <endpoint_register_bidir_t, decltype(reg)>) {
    reg.stat_tx = static_cast <endpoint_status_e> (0b01);
    // Toggles 0b10 (NAK) into 0b11 (VALID)
    // or 0b00 (DISABLED) into 0b01 (STALL)
  } else {
    static_assert ("Unhandled case.");
  }
  return reg;
}*/

constexpr std::size_t get_application_tx_buffer_index (endpoint_register_in_only_t ep_ctl) {
  return ep_ctl.sw_buf;
}

constexpr std::size_t get_application_rx_buffer_index (endpoint_register_out_only_t ep_ctl) {
  return ep_ctl.sw_buf;
}

constexpr std::size_t get_application_tx_buffer_index (endpoint_register_bidir_t ctl) { return 0; }

constexpr std::size_t get_application_rx_buffer_index (endpoint_register_bidir_t ctl) { return 1; }


//}//end namespce verbs
}//end namespace usb
}//end namespace stm32l41xxx

template <typename...TAGS>
struct transaction_t
  <stm32l41xxx::usb::endpoint_register_bidir_tagged_t <TAGS...>>
{
  using REGISTER_T = stm32l41xxx::usb::endpoint_register_bidir_tagged_t <TAGS...>;
  REGISTER_T transaction;
  REGISTER_T original;

  transaction_t (REGISTER_T snapshot) noexcept
  : transaction (stm32l41xxx::usb::nop(snapshot))
  , original (snapshot)
  { }
};

template <typename...TAGS>
struct transaction_t
  <stm32l41xxx::usb::endpoint_register_out_only_tagged_t <TAGS...>>
{
  using REGISTER_T = stm32l41xxx::usb::endpoint_register_out_only_tagged_t <TAGS...>;
  REGISTER_T transaction;
  REGISTER_T original;

  transaction_t (REGISTER_T snapshot) noexcept
  : transaction (stm32l41xxx::usb::nop(snapshot))
  , original (snapshot)
  { }
};


template <typename...TAGS>
struct transaction_t
  <stm32l41xxx::usb::endpoint_register_in_only_tagged_t <TAGS...>>
{
  using REGISTER_T = stm32l41xxx::usb::endpoint_register_in_only_tagged_t <TAGS...>;
  REGISTER_T transaction;
  REGISTER_T original;

  transaction_t (REGISTER_T snapshot) noexcept
  : transaction (stm32l41xxx::usb::nop(snapshot))
  , original (snapshot)
  { }
};
