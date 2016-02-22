/*
 * esp8266-pvoutput-logger project - pvoutput client functions
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

#include "pvoutput_client.h"
#include "led.h"
#include "date_time.h"
#include "queue.h"
#include "config.h"

#define PVOUTPUT_WEBSERVER_IP       "104.237.141.197"
#define PVOUTPUT_WEBSERVER_HOSTNAME "pvoutput.org"
#define PVOUTPUT_WEBSERVER_PORT     80

struct espconn *pvo_esp_conn = NULL;

static ip_addr_t pvo_webserver_ip;

LOCAL void ICACHE_FLASH_ATTR
pvoutput_disconnect_and_cleanup(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] pvoutput_disconnect_and_cleanup\r\n");
#endif

    struct espconn *pvo_esp_conn = (struct espconn *)arg;

    /* and cleanup the connection pointers */
    if(pvo_esp_conn->proto.tcp != NULL) { 
        os_free(pvo_esp_conn->proto.tcp);
    }

    os_free(pvo_esp_conn);

#ifdef DEBUG
    os_printf("[debug] pvoutput_disconnect_and_cleanup: disconnected from server.\r\n");
#endif
}

LOCAL void ICACHE_FLASH_ATTR
pvoutput_post_data(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] pvoutput_post_data\r\n");
#endif

    int i;

    struct espconn *pvo_esp_conn = (struct espconn *)arg;

    /* allocate 128 bytes for the basic URL string and 64 bytes per queued item */
    char *http_code = (uint8 *) os_zalloc(128 + (queue_count * 64));

#ifdef DEBUG
    os_printf("[debug] pvoutput_post_data: connected to server\r\n");
#endif

    /*
     * PVOutput API
     *
     * http://www.pvoutput.org/help.html#api-spec
     *
     * http://pvoutput.org/service/r2/addbatchstatus.jsp
     *
     * Parameters
     * 
     * Parameter   Field            Required  Example
     * data        Delimited        Yes       See Below
     * c1          Cumulative Flag  No        1
     *
     * Data Structure
     * The data parameter consists of up to 30 statuses, each status contains multiple fields.
     *
     * Field Delimiter   Status Delimiter
     * ,                 ;
     *
     * The following table specifies the composition of each status
     *
     * Field               Required  Format    Unit        Example   Since   
     * Date                Yes       yyyymmdd  date        20100830  r1  
     * Time                Yes       hh:mm     time        14:12     r1  
     * Energy Generation   Yes       number    watt hours  10000     r1   <-- total energy generated today
     * Power Generation    No        number    watts       2000      r1   <-- average power between queue_post_interval
     * Energy Consumption  No        number    watt hours  10000     r1  
     * Power Consumption   No        number    watts       2000      r1  
     * Temperature         No        decimal   celsius     23.4      r1  
     * Voltage             No        decimal   volts       240.7     r1
     *
     * Send a single status with Generation Energy 850Wh, Generation Power 1109W, Temperature 23.1C and Voltage 240V
     * curl -d "data=20110112,10:15,850,1109,-1,-1,23.1,240" -H "X-Pvoutput-Apikey: Your-API-Key" -H "X-Pvoutput-SystemId: Your-System-Id" http://pvoutput.org/service/r2/addbatchstatus.jsp
     */

    /* construct the beginning of the pvoutput URL */
    os_sprintf(http_code, "GET /service/r2/addbatchstatus.jsp?key=%s&sid=%s&data=", PVOUTPUT_APIKEY, PVOUTPUT_SYSTEMID);

    // TODO: pvoutput support max 30 records in batch mode

    /* construct the data part of the pvoutput URL */
    for(i = 0; i < queue_count; i++) {
        os_sprintf(http_code + os_strlen(http_code), "%s,%s,%ld,%ld", pq[i].q_date, pq[i].q_time, pq[i].q_total_energy_gen, pq[i].q_power_gen);

        if ((i + 1) < queue_count)
            os_sprintf(http_code + os_strlen(http_code), ";"); 
    }

    /* construct the end of the pvoutput URL */
    os_sprintf(http_code + os_strlen(http_code), " HTTP/1.1\r\nHOST: %s\r\n\r\n", PVOUTPUT_WEBSERVER_HOSTNAME);

#ifdef DEBUG
    os_printf("[debug] pvoutput_post_data: http_code %d [\r\n\r\n%s\r\n\r\n]\r\n\r\n", i+2, http_code);
#endif

    /* sent the data to the webserver */
    if (espconn_send(pvo_esp_conn, http_code, strlen(http_code)) != ESPCONN_OK) {
        /* if the connection was not ok, turn on the red light and disconnect from the server */
        os_printf("[%s] [error] pvoutput_post_data: failed to post the data: [%s\r\n\r\n]", date_time_get_ts(), http_code);

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);

        espconn_disconnect(pvo_esp_conn);
#ifdef DEBUG
    } else {
        os_printf("[debug] pvoutput_post_data: tcp connection was successfull!\n");
#endif
    }

    /* cleanup */
    os_free(http_code);
}

