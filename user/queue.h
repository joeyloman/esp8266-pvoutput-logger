/*
 * esp8266-pvoutput-logger project - data queue functions
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

#include "date_time.h"

/* queue max 1 day (288 * 5 minutes) */
#define MAX_QUEUE_SIZE  288

typedef struct {
    char q_date[DATE_SIZE+1];
    char q_time[TIME_SIZE+1];
    unsigned long q_power_gen;
    unsigned long q_total_energy_gen;
} post_queue;

post_queue pq[MAX_QUEUE_SIZE];

extern int queue_count;

extern void queue_post_items_to_pvoutput(void);
extern void queue_post_items_to_thingspeak(void);
extern void queue_update_post_queue(void);
