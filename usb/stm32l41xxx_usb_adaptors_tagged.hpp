#pragma once
#include <span>

#include "register_building.hpp"
#include "stm32l41xxx_usb_adaptors.hpp"
/// @brief This file contains register-type-sequences to mimic the actual layout in memory.
/// @brief This file contains disambiguated register-types, for multi-mode registers.

namespace stm32l41xxx {
namespace usb {

/*----------------------------------------------------------------------------*/
template <typename...TAGS>
struct endpoint_register_bidir_tagged_t
: public endpoint_register_bidir_t
, public volatile_assign_by_raw <endpoint_register_bidir_tagged_t <TAGS...> >
, TAGS ...
{
  //g++ needs this apparently:
  using volatile_assign_by_raw <endpoint_register_bidir_tagged_t <TAGS...> >::operator=;
};


template <typename...TAGS>
struct endpoint_register_out_only_tagged_t
: public endpoint_register_out_only_t
, public volatile_assign_by_raw <endpoint_register_out_only_tagged_t <TAGS...> >
, TAGS ...
{
  using volatile_assign_by_raw <endpoint_register_out_only_tagged_t <TAGS...> >::operator=;
};


template <typename...TAGS>
struct endpoint_register_in_only_tagged_t
: public endpoint_register_in_only_t
, public volatile_assign_by_raw <endpoint_register_in_only_tagged_t <TAGS...> >
, TAGS ...
{
  using volatile_assign_by_raw <endpoint_register_in_only_tagged_t <TAGS...> >::operator=;
};

/*----------------------------------------------------------------------------*/

// Note: (tx_buffer_size, rx_buffer_size) are used as parameters, but they
// should be allocators instead.  But like: dynamic allocation only makes sense
// for tx, and is dubious for rx.  And OTG devices can't even dynamically
// allocate.  So like, we stick with these dumb parameters for now.

//And also:
// Isochronous endpoints can't be single-buffered (bidirectional)
// Control endpoints cant be double buffered

template
< uint8_t address
, ::usb::Messages::Descriptor::transfer_type_e transfer_type
, std::size_t rx_buffer_size
, std::size_t tx_buffer_size
, bool prefer_single_buffer = false
, typename ENABLE = void
>
struct as_ep_moded_register_type
{
  using type = std::tuple
  < endpoint_register_bidir_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_rx_buffer_size_tag <rx_buffer_size>
    , endpoint_tx_buffer_size_tag <tx_buffer_size>
    >
  >;
};

template
< uint8_t address
, ::usb::Messages::Descriptor::transfer_type_e transfer_type
, std::size_t tx_buffer_size
, std::size_t rx_buffer_size
>
struct as_ep_moded_register_type <address, transfer_type, rx_buffer_size, tx_buffer_size, false, typename std::enable_if <::usb::Messages::Descriptor::transfer_type_e::CONTROL != transfer_type && ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT != transfer_type>::type > {
  
  using type = std::tuple
  < endpoint_register_out_only_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_rx_buffer_size_tag <rx_buffer_size>
    >
  , endpoint_register_in_only_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_tx_buffer_size_tag <tx_buffer_size>
    >
  >;
};

template
< uint8_t address
, ::usb::Messages::Descriptor::transfer_type_e transfer_type
, std::size_t tx_buffer_size
, std::size_t rx_buffer_size
>
struct as_ep_moded_register_type <address, transfer_type, rx_buffer_size, tx_buffer_size, true, typename std::enable_if <::usb::Messages::Descriptor::transfer_type_e::ISOCHRONOUS != transfer_type>::type> {
  
  using type = std::tuple
  < endpoint_register_bidir_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_rx_buffer_size_tag <rx_buffer_size>
    , endpoint_tx_buffer_size_tag <tx_buffer_size>
    >
  >;
};

template
< uint8_t address
, ::usb::Messages::Descriptor::transfer_type_e transfer_type
//, std::size_t tx_buffer_size
, std::size_t rx_buffer_size
//, bool prefer_single_buffer
>
struct as_ep_moded_register_type <address, transfer_type, rx_buffer_size, 0, false, typename std::enable_if <::usb::Messages::Descriptor::transfer_type_e::CONTROL != transfer_type && ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT != transfer_type>::type> {
  
  using type = std::tuple
  < endpoint_register_out_only_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_rx_buffer_size_tag <rx_buffer_size>
    >
  >;
};

template
< uint8_t address
, ::usb::Messages::Descriptor::transfer_type_e transfer_type
, std::size_t tx_buffer_size
//, std::size_t rx_buffer_size
//, bool prefer_single_buffer
>
struct as_ep_moded_register_type <address, transfer_type, 0, tx_buffer_size, false, typename std::enable_if <::usb::Messages::Descriptor::transfer_type_e::CONTROL != transfer_type && ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT != transfer_type>::type> {
  
