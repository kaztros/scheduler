#include "stm32l41xxx_usb.hpp"

namespace stm32l41xxx {
namespace usb {

using ::usb::Messages::Descriptor::transfer_type_e;

constexpr transfer_type_e as_transfer_type (endpoint_type_e x) noexcept {
  // Hey C++?  This sucks.  The "good safe part" sucks to write, and the allowance of the last part sucks.
  switch (x) {
    case endpoint_type_e::BULK:        return transfer_type_e::BULK;
    case endpoint_type_e::CONTROL:     return transfer_type_e::CONTROL;
    case endpoint_type_e::ISOCHRONOUS: return transfer_type_e::ISOCHRONOUS;
    case endpoint_type_e::INTERRUPT:   return transfer_type_e::INTERRUPT;
  }
  //return ::usb::Messages::Descriptor::transfer_type_e (0b100);
  //Flagrantly die.
}

constexpr endpoint_type_e as_endpoint_type (transfer_type_e x) noexcept {
  switch (x) {
    case transfer_type_e::BULK:        return endpoint_type_e::BULK;
    case transfer_type_e::CONTROL:     return endpoint_type_e::CONTROL;
    case transfer_type_e::ISOCHRONOUS: return endpoint_type_e::ISOCHRONOUS;
    case transfer_type_e::INTERRUPT:   return endpoint_type_e::INTERRUPT;
  }
  //return endpoint_type_e (0b100);
  //Flagrantly die.
}

endpoint_register_tx_only_t::operator endpoint_register_setup_t & () {
  return * reinterpret_cast <endpoint_register_setup_t*> (this);
}

endpoint_register_rx_only_t::operator endpoint_register_setup_t & () {
  return * reinterpret_cast <endpoint_register_setup_t*> (this);
}

buffer_span_t::buffer_span_t (buffer_span_t const volatile & rhs)
: buffer_offset_t (raw_snapshot_of <buffer_offset_t> (rhs))
, buffer_count_t (raw_snapshot_of <buffer_count_t> (rhs))
{ }

device_registers_t volatile usb1;      // @ 0x4000'6800, APB1
extern pma_ram <1024, volatile uint16_t> usb1_sram;   // @ 0x4000'6C00, APB1

}//end namespace usb
}//end namespace stm32l41xxx
