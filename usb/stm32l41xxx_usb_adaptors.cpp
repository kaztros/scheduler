#include "stm32l41xxx_usb_adaptors.hpp"

namespace stm32l41xxx {
namespace usb {

endpoint_register_unidirectional_tx_t::operator endpoint_register_t & () {
  return * reinterpret_cast <endpoint_register_t*> (this);
}

endpoint_register_unidirectional_rx_t::operator endpoint_register_t & () {
  return * reinterpret_cast <endpoint_register_t*> (this);
}

/*
std::tuple <uint16_t*, decltype(buffer_count_t::byte_count) &> as_native_memory_address (btable_register_t btable, usb_buffer_registers_t buffer_desc) {
  uint32_t addr = 0x4000'0600 + btable.addr + buffer_desc.addr;
  std::size_t byte_count = buffer_desc.byte_count;

  return std::make_tuple (reinterpret_cast <uint16_t*> (addr), byte_count);
}
*/

}//end namespace usb
}//end namespace stm32l41xxx
