#include <tuple>

#include "stm32l41xxx_usb_adaptors_interrupts.hpp"
#include "stm32l41xxx_usb_adaptors_tagged.hpp"

#include "usb_standard.hpp"


namespace stm32l41xxx {
namespace usb {

using ::usb::Messages::Descriptor::transfer_type_e;

using USB_CDC_device_endpoint_register_types = decltype
( std::tuple_cat
  ( as_ep_moded_register_type <0, transfer_type_e::CONTROL,   64, 64> ::type ()
  , as_ep_moded_register_type <1, transfer_type_e::BULK,      64, 64> ::type ()
  , as_ep_moded_register_type <2, transfer_type_e::INTERRUPT,  0,  8> ::type ()
  )
);

using cdc_btable_space_t = btable_space_t <1024, 0x0, USB_CDC_device_endpoint_register_types>;

/*cdc_btable_space_t cdc_buffers;

auto do_crime () {
  cdc_buffers.init_hw();
  return span <3,0> (cdc_buffers);
}
*/
struct dummy_delegate
: public as_isr_delegate <USB_CDC_device_endpoint_register_types> ::type
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

template <typename usb_dev_ref, typename T> struct bind_endpoint_registers;

template <typename usb_dev_ref, typename...Ts>
struct bind_endpoint_registers <usb_dev_ref, std::tuple<Ts...>> {
  using tuple_t = std::tuple <Ts...>;
  using usb_dev_t = typename usb_dev_ref::DREAM_T;
  
  template <std::size_t idx>
  using ep_ctl_tagged_by_idx = tuple_element_t <idx, tuple_t>;
  
  template <std::size_t idx>
  using ep_ctl_ref_type_by_index = typename usb_dev_ref
  ::member <&usb_dev_t::ep>
  ::index <idx>
  ::DREAM_T;
  
  template <std::size_t idx>
  using type_by_index = typename usb_dev_ref
    ::member <&usb_dev_t::ep>
    ::index <idx>
    ::base_member <endpoint_register_t>
    ::static_casted <typename ep_ctl_tagged_by_idx <idx>::UNTAGGED_T>
    ::static_casted <ep_ctl_tagged_by_idx <idx>>
  ;
  
  constexpr static auto helper () {
    return [] <std::size_t...idx> (std::index_sequence<idx...>) {
      return std::tuple <type_by_index <idx> ...> ();
    } (std::make_index_sequence <sizeof...(Ts)> ());
  }
  
  using type = decltype (helper());
};

using bound_eps = typename bind_endpoint_registers
  < typeify <usb1>
  , USB_CDC_device_endpoint_register_types
  > ::type;

auto some_drunk_idiot =
generate_sub_isr_jump_table_helper
< typeify <the_delegate>
, typeify <usb1_sram> ::reinterpreted <cdc_btable_space_t>
, bound_eps
>
::doit ();


struct usb_device_isr_delegate {
  //ignore CTR.
  virtual void handle_pma_overrun_underrun () = 0;
  virtual void handle_error () = 0;
  virtual void handle_wakeup () = 0;
  virtual void handle_suspend () = 0;
  virtual void handle_reset () = 0;
  virtual void handle_sof () = 0;     //sof: start of frame
  virtual void handle_expected_sof () = 0;
  virtual void handle_lpm_l1_req () = 0;  //low-power-mode
};

}//end namespace usb
}//end namespace stm32l41xxx


struct device_control_protocol_standard {
  //TODO:  Handles for all the setup messages and shit.
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_standard_status_m) = 0;
  //virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_endpoint_status_m) = 0;
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_interface_status_m) = 0;
  virtual void handle (usb::Messages::Setup::clear_standard_feature_m) = 0;
  //virtual void handle (usb::Messages::Setup::clear_endpoint_feature_m) = 0;
  //virtual void handle (usb::Messages::Setup::clear_interface_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_standard_feature_m) = 0;
  //virtual void handle (usb::Messages::Setup::set_endpoint_feature_m) = 0;
  //virtual void handle (usb::Messages::Setup::set_interface_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_address_m) = 0;
  virtual std::optional <std::span <uint8_t>> handle (usb::Messages::Setup::get_descriptor_m) = 0;
  virtual void handle (usb::Messages::Setup::set_descriptor_m) = 0;
  virtual std::optional <uint8_t> handle (usb::Messages::Setup::get_configuration_m) = 0;
  virtual void handle (usb::Messages::Setup::set_configuration_m) = 0;
  virtual std::optional <uint8_t> handle (usb::Messages::Setup::get_interface_m) = 0;
  virtual void handle (usb::Messages::Setup::set_interface_m) = 0;
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::synch_frame_m) = 0;
};
