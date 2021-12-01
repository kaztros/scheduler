#include "DisableInterruptLock.hpp"

bool IsIRQMode() {
  return 0 != (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);
}

unsigned int DisableInterruptLock::theReentryCounter = 0;

DisableInterruptLock::DisableInterruptLock () {
  /* Disable interrupts */
  __disable_irq();
  
  /* Increase number of disable interrupt function calls */
  ++theReentryCounter;
}

DisableInterruptLock::~DisableInterruptLock () {
  --theReentryCounter;
  
  /* Check if we are the last re-entrant. */
  if (0 == theReentryCounter) {
    /* Enable interrupts */
    __enable_irq();
  }
}
