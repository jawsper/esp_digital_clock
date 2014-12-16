#include "lcd.h"
#include <osapi.h>
#include "driver/i2c.h"

LOCAL ICACHE_FLASH_ATTR
void lcd_hardware_init()
{
	uart0_init(9600);
	uart1_init(9600); // not relevant for lcd really...
	lcd_write(0);
}


LOCAL ICACHE_FLASH_ATTR
void tx(uint8_t val)
{
	uart0_tx_one_char(val);
	// DEBUG
	// char buff[10];
	// os_sprintf(buff, "tx: %02X\n", val);
	// uart1_sendStr(buff);
}

ICACHE_FLASH_ATTR
void lcd_command(uint8_t val)
{
	tx(0xFE);
	tx(val);
	os_delay_us(5000);
}

ICACHE_FLASH_ATTR
void lcd_specialCommand(uint8_t val)
{
	tx(0x7C);
	tx(val);
	os_delay_us(5000);
}

ICACHE_FLASH_ATTR
void lcd_write(uint8_t val)
{
	tx(val);
}

ICACHE_FLASH_ATTR
void lcd_set_cursor(uint8_t x, uint8_t y)
{
	int row_offset[4] = 
		{ 0x00, 0x40 }
	;
	if(x >= 0 && x < 16 && y >= 0 && y < 2)
	{
		lcd_command(LCD_SETDDRAMADDR | (x + row_offset[y]));
	}
}

ICACHE_FLASH_ATTR
void lcd_init()
{
	lcd_hardware_init();
	lcd_clear();
	lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
}

ICACHE_FLASH_ATTR
void lcd_clear()
{
	lcd_command(LCD_CLEARDISPLAY);
}

ICACHE_FLASH_ATTR
void lcd_create_char(uint8_t location, uint8_t charmap[])
{
	int i;
	for(i = 0; i < 8; i++)
	{
		lcd_command(LCD_SETCGRAMADDR | (location << 3) | i);
		lcd_write(charmap[i] & 0x1F);
	}
}

ICACHE_FLASH_ATTR
void lcd_print(const char* str)
{
	while(*str) lcd_write(*str++);
}

ICACHE_FLASH_ATTR
void lcd_set_brightness(uint8_t val)
{
	if(val >= 1 && val <= 30)
	{
		lcd_specialCommand(LCD_BACKLIGHT | (val - 1));
	}
}