#pragma once
#include "stm32l41xxx_usb_adaptors_tagged.hpp"

namespace stm32l41xxx {
namespace usb {

template <typename...TAGS>
constexpr auto endpoint_nop (endpoint_register_bidir_tagged_t <TAGS...> dummy) {
  endpoint_register_setup_t reg;  //Throw-away reg used for type-derived data.
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  
  decltype(dummy) result;
  result._raw = (m(reg.ctr_rx) | m(reg.ctr_tx)) & ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
  result.address = decltype(dummy)::ADDRESS;
  result.ep_type =as_endpoint_type (decltype(dummy)::TRANSFER_TYPE);
  result.ep_kind = 0; //DBLBUF indicator.
  return result;
}

template <typename...TAGS>
constexpr auto endpoint_nop (endpoint_register_out_only_tagged_t <TAGS...> dummy) {
  endpoint_register_setup_t reg;  //Throw-away reg used for type-derived data.
  decltype(dummy) result;
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  result._raw = (m(reg.ctr_rx) | m(reg.ctr_tx)) & ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
  result.address = decltype(dummy)::ADDRESS;
  result.ep_type = as_endpoint_type (decltype(dummy)::TRANSFER_TYPE);
  result.ep_kind = 1; //DBLBUF indicator.
  return result;
}

template <typename...TAGS>
constexpr auto endpoint_nop (endpoint_register_in_only_tagged_t <TAGS...> dummy) {
  endpoint_register_setup_t reg;  //Throw-away reg used for type-derived data.
  decltype(dummy) result;
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  result._raw = (m(reg.ctr_rx) | m(reg.ctr_tx)) & ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
  result.address = decltype(dummy)::ADDRESS;
  result.ep_type = as_endpoint_type (decltype(dummy)::TRANSFER_TYPE);
  result.ep_kind = 1; //DBLBUF indicator.
  return result;
}

constexpr auto endpoint_nop (auto reg_moded) {
  endpoint_register_setup_t & reg = reg_moded;
  auto m = [reg] (auto x) { return mask <decltype(reg._raw)> (typename decltype(x)::ELEMENT_T ()); };
  reg._raw |= m(reg.ctr_rx) | m(reg.ctr_tx);
            //^ These fields don't respond to 1's.
  reg._raw &= ~( m(reg.dtog_rx) | m(reg.stat_rx) | m(reg.dtog_tx) | m(reg.stat_tx) );
            //^ These fields don't respond to 0's.
  return reg_moded;
}

constexpr auto clear_ctr_rx (auto reg) { reg.ctr_rx = 0; return reg; }

constexpr auto clear_ctr_tx (auto reg) { reg.ctr_tx = 0; return reg; }

template <typename EP_CTL_T>
constexpr EP_CTL_T release_rx_buffer (EP_CTL_T reg) {
  if constexpr (std::is_base_of_v <endpoint_register_out_only_t, EP_CTL_T>) {
    reg.sw_buf = 1;   // Toggle sw_buf.
  } else if constexpr (std::is_base_of_v <endpoint_register_bidir_t, EP_CTL_T>) {
    reg.stat_rx = 0b01; // Toggles 0b10 (NAK) into 0b11 (VALID)
  } else {
    static_assert ("Unhandled case.");
  }
  return reg;
}

constexpr auto commit_tx_buffer (auto reg) {
  static_assert (std::is_base_of_v <endpoint_register_in_only_t, decltype(reg)>);
  reg.sw_buf = 1;
  return reg;
}

constexpr endpoint_register_bidir_t commit_tx_buffer (endpoint_register_bidir_t reg) {
  reg.stat_tx = 0b01; // Toggles 0b10 (NAK) into 0b11 (VALID)
  return reg;
}

constexpr std::size_t get_application_tx_buffer_index (endpoint_register_in_only_t ep_ctl) {
  return ep_ctl.sw_buf;
}

constexpr std::size_t get_application_rx_buffer_index (endpoint_register_out_only_t ep_ctl) {
  return ep_ctl.sw_buf;
}

constexpr size_t get_application_tx_buffer_index (endpoint_register_bidir_t ctl) { return 0; }

constexpr size_t get_application_rx_buffer_index (endpoint_register_bidir_t ctl) { return 1; }


}//end namespace usb
}//end namespace stm32l41xxx
