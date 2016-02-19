/*
 * esp8266-pvoutput-logger project - main functions
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

#include "driver/uart.h"

#include "wifi.h"
#include "led.h"
#include "interrupt.h"
#include "pvoutput_client.h"

/*
 * ESP8266 start function
 */
void user_init(void)
{
    /* set the uart bit rate (otherwise the screen gives garbage) */
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

    /* send a boot message to the uart */
    uart0_sendStr("\r\nESP8266 booting...\r\n"); // note: same as os_printf

    /* initialize the led functions */
    LED_init();

    /* initialize the wifi functions */
    wifi_init();

    /* initialize the interrupt functions */
    interrupt_init();

    /* initialize the time service */
    date_time_init();

    /* initialize the scheduler */
    scheduler_init();
}
