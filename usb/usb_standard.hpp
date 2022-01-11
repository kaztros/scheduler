#pragma once
#include "register_building.hpp"

namespace usb {

namespace Messages {

namespace Setup {

enum class type_e : uint8_t {
  STANDARD = 0b00,
  CLASS = 0b01,
  VENDOR = 0b10,
};

enum class recipient_e : uint8_t {
  DEVICE = 0b00,
  INTERFACE = 0b01,
  ENDPOINT = 0b10,
};

enum class request_e : uint8_t {
  GET_STATUS = 0x00U,
  CLEAR_FEATURE = 0x01U,
  SET_FEATURE = 0x03U,
  SET_ADDRESS = 0x05U,
  GET_DESCRIPTOR = 0x06U,
  SET_DESCRIPTOR = 0x07U,
  GET_CONFIGURATION = 0x08U,
  SET_CONFIGURATION = 0x09U,
  GET_INTERFACE = 0x0AU,
  SET_INTERFACE = 0x0BU,
  SYNCH_FRAME = 0x0CU,
};

struct endpoint_address_m {
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint8_t,  0, 4>> address;
    BitfieldMember <uint16_t, BFE <bool,  7, 1>> is_in;
  };
};

struct interface_number_m {
  union {
    uint16_t _raw;
    BitfieldMember <uint8_t, BFE <uint8_t,  0, 8>> number;
  };
};

struct get_standard_status_m {
//  uint16_t wValue;    //should == 0
//  uint16_t wIndex;    //should == 0
//  uint16_t wLength;   //should == 2
};

struct get_endpoint_status_m {
//0
  endpoint_address_m address;
//2
};

struct get_interface_status_m {
  //0
  interface_number_m number;
  //2
};

enum class feature_selector_e : uint16_t { };

struct clear_standard_feature_m {
  feature_selector_e selector;
  //0
  //0
};

struct clear_endpoint_feature_m {
  feature_selector_e selector;
  endpoint_address_m address;
  //0
};

struct clear_interface_feature_m {
  feature_selector_e selector;
  interface_number_m number;
  //0
};

struct set_standard_feature_m {
  feature_selector_e selector;
  //0
  //0
};

struct set_endpoint_feature_m {
  feature_selector_e selector;
  endpoint_address_m address;
  //0
};

struct set_interface_feature_m {
  feature_selector_e selector;
  interface_number_m number;
  //0
};

struct set_address_m {
  uint16_t device_address;  //should < 128
  //0
  //0
};

struct descriptor_type_index_t {
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <uint8_t,  0, 8>> type; //TODO: Right order?
    BitfieldMember <uint16_t, BFE <uint8_t,  8, 8>> index;
  };
};

struct get_descriptor_m {
  descriptor_type_index_t type_and_index;  //should < 128
  uint16_t language_index;
  uint16_t descriptor_length;
};

struct set_descriptor_m {
  descriptor_type_index_t type_and_index;  //should < 128
  uint16_t language_index;
  uint16_t descriptor_length;
};

struct get_configuration_m {
//  uint16_t wValue;    //should == 0
//  uint16_t wIndex;    //should == 0
//  uint16_t wLength;   //should == 1
};

struct set_configuration_m {
  uint16_t configuration_value;
  //0
  //0
};

struct get_interface_m {
  //0
  interface_number_m number;
  //1
};

struct set_interface_m {
  uint16_t alternate_setting;
  interface_number_m number;
  //0
};

struct synch_frame_m {
  //0
  endpoint_address_m address;
  //2
};

struct setup_request_t {
  union {
    uint8_t _raw;
    BitfieldMember <uint8_t, BFE <recipient_e, 0, 5, recipient_e::ENDPOINT>> recipient;
    BitfieldMember <uint8_t, BFE <type_e, 5, 2, type_e::VENDOR>> type;  //Type of request.
    BitfieldMember <uint8_t, BFE <bool, 7, 1>> is_in;
  } bmRequest;
  
  request_e bRequest;
  //boost::variant <std::array<uint16_t, 3>, > payload;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

}//end Setup

namespace Descriptor {
enum class type_e : uint8_t {
  DEVICE = 0x01U,
  CONFIGURATION = 0x02U,
  STRING = 0x03U,
  INTERFACE = 0x04U,
  ENDPOINT = 0x05U,
  DEVICE_QUALIFIER = 0x06U,
  OTHER_SPEED_CONFIGURATION = 0x07U,
  INTERFACE_POWER = 0x08U,
  BOS = 0x0FU,
  DEVICE_CAPABILITY = 0x10U,   //Might be in the wrong enumeration.
};

enum class transfer_type_e : uint8_t {
  CONTROL = 0b00,
  ISOCHRONOUS = 0b01,
  BULK = 0b10,
  INTERRUPT = 0b11,
};

}//end namespace Descriptor



struct device_status_t {
  union {
    uint16_t _raw;
    BitfieldMember <uint16_t, BFE <bool, 0, 1>> isSelfPowered;
    BitfieldMember <uint16_t, BFE <bool, 1, 1>> canRemoteWakeup;
    BitfieldMember <uint16_t, BFE <uint16_t, 2, 14>> reserved_bits_2_14;
  };
};

}//end namespace Messages

}//end namespace usb


