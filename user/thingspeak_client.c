/*
 * esp8266-pvoutput-logger project - thingspeak client functions
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
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"

#include "thingspeak_client.h"
#include "led.h"
#include "date_time.h"
#include "queue.h"
#include "config.h"

#define THINGSPEAK_WEBSERVER_IP         "54.164.214.198"
#define THINGSPEAK_WEBSERVER_HOSTNAME   "api.thingspeak.com"
#define THINGSPEAK_WEBSERVER_PORT       80

struct espconn *tp_esp_conn = NULL;

static ip_addr_t tp_webserver_ip;

LOCAL void ICACHE_FLASH_ATTR
thingspeak_disconnect_and_cleanup(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] thingspeak_disconnect_and_cleanup\r\n");
#endif

    struct espconn *tp_esp_conn = (struct espconn *)arg;

    /* and cleanup the connection pointers */
    if(tp_esp_conn->proto.tcp != NULL) { 
        os_free(tp_esp_conn->proto.tcp);
    }

    os_free(tp_esp_conn);

#ifdef DEBUG
    os_printf("[debug] thingspeak_disconnect_and_cleanup: disconnected from server.\r\n");
#endif
}

LOCAL void ICACHE_FLASH_ATTR
thingspeak_post_data(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] thingspeak_post_data\r\n");
#endif

    int i;

    struct espconn *tp_esp_conn = (struct espconn *)arg;

    /* allocate 128 bytes for the basic URL string and 64 bytes per queued item */
    char *http_code = (uint8 *) os_zalloc(128 + (queue_count * 64));

#ifdef DEBUG
    os_printf("[debug] thingspeak_post_data: connected to server\r\n");
#endif

    /*
     * ThingSpeak API
     *
     * https://nl.mathworks.com/help/thingspeak/update-channel-feed.html
     *
     * Update a Channel Feed
     *
     * To update a channel feed, send an HTTP GET or POST to https://api.thingspeak.com/update.
     *
     * Valid parameters:
     *
     * api_key (string) - Write API key for this specific channel (required). The Write API key can optionally be sent via a THINGSPEAKAPIKEY HTTP header.
     * field1 (string) - Field 1 data (optional)
     * field2 (string) - Field 2 data (optional)
     * field3 (string) - Field 3 data (optional)
     * field4 (string) - Field 4 data (optional)
     * field5 (string) - Field 5 data (optional)
     * field6 (string) - Field 6 data (optional)
     * field7 (string) - Field 7 data (optional)
     * field8 (string) - Field 8 data (optional)
     * lat (decimal) - Latitude in degrees (optional)
     * long (decimal) - Longitude in degrees (optional)
     * elevation (integer) - Elevation in meters (optional)
     * status (string) - Status update message (optional)
     * twitter (string) - Twitter username linked to ThingTweet (optional)
     * tweet (string) - Twitter status update; see Update Twitter Status for more information (optional)
     * created_at (datetime) - Date when this feed entry was created, in ISO 8601 format, for example: 2014-12-31 23:59:59. Time zones can be specified via the timezone parameter (optional)
     * Text
     * Example POST:
     *
     * POST https://api.thingspeak.com/update
     *      api_key=XXXXXXXXXXXXXXXX
     *      field1=73
     */

    /* clear the http_code buffer */
    os_memset(http_code, 0, sizeof(http_code)); 

    /* construct the complete thingspeak URL */
    for(i = 0; i < queue_count; i++) {
        os_sprintf(http_code + os_strlen(http_code), "GET /update?api_key=%s&%s=%ld&%s=%ld&created_at=%s %s HTTP/1.1\r\nHOST: %s\r\n\r\n",
            THINGSPEAK_APIKEY, THINGSPEAK_POWER_FIELD, pq[i].q_power_gen, THINGSPEAK_ENERGY_FIELD, pq[i].q_total_energy_gen,  pq[i].q_date, pq[i].q_time, THINGSPEAK_WEBSERVER_HOSTNAME);
    }

#ifdef DEBUG
    os_printf("[debug] thingspeak_post_data: http_code %d [\r\n\r\n%s\r\n\r\n]\r\n\r\n", i+2, http_code);
#endif

    /* sent the data to the webserver */
    if (espconn_send(tp_esp_conn, http_code, strlen(http_code)) != ESPCONN_OK) {
        /* if the connection was not ok, turn on the red light and disconnect from the server */
        os_printf("[%s] [error] thingspeak_post_data: failed to post the data: [%s\r\n\r\n]", date_time_get_ts(), http_code);

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);

        espconn_disconnect(tp_esp_conn);
#ifdef DEBUG
    } else {
        os_printf("[debug] thingspeak_post_data: tcp connection was successfull!\n");
#endif
    }

    /* cleanup */
    os_free(http_code);
}

