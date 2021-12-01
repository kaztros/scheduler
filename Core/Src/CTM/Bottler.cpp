#include "CTM/Bottler.hpp"

namespace Bottling {


template <> void cap_starter_with_arg_count<0> (void* stack_pointer) {
  asm ("  mov  sp, %0    \r\n"
       "  pop  {lr}  "
      :
      : "r" (stack_pointer)
      );
  //bx lr  <- implied.
}

template <> void cap_starter_with_arg_count<1> (void* stack_pointer) {
  asm ("  mov  sp, %0    \r\n"
       "  pop  {r0, lr}  "
      :
      : "r" (stack_pointer)
      );
  //bx lr  <- implied.
}

template <> void cap_starter_with_arg_count<2> (void* stack_pointer) {
  asm ("  mov  sp, %0    \r\n"
       "  pop  {r0-r1, lr}  "
      :
      : "r" (stack_pointer)
      );
  //bx lr  <- implied.
}

template <> void cap_starter_with_arg_count<3> (void* stack_pointer) {
  asm ("  mov  sp, %0    \r\n"
       "  pop  {r0-r2, lr}  "
      :
      : "r" (stack_pointer)
      );
  //bx lr  <- implied.
}

/* DO NOT CALL */
void __attribute__ ((noinline)) reentry_vfpless (void* stack_pointer) {
  asm ("  mov  sp, %0 "
      :
      : "r" (stack_pointer)
      );
  asm ("  pop  {r4-r11, ip, lr} ");
  //bx lr  <- implied.
}


void __attribute__ ((noinline)) make_vfpless_cap_and_unbottle (Cap* cap_to_make , Cap* cap_to_unbottle) {
  asm ("  push {r4-r11, ip, lr}");
  uint32_t * stack_pointer;
  asm ("  mov %0, sp"
      : "=r" (stack_pointer)
      );
  cap_to_make->stack_pointer = stack_pointer;
  cap_to_make->reentry_routine = &reentry_vfpless;

  asm ("  ldrd r0, r1, [%0]  \r\n"
       "  bx   r1                "
      :
      : "r" (cap_to_unbottle)
      );
}

}//end namespace Bottler


