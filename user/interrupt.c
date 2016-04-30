/*
 * esp8266-pvoutput-logger project - interrupt functions
 *
 * Copyright (C) 2015 Joey Loman <joey@binbash.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "user_interface.h"
#include "osapi.h"
#include "gpio.h"

#include "defs.h"
#include "interrupt.h"
#include "led.h"
#include "config.h"
#include "date_time.h"

LOCAL os_timer_t power_interrupt_lock_timer;
LOCAL os_timer_t update_time_counter_timer;
LOCAL os_timer_t blink_timer;

uint8_t power_interrupt_lock = 0;
uint8_t first_run = YES;
uint32_t pulse_count = 0;
uint32_t interval_pulse_count = 0;

unsigned long time_counter = 0;
unsigned long cur_power = 0;
unsigned long total_watt = 0;

void ICACHE_FLASH_ATTR
unlock_power_interrupt(void)
{
#ifdef DEBUG
    os_printf("[debug] unlock_power_interrupt: enabling interrupt.\r\n");
#endif

    power_interrupt_lock = 0;

    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

#ifdef DEBUG
    if (gpio_status & BIT(13)) {
            os_printf("[debug] unlock_power_interrupt: interrupt bit is set.\r\n");
    } else {
            os_printf("[debug] unlock_power_interrupt: interrupt bit is NOT set.\r\n");
    }
#endif

    /* reactivate interrupts for GPIO13 */
    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_POSEDGE);
}

void ICACHE_FLASH_ATTR
turn_off_interrupt_led(void)
{
    LED_toggle(YELLOW_LED, LED_OFF);
}

/* GPIO interrupt handler */
static void ICACHE_FLASH_ATTR
power_interrupt(int8_t key)
{
#ifdef DEBUG
    os_printf("[debug] power_interrupt\r\n");
#endif

    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    if (gpio_status & BIT(13)) {
#ifdef DEBUG
    os_printf("[debug] power_interrupt: interrupt detected, disabling interrupt.\r\n");
#endif

        /* disable interrupt for GPIO13 */
        gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_DISABLE);

        /* clear interrupt status for GPIO13 */
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(13));
    } else {
        os_printf("[%s] [error] power_interrupt: interrupt detected, but interrupt bit is not set!\r\n", date_time_get_ts());
    }

    power_interrupt_lock++;

    /* if this function is executed multiple times between 100 ms, return */
    if (power_interrupt_lock > 1) {
        os_printf("[%s] [error] power_interrupt: power_interrupt_lock is set [%d], skipping interrupt!\r\n",
            date_time_get_ts(), power_interrupt_lock);
        return;
    }

    /* blink the yellow led 0.1 sec */
    LED_toggle(YELLOW_LED, LED_ON);
    os_timer_disarm(&blink_timer);
    os_timer_setfn(&blink_timer, (os_timer_func_t *)turn_off_interrupt_led, NULL);
    os_timer_arm(&blink_timer, 100, 0);

    if (first_run == NO) {
        if (time_counter > 0) {
            /* calculate the realtime power/watts */
            cur_power = (360000000 / (PULSE_FACTOR * time_counter));

            /* if the current power is more then the maximum of your system (which you defined in pvoutput),
             * skip this pulse. the pvoutput server will decline this value and your queue will not be emptied anymore!
             * note: this can also happen if the pulse took longer then the power_interrupt_lock_timer.
             */
            if (cur_power > ((MAX_WATT_POWER / 100) * 125)) {
                os_printf("[%s] [error] power_interrupt: current power [%lu W] is more then the defined one in MAX_WATT_POWER * 125% [%d W], skipping pulse!\r\n",
                    date_time_get_ts(), cur_power, ((MAX_WATT_POWER / 100) * 125));

                /* unlock the power interrupt (pulse has a delay of 100ms) */
                os_timer_disarm(&power_interrupt_lock_timer);
                os_timer_setfn(&power_interrupt_lock_timer, (os_timer_func_t *)unlock_power_interrupt, NULL);
                os_timer_arm(&power_interrupt_lock_timer, 100, 0);

                return;
            }

            /* add the realtime power to the total amount of watts (which will be devided by the
             * interval_pulse_count before adding it to the queue
             */
            total_watt = total_watt + cur_power;
        }

        pulse_count++;
        interval_pulse_count++;
    } else {
        first_run = NO;
    }

#ifdef DEBUG
    os_printf("[%s] [debug] power_interrupt: current power = %lu W / total power between interval = %lu W / pulse count today = %u / pulse count between interval = %u\r\n",
        date_time_get_ts(), cur_power, total_watt, pulse_count, interval_pulse_count);
#endif

    /* reset the time_counter to start a new calculation */
    time_counter = 0;

    /* unlock the power interrupt (pulse has a delay of 100ms) */
    os_timer_disarm(&power_interrupt_lock_timer);
    os_timer_setfn(&power_interrupt_lock_timer, (os_timer_func_t *)unlock_power_interrupt, NULL);
    os_timer_arm(&power_interrupt_lock_timer, 100, 0);
}

void ICACHE_FLASH_ATTR
update_time_counter(void)
{
    time_counter++;
}

void
start_update_time_counter_timer(void)
{
#ifdef DEBUG
    os_printf("[debug] start_update_time_counter_timer\r\n");
#endif

    /* update the time counter every 10 ms to record the time between the pulses */
    os_timer_disarm(&update_time_counter_timer);
    os_timer_setfn(&update_time_counter_timer, (os_timer_func_t *)update_time_counter, NULL);
    os_timer_arm(&update_time_counter_timer, 10, 1);
}

void
interrupt_reset_power_state(void)
{
#ifdef DEBUG
    os_printf("[debug] interrupt_reset_power_state\r\n");
#endif

    total_watt = 0;
    interval_pulse_count = 0;
}

void
interrupt_reset_total_energy_state(void)
{
#ifdef DEBUG
    os_printf("[debug] interrupt_reset_total_energy_state\r\n");
#endif

    pulse_count = 0;
    total_watt = 0;
    interval_pulse_count = 0;
}

void 
interrupt_init(void)
{
#ifdef DEBUG
    os_printf("[debug] interrupt_init\r\n");
#endif

    /* disable interrupt and set things up */
    ETS_GPIO_INTR_DISABLE();

    /* register the GPIO13 (D7) interrupt handler */
    ETS_GPIO_INTR_ATTACH(power_interrupt, 13);

    /* pin 13 setup */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); // select pin to GPIO 13 mode
    GPIO_DIS_OUTPUT(13); // set pin 13 to "input" mode
    PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U); // enable pin pull up

    /* set the pin state of gpio 13 in interrupt mode positive edge */
    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_POSEDGE);

    /* reset interrupt status */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(13));

    /* enable the GPIO interrupts */
    ETS_GPIO_INTR_ENABLE();

    /* start the time_counter */
    start_update_time_counter_timer();
}
