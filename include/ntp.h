#ifndef _NTP_H_
#define _NTP_H_

#define NTP_ADDR "pool.ntp.org"
#define TIMEZONE 3600

void time_init();
void time_request();
void lcd_print_time();
void time_cancel();

uint8_t time_hour();
uint8_t time_minute();
uint8_t time_second();

#endif