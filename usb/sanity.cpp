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

struct dummy_delegate
: public as_isr_delegate <USB_CDC_Spec> ::type
{
  void handle_correct_rx (buffer_protocol &, endpoint_address_tag <0>) override {
    
  }
  void handle_correct_tx (buffer_protocol &, endpoint_address_tag <0>) override {
    
  }
  void handle_correct_rx (buffer_protocol &, endpoint_address_tag <1>) override {
    
  }
  void handle_correct_tx (buffer_protocol &, endpoint_address_tag <1>) override {
    
  }
  void handle_correct_rx (buffer_protocol &, endpoint_address_tag <2>) override {
  
  }
  void handle_correct_tx (buffer_protocol &, endpoint_address_tag <2>) override {
  
  }
};


template
< device_registers_t volatile & device
, std::size_t ep_ctl_index
, std::size_t PMA_SIZE
, std::size_t btable_offset
, h_array <PMA_SIZE, btable_offset> & PMA_SPACE
, typename EP_CTL_TAGGED_T
//, as_sub_isr_delegate_t <EP_CTL_TAGGED_T> & delegate
, typename ISR_DELEGATE_T
, ISR_DELEGATE_T & delegate
>
void race_fast () {
  auto volatile & ep_ctl = reinterpret_cast <EP_CTL_TAGGED_T volatile &> (device.ep[ep_ctl_index]);
  auto volatile & buffer_spans = PMA_SPACE.buffer_spanses[ep_ctl_index];
  EP_CTL_TAGGED_T ep_ctl_copy;
  
  loop_again:
  ep_ctl_copy = raw_snapshot_of (ep_ctl);
  if constexpr (0 != EP_CTL_TAGGED_T::RX_BUFFER_SIZE) {
    if (ep_ctl_copy.ctr_rx) {
      ep_ctl = clear_ctr_rx (endpoint_nop (ep_ctl_copy));

      //Signal that there's a buffer ready for reading.
      delegate.handle_correct_rx
      ( pma_buffer_t
        { buffer_spans [ get_application_rx_buffer_index (ep_ctl_copy) ]
        }
      , ep_ctl_copy
      );
      ep_ctl = release_rx_buffer (endpoint_nop(ep_ctl_copy));
      goto loop_again;
    }
  }

  if constexpr (0 != EP_CTL_TAGGED_T::TX_BUFFER_SIZE) {
    if (ep_ctl_copy.ctr_tx) {
      ep_ctl = clear_ctr_tx (endpoint_nop (ep_ctl_copy));

      //Signal that there's a buffer ready for writing.
      delegate.handle_correct_tx
      ( pma_buffer_t
        { buffer_spans
          [ get_application_tx_buffer_index (ep_ctl_copy) ]
        }
      , ep_ctl_copy
      );
      goto loop_again;
    }
  }
}

dummy_delegate the_delegate;
as_sub_isr_delegate_t <std::tuple_element <0, USB_CDC_Spec> ::type> & the_delegate_0 = the_delegate;

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


//auto this_is_not_a_game = generate_sub_isr_jump_table (USB_CDC_Spec {}, TheDelegate);

}//end namespace usb
}//end namespace stm32l41xxx
