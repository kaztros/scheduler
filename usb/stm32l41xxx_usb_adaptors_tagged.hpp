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
struct endpoint_register_bidirectional_tagged_t
: public endpoint_register_bidirectional_t
, public volatile_assign_by_raw <endpoint_register_bidirectional_tagged_t <TAGS...> >
, TAGS ...
{
  using volatile_assign_by_raw <endpoint_register_bidirectional_tagged_t<TAGS...>>::operator=;
};


template <typename...TAGS>
struct endpoint_register_unidirectional_rx_tagged_t
: public endpoint_register_unidirectional_rx_t
, public volatile_assign_by_raw <endpoint_register_unidirectional_rx_tagged_t <TAGS...> >
, TAGS ...
{
  using volatile_assign_by_raw <endpoint_register_unidirectional_rx_tagged_t<TAGS...>>::operator=;
};


template <typename...TAGS>
struct endpoint_register_unidirectional_tx_tagged_t
: public endpoint_register_unidirectional_tx_t
, public volatile_assign_by_raw <endpoint_register_unidirectional_tx_tagged_t <TAGS...> >
, TAGS ...
{
  using volatile_assign_by_raw <endpoint_register_unidirectional_tx_tagged_t<TAGS...>>::operator=;
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
  < endpoint_register_bidirectional_tagged_t
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
  < endpoint_register_unidirectional_rx_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_rx_buffer_size_tag <rx_buffer_size>
    >
  , endpoint_register_unidirectional_tx_tagged_t
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
  < endpoint_register_bidirectional_tagged_t
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
  < endpoint_register_unidirectional_rx_tagged_t
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
  < endpoint_register_unidirectional_tx_tagged_t
    < endpoint_address_tag <address>
    , transfer_type_tag <transfer_type>
    , endpoint_tx_buffer_size_tag <tx_buffer_size>
    >
  >;
};

/*----------------------------------------------------------------------------*/
template <uint8_t EP_ADDR>
struct device_endpoint_in_isr_tagged_protocol {
  virtual void handle_correct_tx (std::span <uint8_t volatile>, endpoint_address_tag<EP_ADDR> = {}) = 0;
};

template <uint8_t EP_ADDR>
struct device_endpoint_out_isr_tagged_protocol {
  virtual void handle_correct_rx (std::span <uint8_t volatile>, endpoint_address_tag<EP_ADDR> = {}) = 0;
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
( endpoint_register_bidirectional_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::TX_BUFFER_SIZE, decltype(x)::RX_BUFFER_SIZE);
}

template <typename...TAGS>
constexpr std::tuple <std::size_t, std::size_t> max_sizes
( endpoint_register_unidirectional_rx_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::RX_BUFFER_SIZE, decltype(x)::RX_BUFFER_SIZE);
}

template <typename...TAGS>
constexpr std::tuple <std::size_t, std::size_t> max_sizes
( endpoint_register_unidirectional_tx_tagged_t <TAGS...> x )
{
  //Hardware is laid out to have TX before RX on this board.
  return std::make_tuple (decltype(x)::TX_BUFFER_SIZE, decltype(x)::TX_BUFFER_SIZE);
}

/*----------------------------------------------------------------------------*/

template
< typename BTABLE_REF_TYPEFIED
, std::size_t EP_CTL_IDX
, typename EP_CTL_TAGGED_T
, std::size_t accumulated_bytes
>
struct as_sram_range_types {
  static constexpr std::tuple <std::size_t, std::size_t> SIZES = max_sizes (EP_CTL_TAGGED_T());
  static constexpr auto SIZE = get<0>(SIZES) + get<1>(SIZES);
  using array_type = std::tuple
    < std::array <volatile uint8_t, get<0>(SIZES)>
    , std::array <volatile uint8_t, get<1>(SIZES)>
    >;
  
  using type = std::tuple
    < sram_range
      < typename BTABLE_REF_TYPEFIED::index<EP_CTL_IDX>::index<0>
      , accumulated_bytes + 0
      , get<0>(SIZES)
      >
    , sram_range
      < typename BTABLE_REF_TYPEFIED::index<EP_CTL_IDX>::index<0>
      , accumulated_bytes + get<0>(SIZES)
      , get<1>(SIZES)
      >
    >;
};

template
< typename BTABLE_REF_TYPEFIED
, std::size_t EP_CTL_IDX
, std::size_t ACCUMULATED_BYTES
, typename...SR_RANGES
>
constexpr auto as_sram_range_from_ep_ctl_helper
  ( std::tuple <SR_RANGES...> acc
  , std::tuple <> in
  )
{ return acc; };

