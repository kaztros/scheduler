#include "stm32l41xxx_usb_register_verbs.hpp"

#if 0

namespace stm32l41xxx {
namespace usb {

template <typename EP_MODED_T, typename LAMBDA_T>
void invoke_endpoint_isr_rx (btable_register_t btable /*const-out*/, EP_MODED_T volatile & ep_ctl, usb_buffer_registers_t const volatile (&buffer_descs) [2], LAMBDA_T && op) {
  auto ep_ctl_copy = raw_snapshot_of (ep_ctl);
  if (ep_ctl_copy.ctr_rx) {
    ep_ctl = clear_ctr_rx (endpoint_nop (ep_ctl_copy));
    //Signal that there's a buffer ready for reading.
    std::apply
      ( std::forward <LAMBDA_T> (op)
      , as_native_memory_address /* TODO: This says a buffer is ready */
        ( btable                 /* but can't indicate how much is ready.  Handle it in higher layers. */
        , buffer_descs
          [ get_application_rx_buffer_index (ep_ctl_copy) ]
        )
      );
    ep_ctl = release_rx_buffer (endpoint_nop(ep_ctl_copy));
  }
}

template <typename EP_MODED_T, typename LAMBDA_T>
void invoke_endpoint_isr_tx (btable_register_t btable /*const-out*/, EP_MODED_T volatile & ep_ctl, usb_buffer_registers_t const volatile (&buffer_descs) [2], LAMBDA_T && op) {
  auto ep_ctl_copy = raw_snapshot_of (ep_ctl);
  if (ep_ctl_copy.ctr_tx) {
    ep_ctl = clear_ctr_tx (endpoint_nop (ep_ctl_copy));
    // Signal that there's a buffer ready for writing from application:
    std::apply
      ( std::forward <LAMBDA_T> (op)
      , as_native_memory_address /* TODO: This says a buffer is ready */
        ( btable                 /* but can't indicate how much is ready.  Handle it in higher layers. */
        , buffer_descs
          [ get_application_tx_buffer_index (ep_ctl_copy) ]
        )
      );
    //ep_ctl = commit_tx_buffer (endpoint_nop (ep_ctl_copy)); <- Application should trigger this.
  }
}

template <typename EP_MODED_T, typename LAMBDA_T>
void send_buffer_naive (EP_MODED_T volatile & ep_ctl, usb_buffer_registers_t volatile (&buffer_descs)[2], uint16_t * from, std::size_t const length_words) {

  static_assert (EP_MODED_T::BUFFER_SIZE_BYTES % sizeof(uint16_t) == 0);
  //^ Things are best when the buffer can't truncate mid-element.
  
  std::size_t offset_index = 0;
  constexpr auto MAX_INDEXES = EP_MODED_T::BUFFER_SIZE_BYTES / sizeof (uint16_t);
  
  while (offset_index + MAX_INDEXES <= length_words) {
    pma_buffer_t buffer buffer_descs
    ( [ (get_application_tx_buffer_index (raw_snapshot_of (ep_ctl)))
    ] );

    offset_index += copy ( &from [offset_index], MAX_INDEXES);
    //TODO: Declare buffer size to the hardware.
    //TODO: I need more VERBS!!
    
    ep_ctl = commit_tx_buffer (endpoint_nop (raw_snapshot_of(ep_ctl)));
    
    while (! ep_ctl.ctr_tx) { }
    // TODO: ^ This doesn't cover errors, timeouts, or resets, possibly overruns?
    
    ep_ctl = clear_ctr_tx (endpoint_nop (ep_ctl));
  }
  
  // Send off a smaller-than-max chunk (possibly zero-length) to indicate
  // that the chunk-stream is done for this frame.
  { pma_buffer_t buffer buffer_descs
    ( [ (get_application_tx_buffer_index (raw_snapshot_of (ep_ctl)))
    ] );

    offset_index += copy (from [offset], length_words - offset);
    ep_ctl = commit_tx_buffer (endpoint_nop (raw_snapshot_of(ep_ctl)));
    
    while (! ep_ctl.ctr_tx) { }
    ep_ctl = clear_ctr_tx (endpoint_nop (ep_ctl));
  }
}

}//end namespace usb
}//end namespace stm32l41xxx

#endif //0    