LOCAL void ICACHE_FLASH_ATTR
thingspeak_check_http_return_code(void *arg, char *pdata, unsigned short len)
{
#ifdef DEBUG
    os_printf("[debug] thingspeak_check_http_return_code\r\n");
#endif

    struct espconn *tp_esp_conn = (struct espconn *)arg;

    int i;
    int post_success = 0;

#ifdef DEBUG
    os_printf("[debug] thingspeak_check_http_return_code:\r\n%s\r\n", pdata);
#endif

    /* check for a "HTTP/1.1 200 OK" return code */
    for (i = 0; i < len; i++) {
        if ((pdata[i] == 'H') && (pdata[i+1] == 'T') && (pdata[i+2] == 'T') && (pdata[i+3] == 'P') &&
            (pdata[i+4] == '/') && (pdata[i+5] == '1') && (pdata[i+6] == '.') && (pdata[i+7] == '1') &&
            (pdata[i+8] == ' ') && (pdata[i+9] == '2') && (pdata[i+10] == '0') && (pdata[i+11] == '0')) {
#ifdef DEBUG
            os_printf("[debug] thingspeak_check_http_return_code: successfully posted the data.\r\n");
#endif

            post_success = 1;

            break;
        }
    }

    /* check if the data is successfully posted */
    if (post_success == 1) {
#ifdef DEBUG
        os_printf("[debug] thingspeak_check_http_return_code: reset the counter.\r\n");
#endif

        /* turn off the red led */
        LED_toggle(RED_LED, LED_OFF);

        /* reset the queue counter */
        queue_count = 0;
    } else {
        os_printf("[%s] [error] thingspeak_check_http_return_code: failed to post the data!\r\n", date_time_get_ts());

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);
    }

    /* if the data is received disconnect from the server */
    espconn_disconnect(tp_esp_conn);
}

LOCAL void ICACHE_FLASH_ATTR
thingspeak_connect_to_webserver(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] thingspeak_connect_to_webserver\r\n");
#endif

    struct espconn *tp_esp_conn = (struct espconn *)arg;

    /* uncomment this if you use an ip address instead of a hostname */ 
    //tp_webserver_ip.addr = ipaddr_addr(THINGSPEAK_WEBSERVER_IP);

    /* I think this check is not necessary because it's already done in thingspeak_esp_platform_dns_found */
    if (tp_webserver_ip.addr == 0) {
        os_printf("[%s] [error] thingspeak_connect_to_webserver: no ip address resolved!\r\n", date_time_get_ts());

        /* no ip address resolved, cleanup and try again later */
        thingspeak_disconnect_and_cleanup(tp_esp_conn);

        return;
    }

    /* setup the esp connection struct */
    tp_esp_conn->type = ESPCONN_TCP;
    tp_esp_conn->state = ESPCONN_NONE;
    tp_esp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

    /* register webserver ip address and tcp port */
    os_memcpy(tp_esp_conn->proto.tcp->remote_ip, &tp_webserver_ip.addr, 4);
    tp_esp_conn->proto.tcp->remote_port = THINGSPEAK_WEBSERVER_PORT;

    /* register tcp connect callback function */
    espconn_regist_connectcb(tp_esp_conn, thingspeak_post_data);

    /* register the data receive callback function */
    espconn_regist_recvcb(tp_esp_conn, thingspeak_check_http_return_code);

    /* resgiter the tcp disconnect callback function */
    espconn_regist_disconcb(tp_esp_conn, thingspeak_disconnect_and_cleanup);

    /* make the connection and wait for the callback */
    espconn_connect(tp_esp_conn);
}

LOCAL void ICACHE_FLASH_ATTR 
thingspeak_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg) { 
#ifdef DEBUG
    os_printf("[debug] thingspeak_esp_platform_dns_found\r\n");
#endif

    struct espconn *tp_esp_conn = (struct espconn *)arg;

    if (ipaddr != NULL) {
#ifdef DEBUG
        os_printf("[debug] thingspeak_esp_platform_dns_found: ip address for hostname resolved ->  %d.%d.%d.%d\r\n", 
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),  
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3)); 
#endif

        /* if the resolved ip address is different then the saved one, store it again */
        if (tp_webserver_ip.addr != ipaddr->addr)
            tp_webserver_ip.addr = ipaddr->addr;

        /* if the ip address is successfully resolved, connect to the webserver */
        thingspeak_connect_to_webserver(tp_esp_conn);
    } else {
        os_printf("[%s] [error] thingspeak_esp_platform_dns_found: failed to resolve the server hostname!\r\n", date_time_get_ts());

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);
    }
}

void
thingspeak_prepare_webserver_connection(void)
{
    /* allocate the connection struct */
    tp_esp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));

#ifdef DEBUG
    os_printf("[debug] thingspeak_prepare_webserver_connection\r\n");
#endif

    /* uncomment this if you use an ip address instead of a hostname and comment the espconn_gethostbyname function below */ 
    //thingspeak_connect_to_webserver(pvo_esp_conn);

    /* get the ip address from the webserver hostname */
    espconn_gethostbyname(tp_esp_conn, THINGSPEAK_WEBSERVER_HOSTNAME, &tp_webserver_ip, thingspeak_esp_platform_dns_found);
}
