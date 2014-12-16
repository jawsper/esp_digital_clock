/*
 * ESP8266 clock
 * Connects to WiFi, gets time from time server and prints on LCD screen with bignumbers.
 * Contains code from https://github.com/jawsper/BigNumbers
 * Author: Jasper Seidel
 * Copyright (c) 2014 All rights reserved.
 * License: GPL Version 3
 *
 * This file is released under GPL-v3.0, the license is found at http://www.gnu.org/copyleft/gpl.html
 *
**/

#include <osapi.h>

#include <ip_addr.h>
#include <espconn.h>
#include <user_interface.h>

#include "ntp.h"
#include "lcd.h"
#include "bignumbers.h"
#include "user_config.h"


#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void outer_loop(os_event_t *events);

struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} current_time;

ICACHE_FLASH_ATTR
bool display_time()
{
	bool update_time = false;
	char buff[8];

	uint8_t hour = time_hour();
	uint8_t minute = time_minute();
	uint8_t second = time_second();

	if(current_time.hour != hour)
	{
		current_time.hour = hour;
		bignumbers_print(hour / 10, 0);
		bignumbers_print(hour % 10, 3);
	}

	if(current_time.minute != minute)
	{
		current_time.minute = minute;
		bignumbers_print(minute / 10, 7);
		bignumbers_print(minute % 10, 10);
		update_time = true;
	}

	if(current_time.second != second)
	{
		current_time.second = second;
		lcd_set_cursor(14, 1);
		os_sprintf(buff, "%02d", second);
		lcd_print(buff);
	}

	return update_time;
}

bool init_done = false;

/*
 * loop()
 * Main program loop. Checks if connection succeeded then displays and updates time.
 * @return bool True if error occured.
 */
static ICACHE_FLASH_ATTR
bool loop()
{
	if(!init_done)
	{
		uint8_t status = wifi_station_get_connect_status();
		switch(status)
		{
			case STATION_IDLE:
			case STATION_CONNECTING:
				return false;
			
			case STATION_WRONG_PASSWORD:
			case STATION_NO_AP_FOUND:
			case STATION_CONNECT_FAIL:
				lcd_clear();
				lcd_print("Error: ");
				lcd_set_cursor(0, 1);
				switch(status)
				{
					case STATION_WRONG_PASSWORD:
						lcd_print("Wrong password.");
						return true;
					case STATION_NO_AP_FOUND:
						lcd_print("No AP found.");
						return true;
					case STATION_CONNECT_FAIL:
						lcd_print("Connect fail.");
						return true;
				}
				return;
			case STATION_GOT_IP:
				break;
		}

		struct ip_info ip;
		wifi_get_ip_info(STATION_IF, &ip);
		if(ip.ip.addr == 0)
			return false; // no error

		lcd_print("\xFE\x01IP: ");
		char buff[128];
		os_sprintf(buff, IPSTR, IP2STR(&ip.ip));
		lcd_print(buff);

		os_delay_us(1000000);

		lcd_clear();
		time_request();

		init_done = true;

		display_time();

		return;
	}
	bool update_time = display_time();

	if(update_time)
	{
		// time_request();
	}
}

//Main code function
static ICACHE_FLASH_ATTR
void outer_loop(os_event_t *events)
{
	bool error = loop();
	if(error) while(1);

    os_delay_us(10000);
    system_os_post(user_procTaskPrio, 0, 0 );
}

// void print_int(int val)
// {
// 	char buff[32];
// 	os_sprintf(buff, "%d", val);
// 	lcd_write(buff);
// }

ICACHE_FLASH_ATTR
void user_init(void)
{
	os_delay_us(1000 * 1000);
	lcd_init();
	bignumbers_init();
	os_delay_us(1000 * 1000);
	lcd_clear();
	lcd_print("init");

	time_init();

	current_time.hour = 0xFF;
	current_time.minute = 0xFF;
	current_time.second = 0xFF;

    //Start os task
    system_os_task(outer_loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    system_os_post(user_procTaskPrio, 0, 0 );
}
