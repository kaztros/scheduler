#pragma once

#include <cstdint>
#include "stm32l4xx_hal.h"

bool IsIRQMode();

struct DisableInterruptLock {
  static unsigned int theReentryCounter;
  //^ Assumed that Interrupt Disabled Count will not mutate
  // while interrupts are enabled.

  DisableInterruptLock ();
  ~DisableInterruptLock ();
};
