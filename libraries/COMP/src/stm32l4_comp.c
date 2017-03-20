/*
 * Copyright (c) 2017 Nicholas Waterton.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include <stdio.h>

#include "stm32l4xx.h"

#include "armv7m.h"
#include "stm32l4_comp.h"

#ifdef __cplusplus
 extern "C" {
#endif


#if defined(STM32L432xx)
bool stm32l4_comp_pins_config(stm32l4_comp_t *comp) { //ladybug/NUCLEO 432
  int posinpin = comp->pins.posinpin;
  int neginput = comp->pins.neginput;
  
  COMP_TypeDef *_COMP = comp->COMPx;
  
  if (_COMP == COMP1) {
    switch (posinpin) {
      case GPIO_PIN_PA1:  _COMP->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA1
      default:
        return false;
    }
  }
  else {
    switch (posinpin) {
      case GPIO_PIN_PB4:  break;  //select plus input as 00 - PB4
      case GPIO_PIN_PB6_TIM16_CH1N:  _COMP->CSR |= COMP_CSR_INPSEL_0; break;
      #ifdef GPIO_PIN_PA3_TIM2_CH4
      case GPIO_PIN_PA3_TIM2_CH4:  _COMP->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3
      #else
      case GPIO_PIN_PA3:  _COMP->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3  //NUCLEO 432
      #endif
      default:
        return false;
    }
  }

  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode
  
  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _COMP->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_COMP == COMP1) {
      switch (neginput) {
        #ifdef GPIO_PIN_PB1_TIM1_CH3N
        case GPIO_PIN_PB1_TIM1_CH3N:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits... //NUCLEO 432
        #else
        case GPIO_PIN_PB1:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        #endif
        case GPIO_PIN_PA0:  _COMP->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _COMP->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _COMP->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        #ifdef GPIO_PIN_PB3_TIM2_CH2
        case GPIO_PIN_PB3_TIM2_CH2:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        #else
        case GPIO_PIN_PB3:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits... //NUCLEO 432
        #endif
        case GPIO_PIN_PB7:  break;
        #ifdef GPIO_PIN_PA2_TIM2_CH3
        case GPIO_PIN_PA2_TIM2_CH3:  _COMP->CSR |= COMP_CSR_INMESEL_0; break;
        #else
        case GPIO_PIN_PA2:  _COMP->CSR |= COMP_CSR_INMESEL_0; break;
        #endif
        case GPIO_PIN_PA4:  _COMP->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _COMP->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    #ifdef GPIO_PIN_PB1_TIM1_CH3N
    if(neginput != GPIO_PIN_PB1_TIM1_CH3N && neginput != GPIO_PIN_PB3)  //NUCLEO 432
    #else
    if(neginput != GPIO_PIN_PB1 && neginput != GPIO_PIN_PB3_TIM2_CH2)   //ladybug
    #endif
      _COMP->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2;  //set for GPIO input
  }
  return true;
}

bool stm32l4_comp_blanking(stm32l4_comp_t *comp, comp_blanking_t timer) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _COMP->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _COMP->CSR |= COMP_CSR_BLANKING_1; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_15:  _COMP->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}

#endif

#if defined(STM32L433xx)
bool stm32l4_comp_pins_config(stm32l4_comp_t *comp) { //butterfly
  int posinpin = comp->pins.posinpin;
  int neginput = comp->pins.neginput;
  COMP_TypeDef *_COMP = comp->COMPx;
  
  if (_COMP == COMP1) {
    switch (posinpin) {
      case GPIO_PIN_PB2:  _COMP->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB2
      case GPIO_PIN_PA1_TIM2_CH2:  _COMP->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA1
      default:
        return false;
    }
  }
  else {
    switch (posinpin) {
      case GPIO_PIN_PB4:  break;  //select plus input as 00 - PB4
      case GPIO_PIN_PB6:  _COMP->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB6
      case GPIO_PIN_PA3:  _COMP->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3
      default:
        return false;
    }
  }
  
  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode

  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _COMP->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_COMP == COMP1) {
      switch (neginput) {
        case GPIO_PIN_PB1:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        case GPIO_PIN_PA0:  _COMP->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _COMP->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _COMP->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        case GPIO_PIN_PB3:  _COMP->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        case GPIO_PIN_PB7:  break;
        case GPIO_PIN_PA2_TIM2_CH3:  _COMP->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _COMP->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _COMP->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    if(neginput != GPIO_PIN_PB1 && neginput != GPIO_PIN_PB3)
      _COMP->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2;  //set for GPIO input
  }
  return true;
}

bool stm32l4_comp_blanking(stm32l4_comp_t *comp, comp_blanking_t timer) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _COMP->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _COMP->CSR |= COMP_CSR_BLANKING_1; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_15:  _COMP->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}

#endif

#if defined(STM32L476xx)
bool stm32l4_comp_pins_config(stm32l4_comp_t *comp) { //dragonfly/NUCLEO 476 (NUCLEO 476 only has pins available for COMP2)
  int posinpin = comp->pins.posinpin;
  int neginput = comp->pins.neginput;
  
  COMP_TypeDef *_COMP = comp->COMPx;
  
  if (_COMP == COMP1) {
    switch (posinpin) {
      #ifdef GPIO_PIN_PC5
      case GPIO_PIN_PC5:  break;  //select plus input as 00 - PB5 //no PC5 on NUCLEO 476
      case GPIO_PIN_PB2:  _COMP->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB2 //no PB2 on NUCLEO 476
      #endif
      default:
        return false;
    }
  }
  else {
    switch (posinpin) {
      #ifdef GPIO_PIN_PB4_TIM3_CH1
      case GPIO_PIN_PB4_TIM3_CH1:  break; //NUCLEO 476
      #else
      case GPIO_PIN_PB4:  break;  //select plus input as 00 - PB4
      #endif
      #ifdef GPIO_PIN_PB6_TIM4_CH1
      case GPIO_PIN_PB6_TIM4_CH1:  _COMP->CSR |= COMP_CSR_INPSEL_0; break; //NUCLEO 476
      #else
      case GPIO_PIN_PB6:  _COMP->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB6
      #endif
      default:
        return false;
    }
  }

  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode
  
  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _COMP->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _COMP->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_COMP == COMP1) {
      switch (neginput) {
        #ifdef GPIO_PIN_PB1
        case GPIO_PIN_PB1:  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        case GPIO_PIN_PC4:  _COMP->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        #endif
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        #ifdef GPIO_PIN_PB3_TIM2_CH2
        case GPIO_PIN_PB3_TIM2_CH2:  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;  //NUCLEO 476
        #else
        case GPIO_PIN_PB3:  _COMP->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        case GPIO_PIN_PB7:  _COMP->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        #endif
        default:
          return false;
      }
    }
  }
  return true;
}

bool stm32l4_comp_blanking(stm32l4_comp_t *comp, comp_blanking_t timer) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _COMP->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _COMP->CSR |= COMP_CSR_BLANKING_1; break;
      case COMP_TIMER_3:  _COMP->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _COMP->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_3:  _COMP->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_8:  _COMP->CSR |= COMP_CSR_BLANKING_1; break;
      case COMP_TIMER_15:  _COMP->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}
#endif


void stm32l4_comp_polarity(stm32l4_comp_t *comp, bool inverted) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if(inverted)
    _COMP->CSR |= COMP_CSR_POLARITY; //invert polarity as we are triggering on a negative signal
  else
    _COMP->CSR &= ~COMP_CSR_POLARITY;
}

void stm32l4_comp_enable_device(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  _COMP->CSR |= COMP_CSR_EN;
}

void stm32l4_comp_disable_device(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  _COMP->CSR &= ~COMP_CSR_EN;
}

bool stm32l4_comp_read(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  return ((_COMP->CSR & COMP_CSR_VALUE) == COMP_CSR_VALUE);
}

bool stm32l4_comp_enabled(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  return ((_COMP->CSR & COMP_CSR_EN) == COMP_CSR_EN);
}
  
void stm32l4_comp_lock(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  _COMP->CSR |= COMP_CSR_LOCK;
}

bool stm32l4_comp_locked(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  return ((_COMP->CSR & COMP_CSR_LOCK) == COMP_CSR_LOCK);
}
  
void  stm32l4_comp_power_mode(stm32l4_comp_t *comp, comp_mode_t powermode) {
  COMP_TypeDef *_COMP = comp->COMPx;
  // high is default
  switch (powermode) {
    case HIGH_MODE:
      _COMP->CSR &= ~COMP_CSR_PWRMODE_0;
      _COMP->CSR &= ~COMP_CSR_PWRMODE_1; //00 = high power/speed mode
      break;
    case MED_MODE:
      _COMP->CSR |= COMP_CSR_PWRMODE_0;
      _COMP->CSR &= ~COMP_CSR_PWRMODE_1; //01 = med power/speed mode (or 10 is same)
      break;
    case LOW_MODE:
      _COMP->CSR |= COMP_CSR_PWRMODE_0;
      _COMP->CSR |= COMP_CSR_PWRMODE_1; //11 = ultra low power/speed mode
      break;
  }
}

void stm32l4_comp_hysteresis_mode(stm32l4_comp_t *comp, comp_mode_t hystmode) {
  COMP_TypeDef *_COMP = comp->COMPx;
  // none is default
  switch (hystmode) {
    case HIGH_MODE:
      _COMP->CSR |= COMP_CSR_HYST_0; //hysteresis low bit
      _COMP->CSR |= COMP_CSR_HYST_1; //hysteresis high bit - hysteresis = 00 (none), 01 (low), 10 (med), 11 (high)
      break;
    case MED_MODE:
      _COMP->CSR |= COMP_CSR_HYST_0;
      _COMP->CSR &= ~COMP_CSR_HYST_1;
      break;
    case LOW_MODE:
      _COMP->CSR &= ~COMP_CSR_HYST_0;
      _COMP->CSR |= COMP_CSR_HYST_1;
      break;
    case NONE_MODE:
      _COMP->CSR &= ~COMP_CSR_HYST_0;
      _COMP->CSR &= ~COMP_CSR_HYST_1;
      break;
  }
}

void stm32l4_comp_winmode(stm32l4_comp_t *comp, bool enable) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP != COMP2) //winmode only valid for COMP2 (except l0, where it's COMP1)
    return;
  if (enable)
    _COMP->CSR |= COMP_CSR_WINMODE;
  else
    _COMP->CSR &= ~COMP_CSR_WINMODE;
}

void stm32l4_comp_enable_event(stm32l4_comp_t *comp, uint32_t mode) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  // default is enable rising edge
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    EXTI->PR1 = EXTI_PR1_PIF21;   //Clear EXTI pending bit (if pending) by writing 1 for the bit to the register
    armv7m_atomic_or(&EXTI->EMR1, EXTI_EMR1_EM21);
  }
  else if (_COMP == COMP2) {
    EXTI->PR1 = EXTI_PR1_PIF22;   //Clear EXTI pending bit (if pending)
    armv7m_atomic_or(&EXTI->EMR1, EXTI_EMR1_EM22);
  }
  stm32l4_comp_trigger_edges(comp, mode);
}

void stm32l4_comp_disable_event(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    armv7m_atomic_and(&EXTI->EMR1, ~EXTI_EMR1_EM21);
  }
  else  if (_COMP == COMP2) {
    armv7m_atomic_and(&EXTI->EMR1, ~EXTI_EMR1_EM22);
  }
}

bool stm32l4_comp_event_enabled(stm32l4_comp_t *comp) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    return ((EXTI->EMR1 & EXTI_EMR1_EM21) == EXTI_EMR1_EM21); //Event line 21 is/not masked
  }
  else if (_COMP == COMP2) {
    return ((EXTI->EMR1 & EXTI_EMR1_EM22) == EXTI_EMR1_EM22); //Event line 22 is/not masked
  }
}

void stm32l4_comp_enable_interrupt(stm32l4_comp_t *comp, uint32_t mode, voidFuncPtr callback) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  // default is enable rising edge
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    EXTI->PR1 = EXTI_PR1_PIF21; //Clear EXTI pending bit (if pending)
    armv7m_atomic_or(&EXTI->IMR1, EXTI_IMR1_IM21);  //Interrupt line 21 not masked = enabled (not required for wake from sleep)
  }
  else if (_COMP == COMP2) {
    EXTI->PR1 = EXTI_PR1_PIF22; //Clear EXTI pending bit (if pending)
    armv7m_atomic_or(&EXTI->IMR1, EXTI_IMR1_IM22);  //Interrupt line 22 not masked = enabled (not required for wake from sleep)
  }
  stm32l4_comp_trigger_edges(comp, mode);
  
  stm32l4_comp_notify(comp, callback, comp->context, NULL);

  NVIC_SetPriority(comp->interrupt, comp->priority);
  NVIC_EnableIRQ(comp->interrupt);
}

void stm32l4_comp_disable_interrupt(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    armv7m_atomic_and(&EXTI->IMR1, ~EXTI_IMR1_IM21);  //Interrupt line 21 masked = disabled
  }
  else if (_COMP == COMP2) {
    armv7m_atomic_and(&EXTI->IMR1, ~EXTI_IMR1_IM22);  //Interrupt line 22 masked = disabled
  }
}

bool stm32l4_comp_interrupt_enabled(stm32l4_comp_t *comp) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    return ((EXTI->IMR1 & EXTI_IMR1_IM21) == EXTI_IMR1_IM21); //Event line 21 is/not masked
  }
  else if (_COMP == COMP2) {
    return ((EXTI->IMR1 & EXTI_IMR1_IM22) == EXTI_IMR1_IM22); //Event line 22 is/not masked
  }
}

void stm32l4_comp_interrupt(stm32l4_comp_t *comp) {
  if (comp->callback != NULL)
    comp->callback();
}

void stm32l4_comp_trigger_edges(stm32l4_comp_t *comp, uint32_t mode) {
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  COMP_TypeDef *_COMP = comp->COMPx;
  if (_COMP == COMP1) {
    if (mode == RISING || mode == CHANGE) {
      armv7m_atomic_or(&EXTI->RTSR1, EXTI_RTSR1_RT21);  //EXTI line 21 Rising edge Trigger enabled (depends on polarity)
    }
    else {
      EXTI->RTSR1 &= ~EXTI_RTSR1_RT21; //EXTI line 21 Rising edge Trigger disabled
      armv7m_atomic_and(&EXTI->RTSR1, ~EXTI_RTSR1_RT21);
    }
      
    if (mode == FALLING || mode == CHANGE) {
      armv7m_atomic_or(&EXTI->FTSR1, EXTI_FTSR1_FT21);  //EXTI line 21 Falling edge Trigger enabled (depends on polarity)
    }
    else {
      armv7m_atomic_and(&EXTI->FTSR1, ~EXTI_FTSR1_FT21);  //EXTI line 21 Falling edge Trigger disabled
    }
  }
  else if (_COMP == COMP2) {
    if (mode == RISING || mode == CHANGE) {
      armv7m_atomic_or(&EXTI->RTSR1, EXTI_RTSR1_RT22);  //EXTI line 22 Rising edge Trigger enabled (depends on polarity)
    }
    else {
      armv7m_atomic_and(&EXTI->RTSR1, ~EXTI_RTSR1_RT22);  //EXTI line 22 Rising edge Trigger disabled
    }
      
    if (mode == FALLING || mode == CHANGE) {
      armv7m_atomic_or(&EXTI->FTSR1, EXTI_FTSR1_FT22);  //EXTI line 22 Falling edge Trigger enabled (depends on polarity)
    }
    else {
      armv7m_atomic_and(&EXTI->FTSR1, ~EXTI_FTSR1_FT22);  //EXTI line 22 Falling edge Trigger disabled
    }
  }
}

bool stm32l4_comp_create(stm32l4_comp_t *comp, unsigned int instance, stm32l4_comp_pins_t *pins, unsigned int priority, unsigned int mode) {
 
  comp->state = COMP_STATE_INIT;
  comp->instance = instance;
  if (pins != NULL)
    comp->pins = *pins;
  comp->interrupt = COMP_IRQn;
  comp->callback = NULL;
  comp->priority = priority;
  comp->option = mode;
  
  stm32l4_comp_driver.instances[comp->instance] = comp;

  return true;
}

bool stm32l4_comp_destroy(stm32l4_comp_t *comp) {
    comp->instance == NULL;
    comp->state = COMP_STATE_NONE;
}

bool stm32l4_comp_enable(stm32l4_comp_t *comp, uint32_t option, stm32l4_comp_callback_t callback, void *context, uint32_t events) {
  if (comp->pins.posinpin == GPIO_PIN_NONE || comp->pins.neginput == GPIO_PIN_NONE) return false;
  
  COMP_TypeDef *_COMP = comp->COMPx;
  
  stm32l4_comp_driver.instances[comp->instance] = comp; //duplicate from create in case end is called
  comp->context = context;  //save this instance
  
  if((RCC->APB2ENR & RCC_APB2ENR_SYSCFGEN) != RCC_APB2ENR_SYSCFGEN)  //enable SYSCFG clock if disabled, as it is also used for COMP register access (not needed for comps to operate).
    armv7m_atomic_or(&RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);  //would use stm32l4_system_periph_enable(periph) but RCC_APB2ENR_SYSCFGEN is not defined in the structure, (and this is what it does anyway)
                                                            //do we need to wait for it to become enabled?

  _COMP->CSR = (uint32_t)0x00000000; //erase COMP settings
  if (!stm32l4_comp_pins_config(comp)) return false;
  
  if(option == 0 && comp->option != 0)    //if no options specified, but options specified in object creation, copy them over here.
    option = comp->option;
  
  stm32l4_comp_configure(comp, option);

  comp->state = COMP_STATE_READY;
  return true;
}

bool stm32l4_comp_disable(stm32l4_comp_t *comp) {
  COMP_TypeDef *_COMP = comp->COMPx;
  stm32l4_comp_disable_event(comp);   
  stm32l4_comp_disable_interrupt(comp);
  comp->option = 0; //erase any configured options
  _COMP->CSR = (uint32_t)0x00000000; //erase COMP settings
}

bool stm32l4_comp_configure(stm32l4_comp_t *comp, uint32_t option) {
    COMP_TypeDef *_COMP = comp->COMPx;
    
    comp->option = option;
    
    //invert output polarity
    if (option & COMP_INVERT)
      stm32l4_comp_polarity(comp, true);
    else
      stm32l4_comp_polarity(comp, false);
  
    //event enable
    if (option & COMP_EVENT_ENABLE)
      stm32l4_comp_enable_event(comp, RISING);  //rising edge trigger default
    else
      stm32l4_comp_disable_event(comp);
  
    //interrupt enable
    if (option & COMP_INTERRUPT_ENABLE)
      stm32l4_comp_enable_interrupt(comp, RISING, NULL);  //rising edge trigger default (no callback)
    else
      stm32l4_comp_disable_interrupt(comp);
    
    //power
    if (option & HIGH_POWER_MODE)
      stm32l4_comp_power_mode(comp, HIGH_MODE);
    if (option & MED_POWER_MODE)
      stm32l4_comp_power_mode(comp, MED_MODE);
    if (option & LOW_POWER_MODE)
      stm32l4_comp_power_mode(comp, LOW_MODE);
  
    //hysteresis
    if (option & HIGH_HYST_MODE)
      stm32l4_comp_hysteresis_mode(comp, HIGH_MODE);
    if (option & MED_HYST_MODE)
      stm32l4_comp_hysteresis_mode(comp, MED_MODE);
    if (option & LOW_HYST_MODE)
      stm32l4_comp_hysteresis_mode(comp, LOW_MODE);
    if (option & NONE_HYST_MODE)
      stm32l4_comp_hysteresis_mode(comp, NONE_MODE);
  
    //window comparator mode
    if (option & COMP_WINMODE)
      stm32l4_comp_winmode(comp, true);
    else
      stm32l4_comp_winmode(comp, false);
     
}

bool stm32l4_comp_notify(stm32l4_comp_t *comp, stm32l4_comp_callback_t callback, void *context, uint32_t events) {
    if(callback != NULL)
      comp->callback = callback;
    comp->context = context;

    return true;
}



void COMP_IRQHandler(void) {
  if ((EXTI->PR1 & EXTI_PR1_PIF21) == EXTI_PR1_PIF21) { //COMP1 triggered
     EXTI->PR1 = EXTI_PR1_PIF21;    //clear pending bit
     //armv7m_pendsv_enqueue((armv7m_pendsv_routine_t)stm32l4_comp_interrupt(stm32l4_comp_driver.instances[COMP_INSTANCE_COMP1], NULL, 0);
     stm32l4_comp_interrupt(stm32l4_comp_driver.instances[COMP_INSTANCE_COMP1]);
  }
  if ((EXTI->PR1 & EXTI_PR1_PIF22) == EXTI_PR1_PIF22) { //COMP2 triggered
     EXTI->PR1 = EXTI_PR1_PIF22;    //clear pending bit
     //armv7m_pendsv_enqueue((armv7m_pendsv_routine_t)stm32l4_comp_interrupt(stm32l4_comp_driver.instances[COMP_INSTANCE_COMP2]), NULL, 0);
     stm32l4_comp_interrupt(stm32l4_comp_driver.instances[COMP_INSTANCE_COMP2]);
  }
}


#ifdef __cplusplus
}
#endif

