/*
 * esp8266-pvoutput-logger project - scheduler functions
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

#include "defs.h"
#include "scheduler.h"
#include "date_time.h"
#include "queue.h"
#include "interrupt.h"
#include "led.h"
#include "config.h"

LOCAL os_timer_t scheduler_timer;
LOCAL os_timer_t scheduler_reset_timer;

/* initialize the timer at 1 sec (1000ms) */
int scheduler_timer_ms = 1000;

/* first time epoch check count */
int epoch_check_timer = 0;

/* function prototypes */
static void ICACHE_FLASH_ATTR scheduler_check_tasks(void);

static void ICACHE_FLASH_ATTR
scheduler_format_current_date(void)
{
    date_time_t date_time;
}

static void ICACHE_FLASH_ATTR
scheduler_reset_scheduler_timer(void)
{
#ifdef DEBUG
    os_printf("[debug] scheduler_reset_scheduler_timer\r\n");
#endif

    /* set the scheduler_timer_ms on 1 sec and execute the scheduler every second again to determine the current time */
    scheduler_timer_ms = 1000;
    os_timer_disarm(&scheduler_timer);
    os_timer_setfn(&scheduler_timer, (os_timer_func_t *)scheduler_check_tasks, NULL);
    os_timer_arm(&scheduler_timer, scheduler_timer_ms, 1);
}

static void ICACHE_FLASH_ATTR
scheduler_check_tasks(void)
{
#ifdef DEBUG
    os_printf("[debug] scheduler_check_tasks\r\n");
#endif

    /* if the epoch time is not set, try again the next time when this function is called */
    if (sntp_get_current_timestamp() == 0) {
        os_printf("[error] scheduler_check_tasks: epoch time not set, cannot execute scheduled tasks! retrying in %u secs..\r\n", scheduler_timer_ms / 1000);

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);

        return;
    }

    /* turn off the red led */
    LED_toggle(RED_LED, LED_OFF);

    /* get the current time */
    date_time_t date_time;
    epoch_to_date_time(&date_time, sntp_get_current_timestamp());

#ifdef DEBUG
    os_printf("[debug] scheduler_get_current_date: second: [%u] | minute: [%u] | hour: [%u] | day: [%u] | month: [%u] | year: [%u]\r\n",
        date_time.second, date_time.minute, date_time.hour, date_time.day, date_time.month, date_time.year);
