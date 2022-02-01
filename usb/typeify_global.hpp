#pragma once
#include <array>
#include <tuple>
#include "type_traits_kaz.hpp"

/// "typeify" creates a constexpr type parameter referencing a global variable.
///
/// "Why not use <auto & ref>"
/// ref can be bound to a global
///   but not to a member of a global
///   nor to a base-class of a global
///   nor to a index of a global
/// I'd like it if my templates were compatible with my habitual encapsulation.
/// "Why use <auto & ref> in the first place?"
/// - I want to force inlines from the compiler.
/// - I need a type-erased array of function pointers.
/// - I want a zero-length decorator with guaranteed constexpr parameters, which
///   references a memory-mapped register elsewhere, despite being 0-length.
///   These invariant parameters lend to highly specialized code that's small,
///   fast, and can detect invalid configurations at compile-time.  But I NEED
///   for C++'s template arguments to not be incompatible with C++'s
///   encapsulation.

/// Example Code:
/// @code{.cpp}
/// #include <iostream>
/// #include <string>
///
/// template <typename typefied>
/// auto & foo () { std::cout << *typefied() << std::endl; }
///
/// template <auto & ref>
/// auto & bar () { std::cout << ref << std::endl; }
///
/// struct translations {
///   std::string english [2];  //0 for UK, 1 for American
///   std::string french;
///   std::string dutch;
///   std::string spanish;
///   float translations_count;
/// };
///
/// extern translations greetings;
/// void (*greeting_jump_table) (void) [] =
/// { & foo <typeify<greetings>::member<&translations::english>::index<0> >    // "ello there guv"
/// , & foo <typeify<greetings>::member<&translations::english>::index<1> >    // "ey I'm waalking here"
/// , & foo <typeify<greetings>::member<&translations::spanish> >    // "Hola"
/// , & foo <typeify<greetings>::translations_count>  // "4.5"
/// //, & bar <greetings.dutch> -- invalid template argument.
/// };
///

/*----------------------------------------------------------------------------*/
/// @brief Provides accessor type-methods (member, index, base_member) for self_t.
/// This is forward declared because every type provided by typefied_methods
/// provides it's own typefied_methods.  But template specializations are used
/// to prevent nonsense (e.g. ::member<> when dream_t is a POD-type).
///
/// @param[in] dream_t The type when an instance of self_t is dereferenced.
/// @param[in] self_t The typefied reference to a global-accessible.
template <typename dream_t, typename self_t>
struct typefied_methods;
/*----------------------------------------------------------------------------*/
/// @brief A typefied reference to a base-class of a typefied reference.
/// @param base_t The intended base-class.
/// @param typefied_t The typefied reference to a global-accessible derived class.
template <typename base_t, typename typefied_t>
struct base_of_typefied
: public typefied_methods <base_t, base_of_typefied <base_t, typefied_t> >
{
  constexpr operator base_t &   () noexcept { return static_cast <base_t &> (*typefied_t()); }
  constexpr base_t & operator * () noexcept { return static_cast <base_t &> (*typefied_t()); }
};

///@brief A typefied reference to a reinterpretation of a typeified reference.
template <typename reint_t, typename typefied_t>
struct reinterpreted_of_typefied
: public typefied_methods <reint_t, reinterpreted_of_typefied <reint_t, typefied_t> >
{
  constexpr operator reint_t &   () noexcept { return reinterpret_cast <reint_t &> ( *typefied_t());  }
  constexpr reint_t & operator * () noexcept { return reinterpret_cast <reint_t &> ( *typefied_t());  }
};

/// @brief A typefied reference to a member of a typefied reference.
/// @param member_t The type of the member.
/// @param struct_t The type of the encapsulating struct.
/// @param m_ptr The pointer to the member of the struct.
/// @param typefied_t The typefied reference to a global-accessible class/union.
template
< typename member_t
, typename struct_t
, member_t struct_t::*m_ptr
, typename typefied_t
>
struct member_of_typefied
: public typefied_methods <member_t, member_of_typefied <member_t, struct_t, m_ptr, typefied_t>>
{
  constexpr operator member_t &   () noexcept { return (*typefied_t()).*m_ptr; }
  constexpr member_t & operator * () noexcept { return (*typefied_t()).*m_ptr; }
};

