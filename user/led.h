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

/* LED status */
#define LED_ON  1
#define LED_OFF 2

/* LED names */
#define GREEN_LED   1
#define BLUE_LED    2
#define RED_LED     3
#define YELLOW_LED  4

extern void ICACHE_FLASH_ATTR LED_toggle(int led_id, int new_status);
extern void LED_init(void); 