  using type = std::tuple
  < endpoint_register_in_only_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_tx_buffer_size_tag <tx_buffer_size>
    >
  >;
};

/*----------------------------------------------------------------------------*/
template <uint8_t EP_ADDR>
struct device_endpoint_in_isr_tagged_protocol {
  virtual void handle_correct_tx (std::span <uint8_t volatile>, endpoint_address_tag<EP_ADDR> = {}) noexcept = 0;
};

template <uint8_t EP_ADDR>
struct device_endpoint_out_isr_tagged_protocol {
  virtual void handle_correct_rx (std::span <uint8_t volatile>, endpoint_address_tag<EP_ADDR> = {}) noexcept = 0;
};

/*----------------------------------------------------------------------------*/

template <uint8_t EP_ADDR, bool does_OUT, bool does_IN>
struct as_sub_isr_delegate_helper {
  static_assert("Unhandled case");
};

template <uint8_t EP_ADDR>
struct as_sub_isr_delegate_helper <EP_ADDR, true, true> {
  struct type
  : public as_sub_isr_delegate_helper <EP_ADDR, true, false> ::type
  , public as_sub_isr_delegate_helper <EP_ADDR, false, true> ::type
  { };
};

template <uint8_t EP_ADDR>
struct as_sub_isr_delegate_helper <EP_ADDR, true, false> {
  using type = device_endpoint_out_isr_tagged_protocol <EP_ADDR>;
};

template <uint8_t EP_ADDR>
struct as_sub_isr_delegate_helper <EP_ADDR, false, true> {
  using type = device_endpoint_in_isr_tagged_protocol <EP_ADDR>;
};

template <typename EP_CTL_T>
using as_sub_isr_delegate_t = as_sub_isr_delegate_helper
< EP_CTL_T::ADDRESS
, std::is_base_of_v <endpoint_rx_buffer_size_base, EP_CTL_T>
, std::is_base_of_v <endpoint_tx_buffer_size_base, EP_CTL_T>
> ::type;

/*----------------------------------------------------------------------------*/
template <typename T>
struct as_isr_delegate {
  static_assert ("Expecting tuple of tagged endpoint-control-register types.");
};

template <typename...EP_CTL_T>
struct as_isr_delegate <std::tuple<EP_CTL_T...>> {
  struct type
  : public as_sub_isr_delegate_t <EP_CTL_T> ...
  { };
};

/*----------------------------------------------------------------------------*/
template <typename...TAGS>
constexpr std::tuple <std::size_t, std::size_t> max_sizes
( endpoint_register_bidir_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::TX_BUFFER_SIZE, decltype(x)::RX_BUFFER_SIZE);
}

template <typename...TAGS>
constexpr std::tuple <std::size_t, std::size_t> max_sizes
( endpoint_register_out_only_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::RX_BUFFER_SIZE, decltype(x)::RX_BUFFER_SIZE);
}

template <typename...TAGS>
constexpr std::tuple <std::size_t, std::size_t> max_sizes
( endpoint_register_in_only_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::TX_BUFFER_SIZE, decltype(x)::TX_BUFFER_SIZE);
}
/*----------------------------------------------------------------------------*/

//And I want that to generate the ISR code.
//But this doesn't quite work.  It needs endpoint control types
//endpoint_register_bidir_tagged_t    <0, CONTROL    , 64, 64>
//endpoint_register_out_only_tagged_t <1, BULK       , 64,  0>
//endpoint_register_in_only_tagged_t  <1, BULK       ,  0, 64>
//endpoint_register_in_only_tagged_t  <2, ISOCHRONOUS,  0,  8>

//which translates into:
//isr_rx ([0], addr <0>, protocol)
//isr_tx ([0], addr <0>, protocol)
//isr_rx ([1], addr <1>, protocol)
//isr_tx ([2], addr <1>, protocol)
//isr_tx ([3], addr <2>, protocol)

