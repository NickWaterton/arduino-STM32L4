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

//RCC->APB2ENR  /* SYSTEM_PERIPH_SYSCFG */

#define STM32L4_COMP_IRQ_PRIORITY     15

stm32l4_comp_callback_t COMP_CALLBACK[2];

COMPClass::COMPClass(COMP_TypeDef *comp_reg, int posinpin, int neginput, uint32_t mode) {

  int instance;
  
  if (comp_reg == COMP1) {
    this_comp = 1;
    instance = COMP_INSTANCE_COMP1;
  }
  else if (comp_reg == COMP2) {
    this_comp = 2;
    instance = COMP_INSTANCE_COMP2;
  }
  
  _comp->pins.posinpin = g_APinDescription[posinpin].pin;
  _comp->pins.neginput = neginput;
  
  _comp->COMPx = comp_reg;
  
  stm32l4_comp_create(_comp, instance, NULL, STM32L4_COMP_IRQ_PRIORITY, mode);
}

COMPClass::~COMPClass() {
  stm32l4_comp_destroy(_comp);
}

bool COMPClass::begin(int posinpin, int neginput, uint32_t option) {
  if (posinpin != GPIO_PIN_NONE)
    _comp->pins.posinpin = g_APinDescription[posinpin].pin;
  if (neginput != GPIO_PIN_NONE)
    _comp->pins.neginput = neginput;

  return stm32l4_comp_enable(_comp, option, NULL, (void*)this, NULL);
}

void COMPClass::end() {
  
  stm32l4_comp_disable(_comp);
  
  if(COMP1->CSR == (uint32_t)0x00000000 && COMP2->CSR == (uint32_t)0x00000000)  //if neither comparator is configured (0 is reset value)
    NVIC_DisableIRQ(_comp->interrupt);

  _comp->callback = NULL;
  _comp->state = COMP_STATE_NONE;
}

bool COMPClass::blanking(comp_blanking_t timer) {
  return stm32l4_comp_blanking(_comp, timer);
}

void COMPClass::polarity(bool inverted) {
  stm32l4_comp_polarity(_comp, inverted);
}

void COMPClass::enable() {
  stm32l4_comp_enable_device(_comp);
}

void COMPClass::disable() {
  stm32l4_comp_disable_device(_comp);
}

bool COMPClass::read() {
  return stm32l4_comp_read(_comp);
}

bool COMPClass::enabled() {
  return stm32l4_comp_enabled(_comp);
}

void COMPClass::lock() {
  stm32l4_comp_lock(_comp);
}

bool COMPClass::locked() {
  return stm32l4_comp_locked(_comp);
}

void COMPClass::power_mode(comp_mode_t powermode) {
  stm32l4_comp_power_mode(_comp, powermode);
}

void COMPClass::hysteresis_mode(comp_mode_t hystmode) {
  stm32l4_comp_hysteresis_mode(_comp, hystmode);
}

void COMPClass::winmode(bool enable) {
  stm32l4_comp_winmode(_comp, enable);
}

void COMPClass::enable_event(uint32_t mode) {
  stm32l4_comp_enable_event(_comp, mode);
}

void COMPClass::disable_event() {
  stm32l4_comp_disable_event(_comp);
}

bool COMPClass::event_enabled() {
  return stm32l4_comp_event_enabled(_comp);
}

void COMPClass::enable_interrupt(uint32_t mode, voidFuncPtr callback) {
  stm32l4_comp_enable_interrupt(_comp, mode, callback);
}

void COMPClass::disable_interrupt() {
  stm32l4_comp_disable_interrupt(_comp);
}

bool COMPClass::interrupt_enabled() {
  return stm32l4_comp_interrupt_enabled(_comp);
}

/*
void COMPClass::_eventCallback(void *context) {
  stm32l4_comp_t* comp = reinterpret_cast<stm32l4_comp_t*>(context);
  if ((int)comp->_callback != NULL)
    comp->_callback();
}
*/
void COMPClass::trigger_edges(uint32_t mode) {
  stm32l4_comp_trigger_edges(_comp, mode);
}
