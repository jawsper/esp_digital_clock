/*
 * Author: jawsper
 * Author: drbytes for the NTP part (https://tkkrlab.nl/wiki/DoorAccess)
 * Copyright (c) 2014 All rights reserved.
 * License: GPL Version 3
 *
 * This file is released under GPL-v3.0, the license is found at http://www.gnu.org/copyleft/gpl.html
 * 
 */

#include <c_types.h>
#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include <ip_addr.h>
#include <espconn.h>


#include "ntp.h"
#include "time.h"
#include "lcd.h"
#include "driver/uart.h"


#define NTP_PORT 123
#define NTP_PACKET_SIZE 48

#ifndef DEBUG
#define DEBUG_PRINT(x) uart1_sendStr(x)
#else
#define DEBUG_PRINT(x)
#endif

static struct espconn *pCon = 0;
static uint8_t packetBuffer[NTP_PACKET_SIZE];
static char temp[128]; // string buffer

// functions
static ICACHE_FLASH_ATTR void ntp_recv(void *arg, char *pdata, unsigned short len);
static ICACHE_FLASH_ATTR void ntp_timeout_handler(void* arg);
static ICACHE_FLASH_ATTR void udp_close(struct espconn**);
static ICACHE_FLASH_ATTR void ntp_request_addr(char* addr);
static ICACHE_FLASH_ATTR void ntp_request(uint32_t ip);
static ICACHE_FLASH_ATTR void ntp_refresh(void* arg);
static ICACHE_FLASH_ATTR void time_tick(void* arg);

// clock
static time_t current_time = 0;
static os_timer_t clock;
static os_timer_t ntp_refresh_timer;

ICACHE_FLASH_ATTR
void time_init()
{
    // arm & start a timer that calls the time_tick function
    os_timer_disarm(&clock);
    os_timer_setfn(&clock, (os_timer_func_t *)time_tick, NULL);
    os_timer_arm(&clock, 1000, 1);

    // os_timer_setfn(&ntp_refresh_timer, (os_timer_func_t*)ntp_refresh, NULL);
    // os_timer_arm(&ntp_refresh_timer, 3600000, 1); // update every hour
}

// LOCAL ICACHE_FLASH_ATTR
// void ntp_refresh(void* arg)
// {
// 	ntp_request_addr(NTP_ADDR);
// }

LOCAL ICACHE_FLASH_ATTR
void time_tick(void* arg)
{
	current_time++;
}

// NTP code
static os_timer_t ntp_timeout;
static ip_addr_t target_ip;

LOCAL ICACHE_FLASH_ATTR
void dns_found(const char *name, ip_addr_t *ipaddr, void *callback_arg)
{
	if(ipaddr == NULL)
	{
		// error
		DEBUG_PRINT("[DNS] No IP found.\r\n");
		return;
	}

	if(ipaddr->addr == 0)
	{
		// error
		DEBUG_PRINT("[DNS] No IP found...?\r\n");
		return;
	}

	DEBUG_PRINT("[DNS] Succes!\r\n");

	ntp_request(ipaddr->addr);
}

LOCAL ICACHE_FLASH_ATTR
void ntp_request_addr(char* ip_str)
{
	uint32_t ip = ipaddr_addr(ip_str);

    if((ip == 0xffffffff) && (os_memcmp(ip_str, "255.255.255.255", 16) != 0))
    {
		err_t status = espconn_gethostbyname(pCon, ip_str, &target_ip, dns_found);
		if(status != ESPCONN_OK)
		{
			// either have to wait or error, either way just give up this here
			return;
		}
		ip = target_ip.addr;
    }

    ntp_request(ip);
}

LOCAL ICACHE_FLASH_ATTR
void ntp_request(uint32_t ip)
{
	DEBUG_PRINT("[NTP] Sending NTP packet!\r\n");
	char buff[256];
	os_sprintf(buff, "[NTP] IP: "IPSTR"\r\n", IP2STR(&ip));
	DEBUG_PRINT(buff);

	pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
	pCon->type = ESPCONN_UDP;
	pCon->state = ESPCONN_NONE;
	pCon->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pCon->proto.udp->local_port = espconn_port();
	pCon->proto.udp->remote_port = NTP_PORT;
	os_memcpy(pCon->proto.udp->remote_ip, &ip, 4);
	
    espconn_regist_recvcb(pCon, ntp_recv);
	espconn_create(pCon);
	
	os_memset(packetBuffer, 0, NTP_PACKET_SIZE);	// clear buffer
	// Initialize values needed to form NTP request
	packetBuffer[0] = 0b11100011;   	// LI, Version, Mode
	packetBuffer[1] = 0;     			// Stratum, or type of clock
	packetBuffer[2] = 6;     			// Polling Interval
	packetBuffer[3] = 0xEC;  			// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	// all NTP fields have been given values, now
	
	espconn_sent(pCon, packetBuffer, NTP_PACKET_SIZE);
	
	DEBUG_PRINT("[NTP] UDP packet sent!\r\n");

	os_timer_disarm(&ntp_timeout);
	os_timer_setfn(&ntp_timeout, (os_timer_func_t*)ntp_timeout_handler, NULL);
	os_timer_arm(&ntp_timeout, 30000, 1); // 30 second timeout
}

LOCAL ICACHE_FLASH_ATTR
void ntp_timeout_handler(void* arg)
{
	DEBUG_PRINT("[NTP] Timeout!\r\n");
	udp_close(&pCon);
}

LOCAL ICACHE_FLASH_ATTR
void ntp_recv(void *arg, char *pdata, unsigned short len)
{
	os_timer_disarm(&ntp_timeout);
	time_t timeStamp = pdata[40];     //the timestamp starts at byte 40 of the received packet and is four bytes long.
	timeStamp = timeStamp << 8 | pdata[41];
	timeStamp = timeStamp << 8 | pdata[42];
	timeStamp = timeStamp << 8 | pdata[43];
	timeStamp -=  2208988800UL;						//timestamp is seconds since 1-1-1900. Add 70 years to get epoch, time since 1-1-1970
	timeStamp += 3UL;								//add 2 seconds because we requested a packet about 2 seconds ago and another second for network delays/fractions etc (estimate).
	
	timeStamp += TIMEZONE;

	udp_close(&pCon);
	DEBUG_PRINT("[NTP] Connection closed\r\n");

	current_time = timeStamp;
}

LOCAL ICACHE_FLASH_ATTR 
void udp_close(struct espconn **conn)
{
	// close this stuff
	espconn_delete(*conn);
	os_free((*conn)->proto.udp);
	os_free(*conn);
	*conn = 0;
}

ICACHE_FLASH_ATTR
void time_request()
{
	ntp_request_addr(NTP_ADDR);
}

ICACHE_FLASH_ATTR
void lcd_print_time()
{
	tmElements_t tm;
	breakTime(current_time, &tm);
	os_sprintf(temp, "\xFE\x01%04d-%02d-%02dT%02d:%02d:%02dZ", tmYearToCalendar(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
	lcd_print(temp);	
}

ICACHE_FLASH_ATTR
void time_cancel()
{
	if(pCon != 0) udp_close(&pCon);
}

uint8_t time_hour()
{
	tmElements_t tm;
	breakTime(current_time, &tm);
	return tm.Hour;
}

uint8_t time_minute()
{
	tmElements_t tm;
	breakTime(current_time, &tm);;
	return tm.Minute;
}

uint8_t time_second()
{
	tmElements_t tm;
	breakTime(current_time, &tm);
	return tm.Second;
}