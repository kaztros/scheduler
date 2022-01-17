#pragma once
#include <tuple>
#include <type_traits>
#include "typeify_global.hpp"

#include "llvm/ADT/Bitfields.h"

/*----------------------------------------------------------------------------*/
// This header is for building register types that behave like Plain Ol' Data.
// It makes writing/reading/reasoning easier.
//  - No more using macros on generic data types to simulate member-assignment.
//  -- Non-membership operations are now forbidden by compiler.
//  - Allow references (which GCC bitfield extensions don't)
//  - Memory-length equivalence with POD-types
//  -- Zero-length bitfield members if used with 'union'
//  -- "volatile" keyword works as one would hope.

// Example structure:
//struct usb_device_address_register_t
//: public volatile_assign_by_raw <usb_device_address_register_t>
//    ^ Non-POD types don't have (volatile <-> non-volatile) assignment-ops.
//      So this template-generates them.
//{
//  union {   <- Union makes dumb-pointer tricks easier to execute.
//    uint16_t _raw;  <- Base-type of the register.
//    BitfieldMember <uint16_t, llvm::Bitfield::Element <uint8_t,  0,  7>> address;
//    ^ memory-member of "_raw", object-member of an instantiation.
//    BitfieldMember <uint16_t, llvm::Bitfield::Element <bool,     7,  1>> ef;
//  };
//
//    using volatile_assign_by_raw::operator=;
//    ^ Some classes are funky about this.
//};

/// @brief The const-version of a bitfield member.
template <typename storage_t, typename Element_t>
struct BitfieldMemberRO {
  using ELEMENT_T = Element_t;
  constexpr operator typename Element_t::Type () const {
    const storage_t & ref = *reinterpret_cast <storage_t const *> (this);
    return llvm::Bitfield::get <Element_t> (ref);
  }
};

/// @brief The read-write version of a bitfield member.
template <typename storage_t, typename Element_t>
struct BitfieldMember
: public BitfieldMemberRO <storage_t, Element_t>
{
  using ELEMENT_T = Element_t;
  constexpr storage_t & operator= (typename Element_t::Type rhs) {
    storage_t & ref = *reinterpret_cast <storage_t*> (this);
    llvm::Bitfield::set<Element_t> (ref, rhs);
    return ref;
  }
};

/// @brief Shorthand
template <typename T, unsigned Offset, unsigned Size,
          T MaxValue = std::is_enum<T>::value
                           ? T(0)
                           : std::numeric_limits<T>::max()>
using BFE = llvm::Bitfield::Element<T, Offset, Size, MaxValue>;

/// @brief Helper function that makes the mask for a BitfieldMember in a field.
template <typename Storage_t, typename T, unsigned OFFSET, unsigned SIZE, T MAX_VALUE>
constexpr Storage_t mask (llvm::Bitfield::Element <T, OFFSET, SIZE, MAX_VALUE>) {
  using BP = llvm::bitfields_details::BitPatterns <Storage_t, SIZE>;
  return BP::Umax << OFFSET;
}

/*----------------------------------------------------------------------------*/
/// @brief - CRTP provider of assignment to/from volatile.
template <typename T>
struct volatile_assign_by_raw {
  T clone () volatile {
    auto _this = static_cast <T volatile *> (this); //static_cast because "this" can be stubborn
    T copy;
    copy._raw = _this._raw; //Rely on the native volatile madness.
    return copy;
  }
  
  T volatile & operator= (T const & rhs) volatile {
    auto _this = static_cast <T volatile *> (this); //static_cast because "this" can be stubborn about having "_raw".
    _this -> _raw = rhs._raw;
    return *_this;
  }
  
  T & operator= (T const volatile & rhs) {
    auto _this = static_cast <T*> (this);
    _this->_raw = rhs._raw;
    return *_this;
  }
};

/*----------------------------------------------------------------------------*/
/// @brief - Explicit conversion from volatile.
template <typename T>
T raw_snapshot_of (T const volatile & x) {
  T copy;
  copy._raw = x._raw;
  return copy;
}

/*----------------------------------------------------------------------------*/
/*
template <typename T, typename func> struct map_tuple_by_func;

template <typename...T, typename func>
struct map_tuple_by_func <std::tuple <T...>, func> {
  using type = std::tuple <std::invoke_result_t <func, T> ...>;
};

template <typename T, typename func>
using map_tuple_by_func_t = typename map_tuple_by_func <T, func> ::type;

*/


