#include <tuple>

#include "stm32l41xxx_usb_adaptors_interrupts.hpp"
#include "stm32l41xxx_usb_adaptors_tagged.hpp"

namespace stm32l41xxx {
namespace usb {

using USB_CDC_Spec = decltype
( std::tuple_cat
  ( as_ep_moded_register_type <0, ::usb::Messages::Descriptor::transfer_type_e::CONTROL,   64, 64> ::type ()
  , as_ep_moded_register_type <1, ::usb::Messages::Descriptor::transfer_type_e::BULK,      64, 64> ::type ()
  , as_ep_moded_register_type <2, ::usb::Messages::Descriptor::transfer_type_e::INTERRUPT,  0,  8> ::type ()
  )
);

using sram_ranges_t =
  decltype
  ( as_sram_range_from_ep_ctl_helper
    < typeify <usb1_sram> :: member<& decltype(usb1_sram)::buffer_spanses>
    , 0
    , 8 * std::tuple_size_v <USB_CDC_Spec>
    >
    (std::tuple<>(), USB_CDC_Spec())
  );

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

/*
void (*shovel_ware)(void) = & race_fast
< usb1
, 0
, 0x0400
, 0x0000
, usb1_sram
, typename               std::tuple_element <0, USB_CDC_Spec> ::type
, decltype (the_delegate)
, the_delegate
>;
*/

//auto this_is_not_a_game = generate_sub_isr_jump_table (USB_CDC_Spec {}, TheDelegate);

}//end namespace usb
}//end namespace stm32l41xxx