template
< typename BTABLE_REF_TYPEFIED
, std::size_t EP_CTL_IDX
, std::size_t ACCUMULATED_BYTES
, typename...SR_RANGES
, typename EP_CTL_TAGGED_T
, typename...EP_REST>
constexpr auto as_sram_range_from_ep_ctl_helper
  ( std::tuple <SR_RANGES...> acc
  , std::tuple <EP_CTL_TAGGED_T, EP_REST...> in
  )
{
  using T = as_sram_range_types <BTABLE_REF_TYPEFIED, EP_CTL_IDX, EP_CTL_TAGGED_T, ACCUMULATED_BYTES>;
  
  return as_sram_range_from_ep_ctl_helper
    < BTABLE_REF_TYPEFIED
    , EP_CTL_IDX + 1
    , ACCUMULATED_BYTES + T::SIZE
    >
    ( std::tuple <SR_RANGES... , typename T::type> ()
    , std::tuple <EP_REST...> ()
    );
};


/*----------------------------------------------------------------------------*/

//And I want that to generate the ISR code.
//But this doesn't quite work.  It needs endpoint control types
//endpoint_register_bidirectional_tagged_t      <0, ISOCHRONOUS, 64, 64>
//endpoint_register_unidirectional_rx_tagged_t  <1, BULK       , 64,  0>
//endpoint_register_unidirectional_tx_tagged_t  <1, BULK       ,  0, 64>
//endpoint_register_unidirectional_tx_tagged_t  <2, ISOCHRONOUS,  0,  8>

//which translates into:
//isr_rx ([0], addr <0>, protocol)
//isr_tx ([0], addr <0>, protocol)
//isr_rx ([1], addr <1>, protocol)
//isr_tx ([2], addr <1>, protocol)
//isr_tx ([3], addr <2>, protocol)


template
< typename EP_CTL_TAGGED_T
, as_sub_isr_delegate_t <EP_CTL_TAGGED_T> & delegate
, device_registers_t volatile & device
, std::size_t ep_ctl_index
, std::size_t PMA_SIZE
, std::size_t btable_offset = 0x0000
, h_array <PMA_SIZE, btable_offset> & PMA_SPACE
, buffer_spans_t volatile & buffer_descs = PMA_SPACE.buffer_spans [ep_ctl_index]
>
void endpoint_sub_isr () {
  auto volatile & ep_ctl = reinterpret_cast <EP_CTL_TAGGED_T volatile &> (device.ep[ep_ctl_index]);
  EP_CTL_TAGGED_T ep_ctl_copy;
  
  loop_again:
  ep_ctl_copy = raw_snapshot_of (ep_ctl);
  if constexpr (0 != EP_CTL_TAGGED_T::RX_BUFFER_SIZE) {
    if (ep_ctl_copy.ctr_rx) {
      ep_ctl = clear_ctr_rx (endpoint_nop (ep_ctl_copy));

      //Signal that there's a buffer ready for reading.
      //delegate.handle_correct_rx
      //( pma_buffer_t { buffer_descs         <- Replace with PMA_SPACE.span()
      //  [ get_application_rx_buffer_index (ep_ctl_copy) ]
      //});
      
      ep_ctl = release_rx_buffer (endpoint_nop(ep_ctl_copy));
      goto loop_again;
    }
  }

  if constexpr (0 != EP_CTL_TAGGED_T::TX_BUFFER_SIZE) {
    if (ep_ctl_copy.ctr_tx) {
      ep_ctl = clear_ctr_tx (endpoint_nop (ep_ctl_copy));

      //Signal that there's a buffer ready for writing.
      //delegate.handle_correct_tx
      //( pma_buffer_t { buffer_descs         <- Replace with PMA_SPACE.span()
      //  [ get_application_rx_buffer_index (ep_ctl_copy) ]
      //});
      
      goto loop_again;
    }
  }
}

template <typename...EP_CTL_T, typename ISR_DELEGATE_T>//, ISR_DELEGATE_T & isr_delegate>
constexpr auto generate_sub_isr_jump_table (std::tuple <EP_CTL_T...> reg_types, ISR_DELEGATE_T & isr_delegate) {
  auto fn_ptr_map = [&isr_delegate] (std::size_t index, auto ep_ctl)
  -> void (*)(void) {
    return & endpoint_sub_isr
    < decltype (ep_ctl)
    , isr_delegate
    , usb1
    , index
    , 0x0400
    , 0x0000
    , usb1_sram
    >;
  };
  
  return [&reg_types, fn_ptr_map] <std::size_t...idx> (std::index_sequence <idx...> ) {
    return std::array <void (*)(void), sizeof...(idx)> {
      fn_ptr_map (idx, std::get <idx> (reg_types)) ...
    };
  } (std::make_index_sequence <sizeof...(EP_CTL_T)> ());
}





}//end namespace usb
}//end namespace stm32l41xxx
