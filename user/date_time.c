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
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "date_time.h"

#include "led.h"
#include "config.h"

/* start at the year 2000 */
#define THIRTY_YEAR_EPOCH           946684800

#ifdef ENABLE_DUCTH_DST_TIME
#define ONE_HOUR                    3600
#endif

char date[DATE_SIZE+1];
char time[TIME_SIZE+1];

const days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

#ifdef ENABLE_DUCTH_DST_TIME
/*
 * Get Dutch Daylight Saving Time (DST)
 */
uint32
date_time_get_dutch_dst(uint32 epoch)
{
#ifdef DEBUG
    os_printf("[debug] date_time_get_dst: epoch [%u]\r\n", epoch);
#endif

    /* convert epoch (GMT+1) to GMT */
    uint32 epoch_gmt = epoch - ONE_HOUR;

    /*
     * The code below can be reduced by removing the wintertime checks.
     * But for debugging it's handy.
     */
    if ((epoch_gmt > 1445742000) && (epoch_gmt < 1459047600)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 25-10-2015 and 27-03-2016 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1459047600) && (epoch_gmt < 1477796400)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 27-03-2016 and 30-10-2016 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1477796400) && (epoch_gmt < 1490497200)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 30-10-2016 and 26-03-2017 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1490497200) && (epoch_gmt < 1509246000)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 26-03-2017 and 29-10-2017 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1509246000) && (epoch_gmt < 1521946800)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 29-10-2017 and 25-03-2018 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1521946800) && (epoch_gmt < 1540695600)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 25-03-2018 and 28-10-2018 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1540695600) && (epoch_gmt < 1554001200)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 28-10-2018 and 31-03-2019 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1554001200) && (epoch_gmt < 1572145200)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 31-03-2019 and 27-10-2019 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1572145200) && (epoch_gmt < 1585450800)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 27-10-2019 and 29-03-2020 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1585450800) && (epoch_gmt < 1603594800)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 29-03-2020 and 25-10-2020 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1603594800) && (epoch_gmt < 1616900400)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 25-10-2020 and 28-03-2021 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1616900400) && (epoch_gmt < 1635649200)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 28-03-2021 and 31-10-2021 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    } else if ((epoch_gmt > 1635649200) && (epoch_gmt < 1648350000)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 31-10-2021 and 27-03-2022 (wintertime)\r\n", epoch_gmt);
#endif

        /* return wintertime */
        return(epoch);
    } else if ((epoch_gmt > 1648350000) && (epoch_gmt < 1667098800)) {
#ifdef DEBUG
        os_printf("[debug] date_time_get_dst: epoch_gmt [%u] between 27-03-2022 and 30-10-2022 (summertime)\r\n", epoch_gmt);
#endif

        /* return summertime */
        return(epoch + ONE_HOUR);
    }

    /* if no match found, return the wintertime */
    os_printf("[info] date_time_get_dst: no dst match found, returning wintertime!\r\n");
    return(epoch);
}
#endif

void
epoch_to_date_time(date_time_t* date_time, uint32 epoch)
{
#ifdef ENABLE_DUCTH_DST_TIME
    /* align the epoch time to the year 2000 dst time */
    epoch = date_time_get_dutch_dst(epoch) - THIRTY_YEAR_EPOCH;
#else
    /* align the epoch time to the year 2000 */
    epoch = epoch - THIRTY_YEAR_EPOCH;
#endif

#ifdef DEBUG
    os_printf("[debug] epoch_to_date_time: epoch [%u]\r\n", epoch);
#endif

    date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    const unsigned int years = epoch / (365 * 4 + 1) * 4;
    epoch %= 365 * 4 + 1;

    unsigned int year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    unsigned int month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = (years + year);
    date_time->month = month + 1;
    date_time->day   = (epoch - days[year][month]) + 1;
}

void
date_time_format_line()
{
#ifdef DEBUG
    os_printf("[debug] date_time_format_line\r\n");
#endif

    /*
     * PVOutput API date format: 20151130
     * PVOutput API time format: 13:00
     */

    date_time_t date_time;
    epoch_to_date_time(&date_time, sntp_get_current_timestamp());

#ifdef DEBUG
    os_printf("[debug] date_time_format_line: second: [%u] | minute: [%u] | hour: [%u] | day: [%u] | month: [%u] | year: [%u]\r\n", 
        date_time.second, date_time.minute, date_time.hour, date_time.day, date_time.month, date_time.year);
#endif

    /* store the date */
    os_sprintf(date, "20%u%02u%02u", date_time.year, date_time.month, date_time.day); 

    /* null terminate the date buffer */
    date[sizeof(date) - 1] = '\0';

    /* store the time */
    os_sprintf(time, "%02u:%02u", date_time.hour, date_time.minute); 

    /* null terminate the time buffer */
    time[sizeof(time) - 1] = '\0';

#ifdef DEBUG
    os_printf("[debug] date_time_format_line: date: [%s] | time: [%s]\r\n", date, time);
#endif
}

void
date_time_init(void)
{
#ifdef DEBUG
    os_printf("[debug] fetch_current_date_time\r\n");
#endif

    /* clear the date and time buffers */
    os_memset(date, 0, sizeof(date));
    os_memset(time, 0, sizeof(time));

    /* configure sntp */
    sntp_setservername(0, "0.pool.ntp.org");
    sntp_setservername(1, "1.pool.ntp.org");
    sntp_set_timezone(TIMEZONE);
    sntp_init();
}
