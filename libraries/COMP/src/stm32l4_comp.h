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

#ifndef _STM32L4_COMP_H
#define _STM32L4_COMP_H

#include "armv7m.h"

#include "stm32l4xx.h"
#include "stm32l4_wiring_private.h"

#define COMP_STATE_NONE                 0
#define COMP_STATE_INIT                 1
#define COMP_STATE_BUSY                 2
#define COMP_STATE_READY                3

#define COMP_INVERT                     1
#define COMP_WINMODE                    2
#define HIGH_POWER_MODE                 3
#define MED_POWER_MODE                  4
#define LOW_POWER_MODE                  5
#define HIGH_HYST_MODE                  6
#define MED_HYST_MODE                   7
#define LOW_HYST_MODE                   8
#define NONE_HYST_MODE                  9
#define COMP_EVENT_ENABLE               10
#define COMP_INTERRUPT_ENABLE           11

#ifdef __cplusplus
 extern "C" {
#endif


enum {
    COMP_INSTANCE_COMP1 = 0,
    COMP_INSTANCE_COMP2 = 1,
    COMP_INSTANCE_COUNT
};

typedef void (*stm32l4_comp_callback_t)(void *context);

typedef struct _stm32l4_comp_pins_t {
    int16_t                     posinpin;
    int16_t                     neginput;
} stm32l4_comp_pins_t;

typedef struct _stm32l4_comp_t {
    COMP_TypeDef                 *COMPx;
    volatile uint8_t             state;
    uint8_t                      instance;
    stm32l4_comp_pins_t          pins;
    IRQn_Type                    interrupt;
    uint8_t                      priority;
    uint32_t                     option;
    voidFuncPtr                  callback;
    void                         *context;
} stm32l4_comp_t;

typedef struct _stm32l4_comp_driver_t {
    stm32l4_comp_t     *instances[COMP_INSTANCE_COUNT];
} stm32l4_comp_driver_t;

static stm32l4_comp_driver_t stm32l4_comp_driver;

typedef enum {
    HIGH_MODE,
    MED_MODE,
    LOW_MODE,
    NONE_MODE
} comp_mode_t;

typedef enum {
    COMP_VINTREF = -6,
    COMP_VINTREF_3_4,
    COMP_VINTREF_1_2,
    COMP_VINTREF_1_4,
    COMP_DAC1,
    COMP_DAC2,
} comp_neginput_t;

typedef enum {
    COMP_TIMER_NONE,
    COMP_TIMER_1,
    COMP_TIMER_2,
    COMP_TIMER_3,
    COMP_TIMER_8,
    COMP_TIMER_15,
} comp_blanking_t;

extern bool stm32l4_comp_create(stm32l4_comp_t *comp, unsigned int instance, stm32l4_comp_pins_t *pins, unsigned int priority, unsigned int mode);
extern bool stm32l4_comp_destroy(stm32l4_comp_t *comp);
extern bool stm32l4_comp_enable(stm32l4_comp_t *comp, uint32_t option, stm32l4_comp_callback_t callback, void *context, uint32_t events);
extern bool stm32l4_comp_disable(stm32l4_comp_t *comp);
extern bool stm32l4_comp_configure(stm32l4_comp_t *comp, uint32_t option);
extern bool stm32l4_comp_notify(stm32l4_comp_t *comp, stm32l4_comp_callback_t callback, void *context, uint32_t events);

extern bool stm32l4_comp_pins_config(stm32l4_comp_t *comp);
extern bool stm32l4_comp_blanking(stm32l4_comp_t *comp, comp_blanking_t timer);
extern void stm32l4_comp_polarity(stm32l4_comp_t *comp, bool inverted);
extern void stm32l4_comp_enable_device(stm32l4_comp_t *comp);
extern void stm32l4_comp_disable_device(stm32l4_comp_t *comp);
extern bool stm32l4_comp_read(stm32l4_comp_t *comp);
extern bool stm32l4_comp_enabled(stm32l4_comp_t *comp);
extern void stm32l4_comp_lock(stm32l4_comp_t *comp);
extern bool stm32l4_comp_locked(stm32l4_comp_t *comp);
extern void stm32l4_comp_power_mode(stm32l4_comp_t *comp, comp_mode_t powermode);
extern void stm32l4_comp_hysteresis_mode(stm32l4_comp_t *comp, comp_mode_t hystmode);
extern void stm32l4_comp_winmode(stm32l4_comp_t *comp, bool enable);
extern void stm32l4_comp_enable_event(stm32l4_comp_t *comp, uint32_t mode);
extern void stm32l4_comp_disable_event(stm32l4_comp_t *comp);
extern bool stm32l4_comp_event_enabled(stm32l4_comp_t *comp);
extern void stm32l4_comp_enable_interrupt(stm32l4_comp_t *comp, uint32_t mode, voidFuncPtr callback);
extern void stm32l4_comp_disable_interrupt(stm32l4_comp_t *comp);
extern bool stm32l4_comp_interrupt_enabled(stm32l4_comp_t *comp);
extern void stm32l4_comp_interrupt(stm32l4_comp_t *comp);
extern void stm32l4_comp_trigger_edges(stm32l4_comp_t *comp, uint32_t mode);


extern void COMP_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif 