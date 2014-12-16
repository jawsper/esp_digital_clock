#include <c_types.h>
#include "lcd.h"
#include "bignumbers.h"

uint8_t number_desc[8][8] = 
{
	{
		0x07, // B00111,
		0x0F, // B01111,
		0x0F, // B01111,
		0x0F, // B01111,
		0x0F, // B01111,
		0x0F, // B01111,
		0x0F, // B01111,
		0x07, // B00111
	},
	{
		0x1F, // B11111,
		0x1F, // B11111,
		0x1F, // B11111,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000
	},
	{
		0x1C, // B11100,
		0x1E, // B11110,
		0x1E, // B11110,
		0x1E, // B11110,
		0x1E, // B11110,
		0x1E, // B11110,
		0x1E, // B11110,
		0x1C, // B11100
	},
	{
		0x0F, // B01111,
		0x07, // B00111,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x03, // B00011,
		0x07, // B00111
	},
	{
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x1F, // B11111,
		0x1F, // B11111,
		0x1F, // B11111
	},
	{
		0x1E, // B11110,
		0x1C, // B11100,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x18, // B11000,
		0x1C, // B11100
	},
	{
		0x1F, // B11111,
		0x1F, // B11111,
		0x1F, // B11111,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x1F, // B11111,
		0x1F, // B11111
	},
	{
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x00, // B00000,
		0x07, // B00111,
		0x0F, // B01111
	}
};

uint8_t digits[10][6] = 
{
	/* 0 */ { 0, 1, 2, 0, 4, 2 },
	/* 1 */ { ' ', ' ', 2, ' ', ' ', 2 },
	/* 2 */ { 3, 6, 2, 0, 4, 4 },
	/* 3 */ { 3, 6, 2, 7, 4, 2 },
	/* 4 */ { 0, 4, 2, ' ', ' ', 2 },
	/* 5 */ { 0, 6, 5, 7, 4, 2 },
	/* 6 */ { 0, 6, 5, 0, 4, 2 },
	/* 7 */ { 1, 1, 2, ' ', ' ', 2 },
	/* 8 */ { 0, 6, 2, 0, 4, 2 },
	/* 9 */ { 0, 6, 2, 7, 4, 2 },
};


ICACHE_FLASH_ATTR
void bignumbers_init()
{
	lcd_write(0);
	lcd_create_char(0, number_desc[0]);
	lcd_create_char(1, number_desc[1]);
	lcd_create_char(2, number_desc[2]);
	lcd_create_char(3, number_desc[3]);
	lcd_create_char(4, number_desc[4]);
	lcd_create_char(5, number_desc[5]);
	lcd_create_char(6, number_desc[6]);
	lcd_create_char(7, number_desc[7]);
}

ICACHE_FLASH_ATTR
void bignumbers_print(uint8_t val, uint8_t pos)
{
	if(val > 9) return;
	lcd_set_cursor(pos, 0);
	lcd_write(digits[val][0]);
	lcd_write(digits[val][1]);
	lcd_write(digits[val][2]);
	lcd_set_cursor(pos, 1);
	lcd_write(digits[val][3]);
	lcd_write(digits[val][4]);
	lcd_write(digits[val][5]);
}

ICACHE_FLASH_ATTR
void bignumbers_clear(uint8_t pos)
{
	lcd_set_cursor(pos, 0);
	lcd_print("   ");
	lcd_set_cursor(pos, 1);
	lcd_print("   ");
}
