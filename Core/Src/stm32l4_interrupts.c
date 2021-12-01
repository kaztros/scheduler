/*
 * stm32l4_interrupts.c
 *
 *  Created on: Oct 17, 2020
 *      Author: kaz
 */

#include "stm32l4xx_hal.h"

void blinky() {
  while (1)
  {
	/* USER CODE END WHILE */
	HAL_GPIO_TogglePin(GPIOB, 0x08);
	for (int idx=0; idx<1000000; idx++) ;
	/* USER CODE BEGIN 3 */
  }
}


void ADC1_2_IRQHandler() { blinky(); }
void BusFault_Handler() { blinky(); }
void COMP_IRQHandler() { blinky(); }
void CRS_IRQHandler() { blinky(); }
void DebugMon_Handler() { blinky(); }
//void Default_Handler() { blinky(); }
void DMA1_Channel1_IRQHandler() { blinky(); }
void DMA1_Channel2_IRQHandler() { blinky(); }
void DMA1_Channel3_IRQHandler() { blinky(); }
void DMA1_Channel4_IRQHandler() { blinky(); }
void DMA1_Channel5_IRQHandler() { blinky(); }
void DMA1_Channel6_IRQHandler() { blinky(); }
void DMA1_Channel7_IRQHandler() { blinky(); }
void DMA2_Channel1_IRQHandler() { blinky(); }
void DMA2_Channel2_IRQHandler() { blinky(); }
void DMA2_Channel3_IRQHandler() { blinky(); }
void DMA2_Channel4_IRQHandler() { blinky(); }
void DMA2_Channel5_IRQHandler() { blinky(); }
void DMA2_Channel6_IRQHandler() { blinky(); }
void DMA2_Channel7_IRQHandler() { blinky(); }
void EXTI0_IRQHandler() { blinky(); }
void EXTI1_IRQHandler() { blinky(); }
void EXTI15_10_IRQHandler() { blinky(); }
void EXTI2_IRQHandler() { blinky(); }
void EXTI3_IRQHandler() { blinky(); }
void EXTI4_IRQHandler() { blinky(); }
void EXTI9_5_IRQHandler() { blinky(); }
void FLASH_IRQHandler() { blinky(); }
void FPU_IRQHandler() { blinky(); }
void HardFault_Handler() { blinky(); }
void I2C1_ER_IRQHandler() { blinky(); }
void I2C1_EV_IRQHandler() { blinky(); }
void I2C2_ER_IRQHandler() { blinky(); }
void I2C2_EV_IRQHandler() { blinky(); }
void I2C3_ER_IRQHandler() { blinky(); }
void I2C3_EV_IRQHandler() { blinky(); }
void Infinite_Loop() { blinky(); }
void LPTIM1_IRQHandler() { blinky(); }
void LPTIM2_IRQHandler() { blinky(); }
void LPUART1_IRQHandler() { blinky(); }
void MemManage_Handler() { blinky(); }
void NMI_Handler() { blinky(); }
void PendSV_Handler() { blinky(); }
void PVD_PVM_IRQHandler() { blinky(); }
void QUADSPI_IRQHandler() { blinky(); }
void RCC_IRQHandler() { blinky(); }
void RNG_IRQHandler() { blinky(); }
void RTC_Alarm_IRQHandler() { blinky(); }
void RTC_WKUP_IRQHandler() { blinky(); }
void SPI1_IRQHandler() { blinky(); }
void SPI2_IRQHandler() { blinky(); }
void SVC_Handler() { blinky(); }

void SysTick_Handler() {
	/* This is enabled by default.  Intentionally do nothing. */
}

void TAMP_STAMP_IRQHandler() { blinky(); }
void TIM1_BRK_TIM15_IRQHandler() { blinky(); }
void TIM1_CC_IRQHandler() { blinky(); }
void TIM1_TRG_COM_IRQHandler() { blinky(); }
void TIM1_UP_TIM16_IRQHandler() { blinky(); }
void TIM2_IRQHandler() { blinky(); }
void TIM6_IRQHandler() { blinky(); }
void TSC_IRQHandler() { blinky(); }
void UsageFault_Handler() { blinky(); }
void USART1_IRQHandler() { blinky(); }
void USART2_IRQHandler() { blinky(); }
void USART3_IRQHandler() { blinky(); }
void USB_IRQHandler() { blinky(); }
void WWDG_IRQHandler() { blinky(); }


