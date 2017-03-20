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

//butterfly (433):
// NOTE: COMP1 +input can only be attached to PC5, PB2, PA1
//       COMP2 +input can only be attached to PB4, PB6, PA3
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PC4, PA0, PA4, PA5
//       COMP2 PB7, PA2, PA4, PA5
//ladybug (432)
// NOTE: COMP1 +input can only be attached to PC5, PB2, PA1
//       COMP2 +input can only be attached to PB4, PB6, PA3
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PB1, PC4, PA0, PA4, PA5
//       COMP2 PB3, PB7, PA2, PA4, PA5
//dragonfly (476)
// NOTE: COMP1 +input can only be attached to PC5, PB2
//       COMP2 +input can only be attached to PB4, PB6
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PB1, PC4
//       COMP2 PB3, PB7
// Not all pins are broken out/available on all boards - check your board pin out!
// you must enter the pin number as the names are not defined, except for some specialist pins (A3, PIN_LED etc.)

#ifndef _COMP_H_INCLUDED
#define _COMP_H_INCLUDED

#include <Arduino.h>
#include "stm32l4_comp.h"

#ifdef __cplusplus
extern "C" {
#endif

class COMPClass
{
public:
    COMPClass(COMP_TypeDef *comp_reg, int posinpin = GPIO_PIN_NONE, int neginput = GPIO_PIN_NONE, uint32_t mode = 0);
    ~COMPClass();

    bool begin(int posinpin = GPIO_PIN_NONE, int neginput = GPIO_PIN_NONE, uint32_t option = 0);
    void end();
    
    void polarity(bool inverted);
    void enable();
    void disable();
    bool read();
    bool enabled();
    void lock();
    bool locked();
    bool blanking(comp_blanking_t timer);
    void power_mode(comp_mode_t powermode);
    void hysteresis_mode(comp_mode_t hystmode);
    void winmode(bool enable);
    void enable_event(uint32_t mode = RISING);
    void disable_event();
    bool event_enabled();
    void enable_interrupt(uint32_t mode = RISING, voidFuncPtr callback = NULL);
    void disable_interrupt();
    bool interrupt_enabled();
    void trigger_edges(uint32_t mode = RISING);
    
    uint8_t this_comp = 0;   
    
private:   
    //struct _stm32l4_comp_t *_comp = new stm32l4_comp_t;
    stm32l4_comp_t comp_struct;
    struct _stm32l4_comp_t *_comp = &comp_struct; 
};

#ifdef __cplusplus
}
#endif

#endif
