#include "stm32l41xxx_usb.hpp"

namespace stm32l41xxx {
namespace usb {

endpoint_register_in_only_t::operator endpoint_register_setup_t & () {
  return * reinterpret_cast <endpoint_register_setup_t*> (this);
}

endpoint_register_out_only_t::operator endpoint_register_setup_t & () {
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
