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
/// @brief CRTP provider of assignment to/from volatile.
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
///@brief Container for mmap'd register, with _raw member.
template <typename T>
struct volatile_but_raw_c
: public weave_volatile_t <T>
{
  volatile_but_raw_c () noexcept {}
  
  ///@note Return nothing, as GCC writes an essay of warnings.
  void operator= (T const & rhs) { this->_raw = rhs._raw; }

  operator T () { T copy; copy._raw = this->_raw; return copy;  }
};

template <typename T, std::size_t N>
struct volatile_but_raw_c <std::array <T, N>>
: public std::array <volatile_but_raw_c<T>, N>
{
  using U = std::array <volatile_but_raw_c<T>, N>;
  
  volatile_but_raw_c () noexcept {}

  [[deprecated("Hefty operation.  Are you sure?")]]
  void operator= (U const & rhs) { this->_raw = rhs._raw; }

  [[deprecated("Hefty operation.  Are you sure?")]]
  operator U () { U copy; copy._raw = this->_raw; return copy;  }
};

/*----------------------------------------------------------------------------*/
template <typename T>
struct is_volatile_but_raw
: std::integral_constant <bool, false>
{};

template <typename T>
struct is_volatile_but_raw <volatile_but_raw_c <T>>
: std::integral_constant <bool, true>
{};

template <typename T>
inline constexpr bool is_volatile_but_raw_v = is_volatile_but_raw <T>::value;

/*----------------------------------------------------------------------------*/
template <typename src_t, typename dst_t>
struct imply_volatile_but_raw {
  using type = std::conditional_t
  < is_volatile_but_raw_v <src_t>
  , volatile_but_raw_c <dst_t>
  , dst_t
  >;
};

template <typename src_t, typename T>
struct imply_volatile_but_raw <src_t, volatile_but_raw_c<T>> {
  using type = volatile_but_raw_c <T>;
};

template <typename src_t, typename dst_t>
using imply_volatile_but_raw_t
= typename imply_volatile_but_raw <src_t, dst_t> ::type;

/*----------------------------------------------------------------------------*/
//@brief new-type to distinguish transactions.
template <typename REGISTER_T>
struct transaction_t {
  REGISTER_T transaction;
  REGISTER_T original;
  
  transaction_t (REGISTER_T snapshot) noexcept;
};


///@brief Container for registers that use transactions of the same type.
///@note This is an advanced form of volatile, where even assignment can't
///guarantee equvalence.
template <typename REGISTER_T>
struct transactive_t
: public weave_volatile_t <REGISTER_T>
{
  //Do not allow assignment between the same types:
  transactive_t () = default;
  transactive_t (transactive_t <REGISTER_T> const &) = delete;
  transactive_t (transactive_t <REGISTER_T> &&) = delete;
  transactive_t & operator= (transactive_t <REGISTER_T> const &) = delete;
  transactive_t & operator= (transactive_t <REGISTER_T> &&) = delete;

  //Allow manual static-cast copy.
  REGISTER_T operator() () volatile {
    return *this;
  }
  
  //Allow automatic static-cast copy.
  operator REGISTER_T () volatile {
    return *this;
  }
  
  transaction_t <REGISTER_T> operator++() volatile { return transaction_t (raw_snapshot_of <REGISTER_T> (*this)); }

  ///@note GCC spits out large essays when this returns a volatile reference.
  void operator= (transaction_t <REGISTER_T> x) volatile {
    this->_raw = x.transaction._raw;
    //return *this;
  }
  
  ///@note Because of the volatile variant, this one returns void too.
  void operator= (transaction_t <REGISTER_T> x) {
    this->_raw = x.transaction._raw;
    //return *this;
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