template
< typename sub_isr_delegate_ref
, typename buffer_ctls_ref
, typename buffer_datas_ref
, typename ep_ctl_tagged_t
>
void inline endpoint_sub_isr_rx (ep_ctl_tagged_t volatile & ep_ctl, ep_ctl_tagged_t copy) noexcept {
  ep_ctl = clear_ctr_rx (endpoint_nop (copy));
  
  if (0 == get_application_rx_buffer_index (copy)) {
    (*sub_isr_delegate_ref()).handle_correct_rx
     ( span (getm<0> (*buffer_datas_ref()), getm<0> (*buffer_ctls_ref()))
     , copy
     );
  } else {
    (*sub_isr_delegate_ref()).handle_correct_rx
     ( span (getm<1> (*buffer_datas_ref()), getm<1> (*buffer_ctls_ref()))
     , copy
     );
  }

  ep_ctl = release_rx_buffer (endpoint_nop (copy));
}

template
< typename sub_isr_delegate_ref
, typename buffer_ctls_ref
, typename buffer_datas_ref
, typename ep_ctl_tagged_t
>
void inline endpoint_sub_isr_tx (ep_ctl_tagged_t volatile & ep_ctl, ep_ctl_tagged_t copy) noexcept {
  ep_ctl = clear_ctr_tx (endpoint_nop (copy));
  
  if (0 == get_application_tx_buffer_index (copy)) {
    (*sub_isr_delegate_ref()).handle_correct_tx
    ( max_span (getm<0> (*buffer_datas_ref()))
    , copy
    );
  } else {
    (*sub_isr_delegate_ref()).handle_correct_tx
    ( max_span (getm<1> (*buffer_datas_ref()))
    , copy
    );
  }
}

/*----------------------------------------------------------------------------*/

// @brief Bi-dir ISR for a bi-dir endpoint, specifically typed void(void).
template
< typename ep_ctl_tagged_ref
, typename sub_isr_delegate_ref
, typename buffer_ctls_ref
, typename buffer_datas_ref
>
void endpoint_sub_isr () noexcept {
  auto & ep_ctl = *ep_ctl_tagged_ref();
  std::decay_t <decltype (ep_ctl)> ep_ctl_local;
  
  ep_ctl_local = ep_ctl;

  if constexpr
  ( std::is_base_of_v <endpoint_register_bidir_t, decltype(ep_ctl_local)>
  || std::is_base_of_v <endpoint_register_out_only_t, decltype(ep_ctl_local)>
  ) {
    if (ep_ctl_local.ctr_rx) {
      endpoint_sub_isr_rx
        < sub_isr_delegate_ref, buffer_ctls_ref, buffer_datas_ref >
        ( ep_ctl, ep_ctl_local );
    }
  }
  
  if constexpr
  ( std::is_base_of_v <endpoint_register_bidir_t, decltype(ep_ctl_local)>
  || std::is_base_of_v <endpoint_register_in_only_t, decltype(ep_ctl_local)>
  ) {
    if (ep_ctl_local.ctr_tx) {
      endpoint_sub_isr_tx
        < sub_isr_delegate_ref, buffer_ctls_ref, buffer_datas_ref >
        ( ep_ctl, ep_ctl_local );
    }
  }
}


template
< typename sub_isr_delegate_ref
, typename btable_space_ref
, typename tuple_t
>
struct generate_sub_isr_jump_table_helper;

template
< typename sub_isr_delegate_ref
, typename btable_space_ref
, typename...ep_ctl_tagged_refs
>
struct generate_sub_isr_jump_table_helper
<sub_isr_delegate_ref, btable_space_ref, std::tuple<ep_ctl_tagged_refs...>>
{
  using tuple_t = std::tuple <ep_ctl_tagged_refs...>;
  using btable_space_t = btable_space_ref::DREAM_T;

  template <std::size_t idx>
  using sub_isr_delegate_ref_by_index = typename sub_isr_delegate_ref
    ::base_member
      < as_sub_isr_delegate_t
        < typename tuple_element_t <idx, tuple_t>::DREAM_T >
      >;

  template <std::size_t idx>
  using ctls_ref_by_index = typename btable_space_ref
    ::member <&btable_space_t::ctlses> ::index <idx>;

  template <std::size_t idx>
  using datas_ref_by_index = typename btable_space_ref
    ::member <&btable_space_t::datases> ::index <idx>;

  constexpr static auto doit () {
    return [] <std::size_t...idx> (std::index_sequence <idx...>) {
      return std::array <void (*)(void) noexcept, sizeof...(idx)> {
        & endpoint_sub_isr
        < tuple_element_t <idx, tuple_t>
        , sub_isr_delegate_ref_by_index <idx>
        , ctls_ref_by_index <idx>
        , datas_ref_by_index <idx>
        >
        ...
      };
    } (std::make_index_sequence <sizeof...(ep_ctl_tagged_refs)> ());
  }
};


}//end namespace usb
}//end namespace stm32l41xxx
