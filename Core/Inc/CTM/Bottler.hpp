#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Bottling {

/* DO NOT CALL */
void __attribute__ ((noinline)) reentry_4_args (void* stack_pointer);

struct Cap {
  void* stack_pointer;
  /* argument to uncapping_routine */
  void (*reentry_routine)(void*);
  /* Having a variable uncapping routine allows for faster-switches */
};

void __attribute__ ((noinline)) make_vfpless_cap_and_unbottle (Cap* cap_to_make , Cap* cap_to_unbottle);
/* Hope that no-inline prevents LTO optimization */

void __attribute__ ((noinline)) make_vfp_cap_and_unbottle (Cap* cap_to_make , Cap* cap_to_unbottle);
/* Hope that no-inline prevents LTO optimization */


template <size_t N>
void __attribute__ ((noinline)) cap_starter_with_arg_count (void* stack_pointer) {
  asm ("  mov  sp, %0    \r\n"
       "  pop  {r0-r3, lr}  "
      :
      : "r" (stack_pointer)
      );
  //bx lr  <- implied.
}

template <> void __attribute__ ((noinline)) cap_starter_with_arg_count<0> (void* stack_pointer);
template <> void __attribute__ ((noinline)) cap_starter_with_arg_count<1> (void* stack_pointer);
template <> void __attribute__ ((noinline)) cap_starter_with_arg_count<2> (void* stack_pointer);
template <> void __attribute__ ((noinline)) cap_starter_with_arg_count<3> (void* stack_pointer);


template <typename T>
auto as_register_value (T& x) {
  //Catches l-value references
  if (std::is_pointer_v <T>) {
    return reinterpret_cast <void*> (x);
  } else if (std::is_class_v <T>) {
    return reinterpret_cast <void*> (&x);
  } else if (sizeof(T) <= sizeof(void*)) {
    return reinterpret_cast <void*> (x);
  }
}


template <typename ... TRUE_ARGS_T, typename ... ARGS_T>
Cap make_cap_of (void* (stack_pointer)[], void (*fptr)(TRUE_ARGS_T...), ARGS_T...args) {
  /* FRAGILE.  But the 4-argument case inducts all other cases. */
  constexpr auto ARG_C = sizeof...(TRUE_ARGS_T);
  static_assert (ARG_C <= 4);
  static_assert (sizeof...(ARGS_T) <= 4);
  static_assert (ARG_C == sizeof...(ARGS_T));

  stack_pointer = &stack_pointer [-sizeof...(TRUE_ARGS_T) - 1];
  auto stack_pointer_for_cap = stack_pointer;
  
  auto push_to_stack = [&stack_pointer] (void* x) {
    *stack_pointer++ = x;
  };
  (push_to_stack (as_register_value (args)), ...);
  push_to_stack (as_register_value (fptr));
  
  return Cap {stack_pointer_for_cap, & cap_starter_with_arg_count <ARG_C> };
}

}//end namespace Bottling
