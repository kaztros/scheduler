#pragma once
#include <optional>
#include <span>
#include "usb_standard.hpp"
//The issue is that I need concepts.
// But the issue is also that I need clang :[

struct sleep_wake_protocol {
  virtual void sleep () = 0; //Called by the function.
  virtual void wake () = 0;  //Called by an interrupt.
};

struct endpoint_tx_provider_protocol {
  //I thought I could obtain buffers statelessly.  But I can't.
  //There's a state in the single-buffered hardware that invalidates buffers.
  //There's a state in the double-buffered hardware that invalidates buffers.
};

struct endpoint_tx_protocol {
  static constexpr std::size_t MAX_SIZE = 0;
  
  std::size_t write_buffer_partial (std::size_t byte_offset, uint16_t const * from, std::size_t const); //STATELESS - it's not though :[
  std::size_t write_buffer (uint16_t const * from);   //STATELESS - it's not though :[
  
  void commit_without_blocking (std::size_t length);
  void commit_while_blocking (std::size_t length);
  void commit (std::size_t length, auto sleep, auto wake);
};

endpoint_tx_protocol allocate_tx_buffer (endpoint_tx_provider_protocol);

/*
template <typename T>
endpoint_tx_protocol allocate_tx_buffer (T x, usb_buffer_registers_t buffers[]) {
  //need:
  //interrupt register
  //endpoint ctl register
  //btable
  //buffer descriptors
  //interrupt hook
  
  while (ep_ctl.stat_tx != 0b11 && ) {}  //Wait until VALID isn't so.
  auto index = get_application_tx_buffer_index (x);
  pma_buffer_t pma_buffer { buffers[index] };
  //needs to return with:
  //pma_buffer              for writing to.
  //endpoint_ctl_register   for committing to
  //interrupt hook          for invalidation
};
*/

std::size_t write_partial (endpoint_tx_protocol, std::size_t byte_offset, uint16_t const * from, std::size_t const max_index);
std::size_t write_partial (endpoint_tx_protocol, std::size_t byte_offset, uint8_t const * from, std::size_t const max_index);

template <typename ENDPOINT_TX>
std::size_t write (ENDPOINT_TX, uint8_t const (&from) [endpoint_tx_protocol::MAX_SIZE]);

template <typename ENDPOINT_TX>
std::size_t write (ENDPOINT_TX, uint16_t const (&from) [ENDPOINT_TX::MAX_SIZE/sizeof(uint16_t)]);

template <typename ENDPOINT_TX>
struct endpoint_tx_protocol_iterator
: protected ENDPOINT_TX {
  //static constexpr TX_BUFFER_SIZE_BYTES <- Get this from ENDPOINT_TX.

  endpoint_tx_protocol_iterator (ENDPOINT_TX inner)
  : ENDPOINT_TX (inner)
  { }
  
  uint16_t partial_word;  //Needed state for 16-bit peripheral memory address.
  size_t tx_index_bytes;  //Needed state for "write" interface.

  std::size_t write_buffer_partial (uint16_t const * from, std::size_t const max_index);  //This ought to be a span, but clang ;-;.
  
  bool write (uint16_t);  //Little-endian is sent first.
  bool write (uint8_t);
  
  using ENDPOINT_TX::commit_without_blocking;
  using ENDPOINT_TX::commit_while_blocking;
  using ENDPOINT_TX::commit;
};

template <typename T = uint16_t>
struct endpoint_rx_protocol {
  void received_words (T volatile *, std::size_t) = 0;
  void reception_over ();
};

struct device_control_protocol {
  //TODO:  Handles for all the setup messages and shit.
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_standard_status_m) = 0;
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_endpoint_status_m) = 0;
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::get_interface_status_m) = 0;
  virtual void handle (usb::Messages::Setup::clear_standard_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::clear_endpoint_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::clear_interface_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_standard_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_endpoint_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_interface_feature_m) = 0;
  virtual void handle (usb::Messages::Setup::set_address_m) = 0;
  virtual std::optional <std::span <uint8_t>> handle (usb::Messages::Setup::get_descriptor_m) = 0;
  virtual void handle (usb::Messages::Setup::set_descriptor_m) = 0;
  virtual std::optional <uint8_t> handle (usb::Messages::Setup::get_configuration_m) = 0;
  virtual void handle (usb::Messages::Setup::set_configuration_m) = 0;
  virtual std::optional <uint8_t> handle (usb::Messages::Setup::get_interface_m) = 0;
  virtual void handle (usb::Messages::Setup::set_interface_m) = 0;
  virtual std::optional <uint16_t> handle (usb::Messages::Setup::synch_frame_m) = 0;
};

struct endpoint_rx_buffer_size_base { };

template <std::size_t _BUFFER_SIZE>
struct endpoint_rx_buffer_size_tag
: public endpoint_rx_buffer_size_base
{
  static constexpr std::size_t RX_BUFFER_SIZE = _BUFFER_SIZE;
};

struct endpoint_tx_buffer_size_base { };

template <std::size_t _BUFFER_SIZE>
struct endpoint_tx_buffer_size_tag
: public endpoint_tx_buffer_size_base
{
  static constexpr std::size_t TX_BUFFER_SIZE = _BUFFER_SIZE;
};

struct endpoint_address_tag_base { };

template <uint8_t ENDPOINT_ADDRESS>
struct endpoint_address_tag
: public endpoint_address_tag_base
{
  static constexpr auto ADDRESS = ENDPOINT_ADDRESS;
};

struct transfer_type_tag_base { };

template <usb::Messages::Descriptor::transfer_type_e transfer_type>
struct transfer_type_tag
: public transfer_type_tag_base
{
  static constexpr auto TRANSFER_TYPE = transfer_type;
};

template <typename T>
struct is_tagged {
  static constexpr auto value = std::is_base_of_v <T, endpoint_address_tag_base>
    && std::is_base_of_v <T, transfer_type_tag_base>;
};

/*----------------------------------------------------------------------------*/
struct buffer_protocol {
  virtual uint8_t * data () const = 0;
  virtual std::size_t * size () const = 0;
};
