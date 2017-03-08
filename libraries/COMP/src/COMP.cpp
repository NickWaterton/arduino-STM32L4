/*
 * Copyright (c) 2017 Nick Waterton.  All rights reserved.
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

#include "Arduino.h"
#include "stm32l4_wiring_private.h"
#include "COMP.h"

//#define Serial Serial1
//#define BITS32 (0x80000000U)  //1 followed by 31 0's
//#define PRREGBIN(x) Serial.print(#x" 0x"); Serial.println((x|BITS32),BIN)

voidFuncPtr COMP_CALLBACK[2];

COMPClass::COMPClass(COMP_TypeDef *comp, int posinpin, int neginput, bool invert, bool events, bool interrupt) {
  _comp = comp;
  if (_comp == COMP1) {
    this_comp = 1;
    COMP_CALLBACK[0] = NULL;
  }
  else if (_comp == COMP2) {
    this_comp = 2;
    COMP_CALLBACK[1] = NULL;
  }
  init(posinpin, neginput, invert, events, interrupt);
}

bool COMPClass::init(int posinpin, int neginput, bool invert, bool events, bool interrupt) {
  if (posinpin == GPIO_PIN_NONE || neginput == GPIO_PIN_NONE || locked()) return false;

  posinpin = g_APinDescription[posinpin].pin;
  _comp->CSR = (uint32_t)0x00000000; //erase COMP settings
  
  if (!config_stm32l4xx_comp_pins(posinpin, neginput))
    return false;
  
  polarity(invert);

  if (events)
    enable_event(RISING); //enable rising edge by default
    
  if (interrupt)
    enable_interrupt(RISING); //enable rising edge by default

  _init = true;
  
  return true;
}

#if defined(STM32L432xx)
bool COMPClass::config_stm32l4xx_comp_pins(int posinpin, int neginput) { //ladybug/NUCLEO 432
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (posinpin) {
      case GPIO_PIN_PA1:  _comp->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA1
      default:
        return false;
    }
  }
  else {
    switch (posinpin) {
      case GPIO_PIN_PB4:  break;  //select plus input as 00 - PB4
      case GPIO_PIN_PB6_TIM16_CH1N:  _comp->CSR |= COMP_CSR_INPSEL_0; break;
      #ifdef GPIO_PIN_PA3_TIM2_CH4
      case GPIO_PIN_PA3_TIM2_CH4:  _comp->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3
      #else
      case GPIO_PIN_PA3:  _comp->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3  //NUCLEO 432
      #endif
      default:
        return false;
    }
  }

  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode
  
  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _comp->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_comp == COMP1) {
      switch (neginput) {
        #ifdef GPIO_PIN_PB1_TIM1_CH3N
        case GPIO_PIN_PB1_TIM1_CH3N:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits... //NUCLEO 432
        #else
        case GPIO_PIN_PB1:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        #endif
        case GPIO_PIN_PA0:  _comp->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _comp->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _comp->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        #ifdef GPIO_PIN_PB3_TIM2_CH2
        case GPIO_PIN_PB3_TIM2_CH2:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        #else
        case GPIO_PIN_PB3:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits... //NUCLEO 432
        #endif
        case GPIO_PIN_PB7:  break;
        #ifdef GPIO_PIN_PA2_TIM2_CH3
        case GPIO_PIN_PA2_TIM2_CH3:  _comp->CSR |= COMP_CSR_INMESEL_0; break;
        #else
        case GPIO_PIN_PA2:  _comp->CSR |= COMP_CSR_INMESEL_0; break;
        #endif
        case GPIO_PIN_PA4:  _comp->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _comp->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    #ifdef GPIO_PIN_PB1_TIM1_CH3N
    if(neginput != GPIO_PIN_PB1_TIM1_CH3N && neginput != GPIO_PIN_PB3)  //NUCLEO 432
    #else
    if(neginput != GPIO_PIN_PB1 && neginput != GPIO_PIN_PB3_TIM2_CH2)   //ladybug
    #endif
      _comp->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2;  //set for GPIO input
  }
  return true;
}

bool COMPClass::blanking(comp_blanking_t timer) {
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _comp->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _comp->CSR |= COMP_CSR_BLANKING_1; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_15:  _comp->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}

#endif

#if defined(STM32L433xx)
bool COMPClass::config_stm32l4xx_comp_pins(int posinpin, int neginput) { //butterfly
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (posinpin) {
      case GPIO_PIN_PB2:  _comp->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB2
      case GPIO_PIN_PA1_TIM2_CH2:  _comp->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA1
      default:
        return false;
    }
  }
  else {
    switch (posinpin) {
      case GPIO_PIN_PB4:  break;  //select plus input as 00 - PB4
      case GPIO_PIN_PB6:  _comp->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB6
      case GPIO_PIN_PA3:  _comp->CSR |= COMP_CSR_INPSEL_1; break;  //select plus input as 10 - PA3
      default:
        return false;
    }
  }

  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode
  
  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _comp->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_comp == COMP1) {
      switch (neginput) {
        case GPIO_PIN_PB1:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        case GPIO_PIN_PA0:  _comp->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _comp->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _comp->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        case GPIO_PIN_PB3:  _comp->CSR |= COMP_CSR_INMSEL_2 | COMP_CSR_INMSEL_1; break; //NOTE: not using extended bits...
        case GPIO_PIN_PB7:  break;
        case GPIO_PIN_PA2_TIM2_CH3:  _comp->CSR |= COMP_CSR_INMESEL_0; break;
        case GPIO_PIN_PA4:  _comp->CSR |= COMP_CSR_INMESEL_1; break;
        case GPIO_PIN_PA5:  _comp->CSR |= COMP_CSR_INMESEL_0 | COMP_CSR_INMESEL_1; break;
        default:
          return false;
      }
    }
    if(neginput != GPIO_PIN_PB1 && neginput != GPIO_PIN_PB3)
      _comp->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2;  //set for GPIO input
  }
  return true;
}

bool COMPClass::blanking(comp_blanking_t timer) {
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _comp->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _comp->CSR |= COMP_CSR_BLANKING_1; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_15:  _comp->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}

#endif

#if defined(STM32L476xx)
bool COMPClass::config_stm32l4xx_comp_pins(int posinpin, int neginput) { //dragonfly/NUCLEO 476 (NUCLEO 476 only has pins available for COMP2)
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (posinpin) {
      #ifdef GPIO_PIN_PC5
      case GPIO_PIN_PC5:  break;  //select plus input as 00 - PB5 //no PC5 on NUCLEO 476
      case GPIO_PIN_PB2:  _comp->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB2 //no PB2 on NUCLEO 476
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
      case GPIO_PIN_PB6_TIM4_CH1:  _comp->CSR |= COMP_CSR_INPSEL_0; break; //NUCLEO 476
      #else
      case GPIO_PIN_PB6:  _comp->CSR |= COMP_CSR_INPSEL_0; break;  //select plus input as 01 - PB6
      #endif
      default:
        return false;
    }
  }

  stm32l4_gpio_pin_configure(posinpin, (GPIO_PUPD_NONE | GPIO_MODE_ANALOG)); //configure pin for analog input mode
  
  //configure negative input
  if (neginput < 0) { //internal negative input
    switch (neginput) {      
      case COMP_VINTREF_1_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN; break;  //select minus input as 1/4 vintref (000)
      case COMP_VINTREF_1_2:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_0; break; //select minus input as 1/2 vintref (001)
      case COMP_VINTREF_3_4:  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_BRGEN | COMP_CSR_INMSEL_1; break; //select minus input as 3/4 vintref (010)
      case COMP_VINTREF    :  _comp->CSR |= COMP_CSR_SCALEN | COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1; break;  //select minus input as vintref (011)
      case COMP_DAC1       :  _comp->CSR |= COMP_CSR_INMSEL_2; break; //(100)
      case COMP_DAC2       :  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break; //(110)
      default:
        return false;
    }
    PRREGBIN(COMP2->CSR);
  }
  else {  //select GPIO neg input
    neginput = g_APinDescription[neginput].pin;
    if (_comp == COMP1) {
      switch (neginput) {
        #ifdef GPIO_PIN_PB1
        case GPIO_PIN_PB1:  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        case GPIO_PIN_PC4:  _comp->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        #endif
        default:
          return false;
      }
    }
    else {
      switch (neginput) {
        #ifdef GPIO_PIN_PB3_TIM2_CH2
        case GPIO_PIN_PB3_TIM2_CH2:  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;  //NUCLEO 476
        #else
        case GPIO_PIN_PB3:  _comp->CSR |= COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        case GPIO_PIN_PB7:  _comp->CSR |= COMP_CSR_INMSEL_0 | COMP_CSR_INMSEL_1 | COMP_CSR_INMSEL_2; break;
        #endif
        default:
          return false;
      }
    }
  }
  return true;
}

bool COMPClass::blanking(comp_blanking_t timer) {
  if (locked()) return false; //check lock bit
  if (_comp == COMP1) {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_1:  _comp->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_2:  _comp->CSR |= COMP_CSR_BLANKING_1; break;
      case COMP_TIMER_3:  _comp->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  else {
    switch (timer) {
      case COMP_TIMER_NONE:  _comp->CSR &= ~COMP_CSR_BLANKING; break;
      case COMP_TIMER_3:  _comp->CSR |= COMP_CSR_BLANKING_0; break;
      case COMP_TIMER_8:  _comp->CSR |= COMP_CSR_BLANKING_1; break;
      case COMP_TIMER_15:  _comp->CSR |= COMP_CSR_BLANKING_2; break;
      default:
        return false;
    }
  }
  return true;
}

#endif

void COMPClass::polarity(bool inverted) {
  if (locked()) return; //check lock bit
  if(inverted)
    _comp->CSR |= COMP_CSR_POLARITY; //invert polarity as we are triggering on a negative signal
  else
    _comp->CSR &= ~COMP_CSR_POLARITY;
}

void COMPClass::enable() {
  if (locked() || !_init) return; //check lock bit & input has been configured
  _comp->CSR |= COMP_CSR_EN;
}

void COMPClass::disable() {
  if (locked()) return; //check lock bit
  _comp->CSR &= ~COMP_CSR_EN;
}


bool COMPClass::read() {
  return ((_comp->CSR & COMP_CSR_VALUE) == COMP_CSR_VALUE);
}

bool COMPClass::enabled() {
  return ((_comp->CSR & COMP_CSR_EN) == COMP_CSR_EN);
}

void COMPClass::lock() {
  if (locked()) return; //check lock bit
  _comp->CSR |= COMP_CSR_LOCK;
}

bool COMPClass::locked() {
  return ((_comp->CSR & COMP_CSR_LOCK) == COMP_CSR_LOCK);
}


void COMPClass::power_mode(comp_mode_t powermode) {
  if (locked()) return; //check lock bit
  // high is default
  switch (powermode) {
    case HIGH_MODE:
      _comp->CSR &= ~COMP_CSR_PWRMODE_0;
      _comp->CSR &= ~COMP_CSR_PWRMODE_1; //00 = high power/speed mode
      break;
    case MED_MODE:
      _comp->CSR |= COMP_CSR_PWRMODE_0;
      _comp->CSR &= ~COMP_CSR_PWRMODE_1; //01 = med power/speed mode (or 10 is same)
      break;
    case LOW_MODE:
      _comp->CSR |= COMP_CSR_PWRMODE_0;
      _comp->CSR |= COMP_CSR_PWRMODE_1; //11 = ultra low power/speed mode
      break;
  }
}

void COMPClass::hysteresis_mode(comp_mode_t hystmode) {
  if (locked()) return; //check lock bit
  // none is default
  switch (hystmode) {
    case HIGH_MODE:
      _comp->CSR |= COMP_CSR_HYST_0; //hysteresis low bit
      _comp->CSR |= COMP_CSR_HYST_1; //hysteresis high bit - hysteresis = 00 (none), 01 (low), 10 (med), 11 (high)
      break;
    case MED_MODE:
      _comp->CSR |= COMP_CSR_HYST_0;
      _comp->CSR &= ~COMP_CSR_HYST_1;
      break;
    case LOW_MODE:
      _comp->CSR &= ~COMP_CSR_HYST_0;
      _comp->CSR |= COMP_CSR_HYST_1;
      break;
    case NONE_MODE:
      _comp->CSR &= ~COMP_CSR_HYST_0;
      _comp->CSR &= ~COMP_CSR_HYST_1;
      break;
  }
}

void COMPClass::winmode(bool enable) {
  if (locked()) return; //check lock bit
  if (_comp != COMP2) //winmode only valid for COMP2
    return;
  if (enable)
    _comp->CSR |= COMP_CSR_WINMODE;
  else
    _comp->CSR &= ~COMP_CSR_WINMODE;
}

void COMPClass::enable_event(uint32_t mode) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  // default is enable rising edge
  if (locked()) return; //check lock bit
  if (_comp == COMP1) {
    EXTI->PR1 = EXTI_PR1_PIF21;   //Clear EXTI pending bit (if pending) by writing 1 for the bit to the register
    EXTI->EMR1 |= EXTI_EMR1_EM21; //Event line 21 not masked (enabled) //do we need armv7m_atomic_or / armv7m_atomic_and here?
  }
  else if (_comp == COMP2) {
    EXTI->PR1 = EXTI_PR1_PIF22;   //Clear EXTI pending bit (if pending)
    EXTI->EMR1 |= EXTI_EMR1_EM22; //Event line 22 not masked (enabled)  
  }
  trigger_edges(mode);
}

void COMPClass::disable_event() {
  if (locked()) return; //check lock bit
  if (_comp == COMP1) {
    EXTI->EMR1 &= ~EXTI_EMR1_EM21; //Event line 21 masked (disabled) //do we need armv7m_atomic_or / armv7m_atomic_and here?
  }
  else  if (_comp == COMP2) {
    EXTI->EMR1 &= ~EXTI_EMR1_EM22; //Event line 22 masked (disabled)
  }
}

bool COMPClass::event_enabled() {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  if (_comp == COMP1) {
    return ((EXTI->EMR1 & EXTI_EMR1_EM21) == EXTI_EMR1_EM21); //Event line 21 is/not masked
  }
  else if (_comp == COMP2) {
    return ((EXTI->EMR1 & EXTI_EMR1_EM22) == EXTI_EMR1_EM22); //Event line 22 is/not masked
  }
}

void COMPClass::enable_interrupt(uint32_t mode, voidFuncPtr callback) {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  // default is enable rising edge 
  if (_comp == COMP1) {
    COMP_CALLBACK[0] = callback;  //assign callback
    EXTI->PR1 = EXTI_PR1_PIF21; //Clear EXTI pending bit (if pending)
    EXTI->IMR1 |= EXTI_IMR1_IM21; //Interrupt line 21 not masked = enabled (not required for wake from sleep) //do we need armv7m_atomic_or / armv7m_atomic_and here?
  }
  else if (_comp == COMP2) {
    COMP_CALLBACK[1] = callback;  //assign callback
    EXTI->PR1 = EXTI_PR1_PIF22; //Clear EXTI pending bit (if pending)
    EXTI->IMR1 |= EXTI_IMR1_IM22; //Interrupt line 22 not masked = enabled (not required for wake from sleep)
  }
  trigger_edges(mode);

  NVIC_EnableIRQ(COMP_IRQn);
}

void COMPClass::disable_interrupt() {
  if (_comp == COMP1) {
    EXTI->IMR1 &= ~EXTI_IMR1_IM21; //Interrupt line 21 masked = disabled //do we need armv7m_atomic_or / armv7m_atomic_and here?
  }
  else if (_comp == COMP2) {
    EXTI->IMR1 &= ~EXTI_IMR1_IM22; //Interrupt line 22 masked = disabled
  }
  //NVIC_DisableIRQ(COMP_IRQn); //disables interrupt for both COMP1 and COMP2
}

bool COMPClass::interrupt_enabled() {
  // EXTI interrupt/event enable, COMP2 is on EXTI line 22, COMP1 is on line 21
  if (_comp == COMP1) {
    return ((EXTI->IMR1 & EXTI_IMR1_IM21) == EXTI_IMR1_IM21); //Event line 21 is/not masked
  }
  else if (_comp == COMP2) {
    return ((EXTI->IMR1 & EXTI_IMR1_IM22) == EXTI_IMR1_IM22); //Event line 22 is/not masked
  }
}

void COMPClass::trigger_edges(uint32_t mode) {
  // NOTE: rising/falling edge trigger is common for both interrupts and events
  if (_comp == COMP1) {
    if (mode == RISING || mode == CHANGE)
      EXTI->RTSR1 |= EXTI_RTSR1_RT21; //EXTI line 21 Rising edge Trigger enabled (depends on polarity)
    else
      EXTI->RTSR1 &= ~EXTI_RTSR1_RT21; //EXTI line 21 Rising edge Trigger disabled
      
    if (mode == FALLING || mode == CHANGE)
      EXTI->FTSR1 |= EXTI_FTSR1_FT21; //EXTI line 21 Falling edge Trigger enabled (depends on polarity)
    else
      EXTI->FTSR1 &= ~EXTI_FTSR1_FT21; //EXTI line 21 Falling edge Trigger disabled
  }
  else if (_comp == COMP2) {
    if (mode == RISING || mode == CHANGE)
      EXTI->RTSR1 |= EXTI_RTSR1_RT22; //EXTI line 22 Rising edge Trigger enabled (depends on polarity)
    else
      EXTI->RTSR1 &= ~EXTI_RTSR1_RT22; //EXTI line 22 Rising edge Trigger disabled
      
    if (mode == FALLING || mode == CHANGE)
      EXTI->FTSR1 |= EXTI_FTSR1_FT22; //EXTI line 22 Falling edge Trigger enabled (depends on polarity)
    else
      EXTI->FTSR1 &= ~EXTI_FTSR1_FT22; //EXTI line 22 Falling edge Trigger disabled
  }
}

void COMP_IRQHandler(void) {
  if ((EXTI->PR1 & EXTI_PR1_PIF21) == EXTI_PR1_PIF21) { //COMP1 triggered
     EXTI->PR1 = EXTI_PR1_PIF21;    //clear pending bit
     if((int)COMP_CALLBACK[0] != NULL)
       COMP_CALLBACK[0]();
  }
  if ((EXTI->PR1 & EXTI_PR1_PIF22) == EXTI_PR1_PIF22) { //COMP2 triggered
     EXTI->PR1 = EXTI_PR1_PIF22;    //clear pending bit
     if((int)COMP_CALLBACK[1] != NULL)
       COMP_CALLBACK[1]();
  }
}
