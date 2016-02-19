/*
 * esp8266-pvoutput-logger project - date/time functions
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

#ifndef DATE_TIME_H
#define DATE_TIME_H

#define DATE_SIZE   8
#define TIME_SIZE   5

extern char date[DATE_SIZE+1];
extern char time[TIME_SIZE+1];

typedef struct
{
    unsigned int second; // 0-59
    unsigned int minute; // 0-59
    unsigned int hour;   // 0-23
    unsigned int day;    // 1-31
    unsigned int month;  // 1-12
    unsigned int year;   // 0-99 (representing 2000-2099)
} date_time_t;

extern void date_time_format_line();
extern void date_time_init(void);

#endif
