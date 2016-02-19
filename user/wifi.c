/*
 * esp8266-pvoutput-logger project - wifi functions
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

#include "wifi.h"
#include "led.h"
#include "config.h"

LOCAL os_timer_t wifi_con_test_timer;

void ICACHE_FLASH_ATTR
esp_platform_check_ip(void)
{
#ifdef DEBUG
    os_printf("[debug] esp_platform_check_ip\r\n");
#endif

    struct ip_info ipconfig;

    /* disarm the timer first */
    os_timer_disarm(&wifi_con_test_timer);

    /* get the ip info of the ESP8266 station */
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
#ifdef DEBUG
        os_printf("[debug] esp_platform_check_ip: ip address is configured.\r\n");
#endif

        /* turn on the blue led */
        LED_toggle(BLUE_LED, LED_ON);

        /* re-arm the timer and check for the ip address again after 5 minutes.
         * this indicates if we still have a wifi connection.
         */
        os_timer_setfn(&wifi_con_test_timer, (os_timer_func_t *)esp_platform_check_ip, NULL);
        os_timer_arm(&wifi_con_test_timer, 300000, 0);
    } else {
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {    
            os_printf("[error] esp_platform_check_ip: connection failed!\r\n"); 

            /* no wifi connection -> turn off the blue led */
            LED_toggle(BLUE_LED, LED_OFF);
        } else {
            /* re-arm the timer and check for the ip address */
            os_timer_setfn(&wifi_con_test_timer, (os_timer_func_t *)esp_platform_check_ip, NULL);
            os_timer_arm(&wifi_con_test_timer, 100, 0);
        }
    }
}

void ICACHE_FLASH_ATTR
set_station_config(void)
{
#ifdef DEBUG
    os_printf("[debug] set_station_config\r\n");
#endif

    struct station_config stationConf; 
  
    /* clear the ssid and password buffers */ 
    os_memset(stationConf.ssid, 0, 32);
    os_memset(stationConf.password, 0, 64);

    /* we don't need a mac address */
    stationConf.bssid_set = 0; 
   
    /* configure the AP settings */ 
    os_memcpy(&stationConf.ssid, WIFI_SSID, 32); 
    os_memcpy(&stationConf.password, WIFI_PASS, 64); 
    wifi_station_set_config(&stationConf); 

    /* set a timer to check whether we got an ip from the router */
    os_timer_disarm(&wifi_con_test_timer);
    os_timer_setfn(&wifi_con_test_timer, (os_timer_func_t *)esp_platform_check_ip, NULL);
    os_timer_arm(&wifi_con_test_timer, 100, 0);
}

void
wifi_init(void)
{
    /* set the chip in softAP + station mode */
    wifi_set_opmode(STATIONAP_MODE); 

    /* connect to the router */
    set_station_config();
}