#endif

    /*
     * the following code outlines the scheduler every 5 minutes on 0,5,10,15,20,25,30,35,40,45,50,55
     */
    if (scheduler_timer_ms == 1000) {
        /* if the seconds are on 0, update the timer to 1 minute */
        if (date_time.second == 0) {
            /* if the the minutes are on 0,5,10,15,20,25,30,35,40,45,50,55 update the timer to 5 minutes */
            if ((date_time.minute == 0) || (date_time.minute == 5) || (date_time.minute == 10) || (date_time.minute == 15) || 
                (date_time.minute == 20) || (date_time.minute == 25) || (date_time.minute == 30) || (date_time.minute == 35) ||
                (date_time.minute == 40) || (date_time.minute == 45) || (date_time.minute == 50) || (date_time.minute == 55)) {

#ifdef DEBUG
                os_printf("[debug] scheduler_get_current_date: updating scheduler to every 5 minutes.\r\n");
#endif

                /* disarm the timer */
                os_timer_disarm(&scheduler_timer);

                /* 5 minutes (300000ms) */
                scheduler_timer_ms = 300000;

                /* re-arm the timer and execute the scheduler every 5 minutes */
                os_timer_setfn(&scheduler_timer, (os_timer_func_t *)scheduler_check_tasks, NULL);
                os_timer_arm(&scheduler_timer, scheduler_timer_ms, 1);
            } else {
#ifdef DEBUG
                os_printf("[debug] scheduler_get_current_date: updating scheduler to every minute.\r\n");
#endif

                /* disarm the timer */
                os_timer_disarm(&scheduler_timer);

                /* 1 minute (60000ms) */
                scheduler_timer_ms = 60000;

                /* re-arm the timer and execute the scheduler every minute */
                os_timer_setfn(&scheduler_timer, (os_timer_func_t *)scheduler_check_tasks, NULL);
                os_timer_arm(&scheduler_timer, scheduler_timer_ms, 1);
            }
        } else {
            /* do nothing */
            return;
        }
    } else if (scheduler_timer_ms == 60000) {
        /* check if the timer is outlined correctly, if not reschedule it again */
        if (date_time.second != 0) {
            /* disarm the timer */
            os_timer_disarm(&scheduler_timer);

             /* reset it after 5 secs */
            os_timer_disarm(&scheduler_reset_timer);
            os_timer_setfn(&scheduler_reset_timer, (os_timer_func_t *)scheduler_reset_scheduler_timer, NULL);
            os_timer_arm(&scheduler_reset_timer, 5000, 0);
        }

        /* if the the minutes are on 0,5,10,15,20,25,30,35,40,45,50,55 update the timer to 5 minutes */
        if ((date_time.minute == 0) || (date_time.minute == 5) || (date_time.minute == 10) || (date_time.minute == 15) || 
            (date_time.minute == 20) || (date_time.minute == 25) || (date_time.minute == 30) || (date_time.minute == 35) ||
            (date_time.minute == 40) || (date_time.minute == 45) || (date_time.minute == 50) || (date_time.minute == 55)) {

#ifdef DEBUG
            os_printf("[debug] scheduler_get_current_date: updating scheduler to every 5 minutes.\r\n");
#endif

            /* disarm the timer */
            os_timer_disarm(&scheduler_timer);

            /* 5 minutes (300000ms) */
            scheduler_timer_ms = 300000;

            /* re-arm the timer and execute the scheduler every 5 minutes */
            os_timer_setfn(&scheduler_timer, (os_timer_func_t *)scheduler_check_tasks, NULL);
            os_timer_arm(&scheduler_timer, scheduler_timer_ms, 1);
        }
    } else if (scheduler_timer_ms == 300000) {
        /* check if the timer is outlined correctly, if not reschedule it again */
        if (date_time.second != 0) {
            /* disarm the timer */
            os_timer_disarm(&scheduler_timer);

             /* reset it after 5 secs */
            os_timer_disarm(&scheduler_reset_timer);
            os_timer_setfn(&scheduler_reset_timer, (os_timer_func_t *)scheduler_reset_scheduler_timer, NULL);
            os_timer_arm(&scheduler_reset_timer, 5000, 0);
        }
    }

    /* if the queue post interval is 5, post the queue every 5 minutes */
    if (queue_post_interval == 5) {
        if ((date_time.minute == 0) || (date_time.minute == 5) || (date_time.minute == 10) || (date_time.minute == 15) ||
            (date_time.minute == 20) || (date_time.minute == 25) || (date_time.minute == 30) || (date_time.minute == 35) ||
            (date_time.minute == 40) || (date_time.minute == 45) || (date_time.minute == 50) || (date_time.minute == 55)) {

            /* if it's 00:00 reset all counters */
            if ((date_time.hour == 0) && (date_time.minute == 0)) {
                interrupt_reset_total_energy_state();
            }

            /* update the post queue */
            queue_update_post_queue();

            /* reset the power and interval pulse count variables */
            interrupt_reset_power_state();

            if (OUTPUT_CLIENT == PVOUTPUT) {
                /* try to post the queue items and if it's successfull empty the queue */
                queue_post_items_to_pvoutput();
            } else if (OUTPUT_CLIENT == THINGSPEAK) {
                /* try to post the queue items and if it's successfull empty the queue */
                queue_post_items_to_thingspeak();
            }
        }
    } else if (queue_post_interval == 15) {
        /* if the queue post interval is 15, post the queue every 15 minutes */
        if ((date_time.minute == 0) || (date_time.minute == 15) || (date_time.minute == 30) || (date_time.minute == 45)) {
            /* if it's 00:00 reset all counters */
            if ((date_time.hour == 0) && (date_time.minute == 0)) {
                interrupt_reset_total_energy_state();
            }

            /* update the post queue */
            queue_update_post_queue();

            /* reset the power and interval pulse count variables */
            interrupt_reset_power_state();

            if (OUTPUT_CLIENT == PVOUTPUT) {
                /* try to post the queued items and if it's successfull empty the queue */
                queue_post_items_to_pvoutput();
            } else if (OUTPUT_CLIENT == THINGSPEAK) {
                /* try to post the queue items and if it's successfull empty the queue */
                queue_post_items_to_thingspeak();
            }
        }
    }
}

void
scheduler_init(void)
{
#ifdef DEBUG
    os_printf("[debug] scheduler_init\r\n");
#endif

    /* first execute the scheduler every second to determine the current time */
    os_timer_disarm(&scheduler_timer);
    os_timer_setfn(&scheduler_timer, (os_timer_func_t *)scheduler_check_tasks, NULL);
    os_timer_arm(&scheduler_timer, scheduler_timer_ms, 1);
}
