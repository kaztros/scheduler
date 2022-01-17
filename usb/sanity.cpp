#include <tuple>

#include "stm32l41xxx_usb_adaptors_interrupts.hpp"
#include "stm32l41xxx_usb_adaptors_tagged.hpp"

namespace stm32l41xxx {
namespace usb {

using ::usb::Messages::Descriptor::transfer_type_e;

using USB_CDC_Spec = decltype
( std::tuple_cat
  ( as_ep_moded_register_type <0, transfer_type_e::CONTROL,   64, 64> ::type ()
  , as_ep_moded_register_type <1, transfer_type_e::BULK,      64, 64> ::type ()
  , as_ep_moded_register_type <2, transfer_type_e::INTERRUPT,  0,  8> ::type ()
  )
);

using cdc_btable_space_t = btable_space_t <1024, 0x0, USB_CDC_Spec>;

cdc_btable_space_t cdc_buffers;

auto do_crime () {
  cdc_buffers.init_hw();
  return span <2,0> (cdc_buffers);
}

/*
using sram_ranges_t =
  decltype
  ( as_sram_range_from_ep_ctl_helper
    < typeify <usb1_sram> :: member<& decltype(usb1_sram)::buffer_spanses>
    , 0
    , 8 * std::tuple_size_v <USB_CDC_Spec>
    >
    (std::tuple<>(), USB_CDC_Spec())
  );
*/
struct dummy_delegate
: public as_isr_delegate <USB_CDC_Spec> ::type
{
  void handle_correct_rx (std::span <uint8_t volatile>, endpoint_address_tag <0>) noexcept override {
    
  }
  void handle_correct_tx (std::span <uint8_t volatile>, endpoint_address_tag <0>) noexcept override {
    
  }
  void handle_correct_rx (std::span <uint8_t volatile>, endpoint_address_tag <1>) noexcept override {
    
  }
  void handle_correct_tx (std::span <uint8_t volatile>, endpoint_address_tag <1>) noexcept override {
    
  }
  void handle_correct_rx (std::span <uint8_t volatile>, endpoint_address_tag <2>) noexcept override {
    /* This shouldn't be. */
  }
  void handle_correct_tx (std::span <uint8_t volatile>, endpoint_address_tag <2>) noexcept override {
  
  }
};

dummy_delegate the_delegate;

as_sub_isr_delegate_t <std::tuple_element <0, USB_CDC_Spec> ::type> & the_delegate_0 = the_delegate;


}//end namespace usb
}//end namespace stm32l41xxx
