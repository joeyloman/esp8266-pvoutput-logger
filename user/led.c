/*
 * esp8266-pvoutput-logger project - LED control functions
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

/*
 * LED status definitions:
 *
 * GREEN_LED ON -> Power on
 * BLUE_LED ON -> Wifi connected
 * RED_LED ON  -> Webserver connection error
 * YELLOW_LED BLINK  -> Interrupt pulse
 */

#include "user_interface.h"
#include "osapi.h"
#include "gpio.h"

#include "led.h"
#include "config.h"

/* initialize the LED status to OFF */
static int green_led_status = LED_OFF;
static int blue_led_status = LED_OFF;
static int red_led_status = LED_OFF;
static int yellow_led_status = LED_OFF;

void ICACHE_FLASH_ATTR
LED_toggle(int lid, int new_status)
{
#ifdef DEBUG
    os_printf("[debug] LED_toggle\r\n");
#endif

#ifdef DEBUG
    os_printf("[debug] LED_toggle: lid: [%d] | new_status: [%d] | green_led_status: [%d] | blue_led_status: [%d] | red_led_status: [%d] | yellow_led_status: [%d]\r\n",
        lid, new_status, green_led_status, blue_led_status, red_led_status, yellow_led_status);
#endif

    switch(lid) {
        case GREEN_LED:
            if (green_led_status == new_status)
                break;

            /* the green LED is wired on GPIO5 (D1) */
            if (new_status == LED_ON) {
#ifdef DEBUG
                os_printf("[debug] turning GREEN_LED ON\r\n");
#endif
                gpio_output_set(BIT(5), 0, BIT(5), 0);
            } else {
#ifdef DEBUG
                os_printf("[debug] turning GREEN_LED OFF\r\n"); 
#endif
                gpio_output_set(0, BIT(5), BIT(5), 0);
            }

            green_led_status = new_status;

            break;
            ;;
        case BLUE_LED:
            if (blue_led_status == new_status)
                break;

            /* the blue LED is wired on GPIO4 (D2) */
            if (new_status == LED_ON) {
#ifdef DEBUG
                os_printf("[debug] turning BLUE_LED ON\r\n"); 
#endif

                gpio_output_set(BIT(4), 0, BIT(4), 0);
            } else {
#ifdef DEBUG
                os_printf("[debug] turning BLUE_LED OFF\r\n"); 
#endif
                gpio_output_set(0, BIT(4), BIT(4), 0);
            }

            blue_led_status = new_status;

            break;
            ;;
        case RED_LED:
            if (red_led_status == new_status)
                break;

            /* the red LED is wired on GPIO2 (D4) */
            if (new_status == LED_ON) {
#ifdef DEBUG
                os_printf("[debug] turning RED_LED ON\r\n"); 
#endif
                gpio_output_set(BIT(2), 0, BIT(2), 0);
            } else {
#ifdef DEBUG
                os_printf("[debug] turning RED_LED OFF\r\n");
#endif
                gpio_output_set(0, BIT(2), BIT(2), 0);
            }

            red_led_status = new_status;

            break;
            ;;
        case YELLOW_LED:
            if (yellow_led_status == new_status)
                break;

            /* the yellow LED is wired on GPIO12 (D6/HSPIQ) */
            if (new_status == LED_ON) {
#ifdef DEBUG
                os_printf("[debug] turning YELLOW_LED ON\r\n"); 
#endif
                gpio_output_set(BIT(12), 0, BIT(12), 0);
            } else {
#ifdef DEBUG
                os_printf("[debug] turning YELLOW_LED OFF\r\n");
#endif
                gpio_output_set(0, BIT(12), BIT(12), 0);
            }
            
            yellow_led_status = new_status;

            break;
        default:
            os_printf("[error] unknown LED id!\r\n"); 

            break;
            ;;
    }
}

void
LED_init(void)
{
#ifdef DEBUG
    os_printf("[debug] LED_init\r\n");
#endif

    /* initialize GPIO2 as a OUTPUT function (D4) and turn it off */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    gpio_output_set(0, BIT(2), BIT(2), 0);

    /* initialize GPIO12 as a OUTPUT function (D6) */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);

    /* turn on the green led */
    LED_toggle(GREEN_LED, LED_ON);
}
