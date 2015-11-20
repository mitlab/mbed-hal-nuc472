/* mbed Microcontroller Library
 * Copyright (c) 2015-2016 Nuvoton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed-hal/gpio_irq_api.h"

#if DEVICE_INTERRUPTIN

#include "gpio_api.h"
#include "cmsis.h"
#include "mbed-hal/pinmap.h"
#include "PeripheralPins.h"
//#include "uvisor-lib/uvisor-lib.h"
#include "nu_bitutil.h"

#define NU_MAX_PIN_PER_PORT     16

struct nu_gpio_irq_var {
    gpio_irq_t *    obj_arr[NU_MAX_PIN_PER_PORT];
    IRQn_Type       irq_n;
    void            (*vec)(void);
};

static void gpio_irq_0_vec(void);
static void gpio_irq_1_vec(void);
static void gpio_irq_2_vec(void);
static void gpio_irq_3_vec(void);
static void gpio_irq_4_vec(void);
static void gpio_irq_5_vec(void);
static void gpio_irq_6_vec(void);
static void gpio_irq_7_vec(void);
static void gpio_irq_8_vec(void);
static void gpio_irq(struct nu_gpio_irq_var *var);

//EINT0_IRQn
static struct nu_gpio_irq_var gpio_irq_var_arr[] = {
    {{NULL}, GPA_IRQn, gpio_irq_0_vec},
    {{NULL}, GPB_IRQn, gpio_irq_1_vec},
    {{NULL}, GPC_IRQn, gpio_irq_2_vec},
    {{NULL}, GPD_IRQn, gpio_irq_3_vec},
    {{NULL}, GPE_IRQn, gpio_irq_4_vec},
    {{NULL}, GPF_IRQn, gpio_irq_5_vec},
    {{NULL}, GPG_IRQn, gpio_irq_6_vec},
    {{NULL}, GPH_IRQn, gpio_irq_7_vec},
    {{NULL}, GPI_IRQn, gpio_irq_8_vec}
};

#define NU_MAX_PORT     (sizeof (gpio_irq_var_arr) / sizeof (gpio_irq_var_arr[0]))

int gpio_irq_init(gpio_irq_t *obj, PinName pin, gpio_irq_handler handler, uint32_t id)
{
    if (pin == NC) {
        return -1;
    }
    
    uint32_t pin_index = PINNAME_TO_PIN(pin);
    uint32_t port_index = PINNAME_TO_PORT(pin);
    if (pin_index >= NU_MAX_PIN_PER_PORT || port_index >= NU_MAX_PORT) {
        return -1;
    }
    
    obj->pin = pin;
    obj->irq_handler = (uint32_t) handler;
    obj->irq_id = id;

    //gpio_set(pin);
    
    // Configure de-bounce clock source and sampling cycle time
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_1);
    
    struct nu_gpio_irq_var *var = gpio_irq_var_arr + port_index;
    
    MBED_ASSERT(pin_index < NU_MAX_PIN_PER_PORT);
    var->obj_arr[pin_index] = obj;
    
    return 0;
}

void gpio_irq_free(gpio_irq_t *obj)
{
    uint32_t pin_index = PINNAME_TO_PIN(obj->pin);
    uint32_t port_index = PINNAME_TO_PORT(obj->pin);
    struct nu_gpio_irq_var *var = gpio_irq_var_arr + port_index;
    
    NVIC_DisableIRQ(var->irq_n);
    PORT_BASE(port_index)->INTEN = 0;
    
    MBED_ASSERT(pin_index < NU_MAX_PIN_PER_PORT);
    var->obj_arr[pin_index] = NULL;
}

void gpio_irq_set(gpio_irq_t *obj, gpio_irq_event event, uint32_t enable)
{
    uint32_t pin_index = PINNAME_TO_PIN(obj->pin);
    uint32_t port_index = PINNAME_TO_PORT(obj->pin);
    GPIO_T *gpio_base = PORT_BASE(port_index);
    
    switch (event) {
        case IRQ_RISE:
            if (enable) {
                GPIO_ENABLE_DEBOUNCE(gpio_base, 1 << pin_index);
                GPIO_EnableInt(gpio_base, pin_index, GPIO_INT_RISING);
            }
            else {
                gpio_base->INTEN &= ~(GPIO_INT_RISING << pin_index);
            }
            break;
        
        case IRQ_FALL:
            if (enable) {
                GPIO_ENABLE_DEBOUNCE(gpio_base, 1 << pin_index);
                GPIO_EnableInt(gpio_base, pin_index, GPIO_INT_FALLING);
            }
            else {
                gpio_base->INTEN &= ~(GPIO_INT_FALLING << pin_index);
            }
            break;
    }
}

void gpio_irq_enable(gpio_irq_t *obj)
{
    //uint32_t pin_index = PINNAME_TO_PIN(obj->pin);
    uint32_t port_index = PINNAME_TO_PORT(obj->pin);
    struct nu_gpio_irq_var *var = gpio_irq_var_arr + port_index;
    
    NVIC_SetVector(var->irq_n, (uint32_t) var->vec);
    NVIC_EnableIRQ(var->irq_n);
}

void gpio_irq_disable(gpio_irq_t *obj)
{
    //uint32_t pin_index = PINNAME_TO_PIN(obj->pin);
    uint32_t port_index = PINNAME_TO_PORT(obj->pin);
    struct nu_gpio_irq_var *var = gpio_irq_var_arr + port_index;
    
    NVIC_DisableIRQ(var->irq_n);
}

static void gpio_irq_0_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 0);
}
static void gpio_irq_1_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 1);
}
static void gpio_irq_2_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 2);
}
static void gpio_irq_3_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 3);
}
static void gpio_irq_4_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 4);
}
static void gpio_irq_5_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 5);
}
static void gpio_irq_6_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 6);
}
static void gpio_irq_7_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 7);
}
static void gpio_irq_8_vec(void)
{
    gpio_irq(gpio_irq_var_arr + 8);
}

static void gpio_irq(struct nu_gpio_irq_var *var)
{
    uint32_t port_index = var->irq_n - GPA_IRQn;
    GPIO_T *gpio_base = PORT_BASE(port_index);
    
    uint32_t intsrc = gpio_base->INTSRC;
    uint32_t inten = gpio_base->INTEN;
    while (intsrc) {
        int pin_index = nu_ctz(intsrc);
        gpio_irq_t *obj = var->obj_arr[pin_index];
        if (inten & (GPIO_INT_RISING << pin_index)) {
            if (GPIO_PIN_ADDR(port_index, pin_index)) {
                if (obj->irq_handler) {
                    ((gpio_irq_handler) obj->irq_handler)(obj->irq_id, IRQ_RISE);
                }
            }
        }
        
        if (inten & (GPIO_INT_FALLING << pin_index)) {   
            if (! GPIO_PIN_ADDR(port_index, pin_index)) {
                if (obj->irq_handler) {
                    ((gpio_irq_handler) obj->irq_handler)(obj->irq_id, IRQ_FALL);
                }
            }
        }
        
        intsrc &= ~(1 << pin_index);
    }
    // Clear all interrupt flags
    gpio_base->INTSRC = gpio_base->INTSRC;
}

#endif