/// @brief A typefied reference to an index of a typefied reference.
/// @param element_t The type expected after indexing.
/// @param index The index.
/// @param typefied_t The typefied reference to a global-accessible tuple/array.
template <typename element_t, std::size_t index, typename typefied_t>
struct index_of_typefied
: public typefied_methods <element_t, index_of_typefied <element_t, index, typefied_t> >
{
  constexpr operator element_t &   () noexcept { return getm <index> (*typefied_t()); }
  constexpr element_t & operator * () noexcept { return getm <index> (*typefied_t()); }
};

/*----------------------------------------------------------------------------*/
/// @brief inheritable ::index<> method, disabled by default
template <typename dream_t, typename self_t>
struct typefied_indexable
{ /* DISABLED */ };

/// @brief inheritable ::index<> method, enable c-array case.
template <typename dream_t, std::size_t N, typename self_t>
struct typefied_indexable <dream_t [N], self_t> {
  template <std::size_t index>
  using index = index_of_typefied <dream_t, index, self_t>;
};

/// @brief inheritable ::index<> method, enable std::tuple case.
template <typename...dreams_t, typename self_t>
struct typefied_indexable <std::tuple <dreams_t...>, self_t> {
  template <std::size_t index>
  using index = index_of_typefied
    <std::tuple_element_t <index, std::tuple <dreams_t...>>, index, self_t>;
};

/// @brief inheritable ::index<> method, enable std::array case.
template <typename dream_t, std::size_t N, typename self_t>
struct typefied_indexable <std::array <dream_t, N>, self_t> {
  template <std::size_t index>
  using index = index_of_typefied <dream_t, index, self_t>;
};

/*----------------------------------------------------------------------------*/
/// @brief inheritable ::member<> method.  Enabled if dream_t is a union/class.
template
< typename dream_t, typename self_t
, bool enabled = std::is_class_v <std::remove_reference_t <dream_t>>
    || std::is_union_v <std::remove_reference_t <dream_t>>
>
struct typefied_memberable {
  using refless_t = std::remove_reference_t <dream_t>;
  //^ This can't be in the template parameters, due to a g++ (20200110) bug.

  template <auto refless_t::*mm_ptr>
  using member = member_of_typefied
    < typename imply_cv <dream_t, typename decltype(dissect(mm_ptr))::member_t> ::type
    , refless_t
    , mm_ptr
    , self_t
    >;
};

/// @brief template specialization to disable non-unions and non-classes.
template <typename dream_t, typename self_t>
struct typefied_memberable <dream_t, self_t, false>
{ /* DISABLED */ };

/*----------------------------------------------------------------------------*/
/// @brief Inheritable, by CRTP, type-methods for {member, index, base_member}
/// @note Some methods are inherited by template, to prevent invalid
/// instantiations by-way-of dead-end specializations.  C++ really hates
/// things like "auto float::*x", since float can't have pointers to members.
/// But I got lazy around base_member, because it's trivial to debug.
template <typename dream_t, typename self_t>
struct typefied_methods
: public typefied_indexable <dream_t, self_t>
, public typefied_memberable <dream_t, self_t>
{
  using DREAM_T = dream_t;

  template <typename base_t>
  using base_member = base_of_typefied <imply_cv_t <dream_t, base_t>, self_t>;

  template <typename same_t>
  using static_casted = base_of_typefied <imply_cv_t <dream_t, same_t>, self_t>;
  
  template <typename reint_t>
  using reinterpreted = reinterpreted_of_typefied
    <imply_cv_t <dream_t, reint_t>, self_t>;
};

/*----------------------------------------------------------------------------*/
/// @brief A reference to a global masquerading as a type.
/// @note All possible references start from here, to go around the global.
template <auto & x>
struct typeify
: public typefied_methods <std::remove_reference_t <decltype(x)>, typeify <x>>
{
  using DREAM_T = std::remove_reference_t <decltype(x)>;

  operator DREAM_T &   () { return x; }
  DREAM_T & operator * () { return x; }
};
