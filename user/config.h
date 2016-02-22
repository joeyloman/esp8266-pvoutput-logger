/*
 * esp8266-pvoutput-logger project - config options
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

#include "defs.h"

/*
 * debug configuration
 */

/* enable this to show all debug information */
//#define DEBUG


/*
 * wifi configuration
 */

#define WIFI_SSID "<Put your wifi SSID here>"
#define WIFI_PASS "<Put your wifi password here>"

/*
 * Power configuration
 */

/* number of blinks per kWh of your meter (max value: 1000) */
#define PULSE_FACTOR 1000

/* max watt power of your system */
#define MAX_WATT_POWER 3924


/*
 * Time configuration
 *

/* define your timezone
 * accepted values:
 * -9,-8,-7,-6,-5,-4,-3,-2,-1,
 * 0,+1,+2,+3,+4,+5,+6,+7,+8,+9
 */
#define TIMEZONE +1

/* uncomment this if you want to enable the dutch daylight savings time */
#define ENABLE_DUCTH_DST_TIME


/*
 * queue configuration
 */

/* time between queue posting, options: 5 or 15 (minutes) */
#define queue_post_interval 5


/*
 * output client configuration
 */

/* choose your output client, options: PVOUTPUT or THINGSPEAK */
#define OUTPUT_CLIENT   PVOUTPUT


/*
 * PVoutput configuration
 */

#define PVOUTPUT_APIKEY             "<Put your pvoutput apikey here>"
#define PVOUTPUT_SYSTEMID           "<Put your pvoutput systemid here>"


/*
 * ThingSpeak configuration
 */

#define THINGSPEAK_APIKEY           "<Put your thingspeak apikey here>"
#define THINGSPEAK_POWER_FIELD      "field1"
#define THINGSPEAK_ENERGY_FIELD     "field2"
