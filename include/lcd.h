#ifndef _LCD_H_
#define _LCD_H_

#include <c_types.h>

// Commands
#define LCD_BACKLIGHT		0x80
#define LCD_CLEARDISPLAY	0x01
#define LCD_CURSORSHIFT		0x10
#define LCD_DISPLAYCONTROL	0x08
#define LCD_ENTRYMODESET	0x04
#define LCD_FUNCTIONSET		0x20
#define LCD_SETCGRAMADDR	0x40
#define LCD_SETDDRAMADDR	0x80
#define LCD_SETSPLASHSCREEN	0x0A
#define LCD_SPLASHTOGGLE	0x09
#define LCD_RETURNHOME		0x02

// Flags for display entry mode
#define LCD_ENTRYRIGHT		0x00
#define LCD_ENTRYLEFT		0x02

// Flags for display on/off control
#define LCD_BLINKON		0x01
#define LCD_CURSORON		0x02
#define LCD_DISPLAYON		0x04

void lcd_command(uint8_t val);
void lcd_specialCommand(uint8_t val);
void lcd_create_char(uint8_t location, uint8_t charmap[]);
void lcd_set_cursor(uint8_t x, uint8_t y);

void lcd_init();
void lcd_clear();
void lcd_write(uint8_t val);
void lcd_print(const char* str);


#endif