LOCAL void ICACHE_FLASH_ATTR
pvoutput_check_http_return_code(void *arg, char *pdata, unsigned short len)
{
#ifdef DEBUG
    os_printf("[debug] pvoutput_check_http_return_code\r\n");
#endif

    struct espconn *pvo_esp_conn = (struct espconn *)arg;

    int i;
    int post_success = 0;

#ifdef DEBUG
    os_printf("[debug] pvoutput_check_http_return_code:\r\n%s\r\n", pdata);
#endif

    /* check for a "HTTP/1.1 200 OK" return code */
    for (i = 0; i < len; i++) {
        if ((pdata[i] == 'H') && (pdata[i+1] == 'T') && (pdata[i+2] == 'T') && (pdata[i+3] == 'P') &&
            (pdata[i+4] == '/') && (pdata[i+5] == '1') && (pdata[i+6] == '.') && (pdata[i+7] == '1') &&
            (pdata[i+8] == ' ') && (pdata[i+9] == '2') && (pdata[i+10] == '0') && (pdata[i+11] == '0')) {
#ifdef DEBUG
            os_printf("[debug] pvoutput_check_http_return_code: successfully posted the data.\r\n");
#endif

            post_success = 1;

            break;
        }
    }

    /* check if the data is successfully posted */
    if (post_success == 1) {
#ifdef DEBUG
        os_printf("[debug] pvoutput_check_http_return_code: reset the counter.\r\n");
#endif

        /* turn off the red led */
        LED_toggle(RED_LED, LED_OFF);

        /* reset the queue counter */
        queue_count = 0;
    } else {
        os_printf("[%s] [error] pvoutput_check_http_return_code: failed to post the data!\r\n", date_time_get_ts());

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);
    }

    /* if the data is received disconnect from the server */
    espconn_disconnect(pvo_esp_conn);
}

LOCAL void ICACHE_FLASH_ATTR
pvoutput_connect_to_webserver(void *arg)
{
#ifdef DEBUG
    os_printf("[debug] pvoutput_connect_to_webserver\r\n");
#endif

    struct espconn *pvo_esp_conn = (struct espconn *)arg;

    /* uncomment this if you use an ip address instead of a hostname */ 
    //pvo_webserver_ip.addr = ipaddr_addr(PVOUTPUT_WEBSERVER_IP);

    /* I think this check is not necessary because it's already done in pvoutput_esp_platform_dns_found */
    if (pvo_webserver_ip.addr == 0) {
        os_printf("[%s] [error] pvoutput_connect_to_webserver: no ip address resolved!\r\n", date_time_get_ts());

        /* no ip address resolved, cleanup and try again later */
        pvoutput_disconnect_and_cleanup(pvo_esp_conn);

        return;
    }

    /* setup the esp connection struct */
    pvo_esp_conn->type = ESPCONN_TCP;
    pvo_esp_conn->state = ESPCONN_NONE;
    pvo_esp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

    /* register webserver ip address and tcp port */
    os_memcpy(pvo_esp_conn->proto.tcp->remote_ip, &pvo_webserver_ip.addr, 4);
    pvo_esp_conn->proto.tcp->remote_port = PVOUTPUT_WEBSERVER_PORT;

    /* register tcp connect callback function */
    espconn_regist_connectcb(pvo_esp_conn, pvoutput_post_data);

    /* register the data receive callback function */
    espconn_regist_recvcb(pvo_esp_conn, pvoutput_check_http_return_code);

    /* resgiter the tcp disconnect callback function */
    espconn_regist_disconcb(pvo_esp_conn, pvoutput_disconnect_and_cleanup);

    /* make the connection and wait for the callback */
    espconn_connect(pvo_esp_conn);
}

LOCAL void ICACHE_FLASH_ATTR 
pvoutput_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg) { 
#ifdef DEBUG
    os_printf("[debug] pvoutput_esp_platform_dns_found\r\n");
#endif

    struct espconn *pvo_esp_conn = (struct espconn *)arg;

    if (ipaddr != NULL) {
#ifdef DEBUG
        os_printf("[debug] pvoutput_esp_platform_dns_found: ip address for hostname resolved ->  %d.%d.%d.%d\r\n", 
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),  
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3)); 
#endif

        /* if the resolved ip address is different then the saved one, store it again */
        if (pvo_webserver_ip.addr != ipaddr->addr)
            pvo_webserver_ip.addr = ipaddr->addr;

        /* if the ip address is successfully resolved, connect to the webserver */
        pvoutput_connect_to_webserver(pvo_esp_conn);
    } else {
        os_printf("[%s] [error] pvoutput_esp_platform_dns_found: failed to resolve the server hostname!\r\n", date_time_get_ts());

        /* turn on the red led */
        LED_toggle(RED_LED, LED_ON);
    }
}

void
pvoutput_prepare_webserver_connection(void)
{
    /* allocate the connection struct */
    pvo_esp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));

#ifdef DEBUG
    os_printf("[debug] pvoutput_prepare_webserver_connection\r\n");
#endif

    /* uncomment this if you use an ip address instead of a hostname and comment the espconn_gethostbyname function below */ 
    //pvoutput_connect_to_webserver(pvo_esp_conn);

    /* get the ip address from the webserver hostname */
    espconn_gethostbyname(pvo_esp_conn, PVOUTPUT_WEBSERVER_HOSTNAME, &pvo_webserver_ip, pvoutput_esp_platform_dns_found);
}
