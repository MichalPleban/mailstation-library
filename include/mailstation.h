#ifndef __MAILSTATION_H__
#define __MAILSTATION_H__

#include <stdint.h>
#include <stdbool.h>

/***************************************************************************************
 * Data structures used by the library.
 ***************************************************************************************/

#define MS_KEYBOARD_BUFFER_SIZE	16

typedef struct ms_port_shadow_t {
    uint8_t irq_mask;       // Port 0x03 - IRQ mask
    uint8_t gpio1;          // Port 0x02 - GPIO1 pins
    uint8_t gpio2;          // Port 0x09 - GPIO2 pins
    uint8_t gpio3;          // Port 0x21 - GPIO3 pins
    uint8_t gpio4;          // Port 0x28 - GPIO4 pins
    uint8_t gpio5;          // Port 0x2D - GPIO5 pins
    uint8_t cpu_speed;      // Port 0x0A - CPU speed control
    uint8_t unused;
} ms_port_shadow_t;

typedef struct ms_screen_t {
	uint8_t type;
	uint8_t test;
    uint8_t cursor_x;
    uint8_t cursor_y;
	uint8_t cursor_enabled;
	bool cursor_state;
	uint8_t cursor_blink_counter;
} ms_screen_t;

typedef struct ms_keyboard_t {
	uint16_t buffer[MS_KEYBOARD_BUFFER_SIZE];
	uint8_t buffer_head;
	uint8_t buffer_tail;
	uint8_t modifier_state;
	uint8_t last_key;
	bool key_pressed;
	bool caps_pressed;
	uint8_t repeat_delay;
} ms_keyboard_t;

/***************************************************************************************
 * Hardware port definitions.
 ***************************************************************************************/

__sfr __at 0x02 ms_port_gpio1;
__sfr __at 0x0B ms_port_gpio1_dir;
__sfr __at 0x09 ms_port_gpio2;
__sfr __at 0x0A ms_port_gpio2_dir;
__sfr __at 0x21 ms_port_gpio3;
__sfr __at 0x28 ms_port_gpio4;
__sfr __at 0x29 ms_port_gpio4_dir;
__sfr __at 0x2D ms_port_gpio5;
__sfr __at 0x2C ms_port_gpio5_dir;

__sfr __at 0x01 ms_port_keyboard;

__sfr __at 0x03 ms_port_irq_mask;

__sfr __at 0x05 ms_port_4000_page;
__sfr __at 0x06 ms_port_4000_device;
__sfr __at 0x07 ms_port_8000_page;
__sfr __at 0x08 ms_port_8000_device;
__sfr __at 0x00 ms_port_screen_page;

__sfr __at 0x0D ms_port_cpu_speed;

__sfr __at 0x10 ms_port_seconds_low;
__sfr __at 0x11 ms_port_seconds_high;
__sfr __at 0x12 ms_port_minutes_low;
__sfr __at 0x13 ms_port_minutes_high;
__sfr __at 0x14 ms_port_hours_low;
__sfr __at 0x15 ms_port_hours_high;
__sfr __at 0x16 ms_port_weekday;
__sfr __at 0x17 ms_port_day_low;
__sfr __at 0x18 ms_port_day_high;
__sfr __at 0x19 ms_port_month_low;
__sfr __at 0x1A ms_port_month_high;
__sfr __at 0x1B ms_port_year_low;
__sfr __at 0x1C ms_port_year_high;
__sfr __at 0x1D ms_port_rtc_control;
__sfr __at 0x1E ms_port_rtc_test;
__sfr __at 0x1F ms_port_rtc_reset;

__sfr __at 0x2F ms_port_irq_speed;

/***************************************************************************************
 * GPIO pin definitions.
 ***************************************************************************************/

#define MS_GPIO1_LCD_COLUMN     	0x08
#define MS_GPIO1_LED_ON		    	0x10
#define MS_GPIO1_LCD_ON		    	0x80
#define MS_GPIO2_POWER_BUTTON		0x10
#define MS_GPIO4_POWER_OFF			0x01

#define MS_DEVICE_CODEFLASH	    	0
#define MS_DEVICE_RAM		    	1
#define MS_DEVICE_LCD_LEFT	    	2
#define MS_DEVICE_DATAFLASH	    	3
#define MS_DEVICE_LCD_RIGHT	    	4
#define MS_DEVICE_MODEM		    	5

#define MS_SCREEN_OLD		  		0
#define MS_SCREEN_NEW		  		1

#define MS_CPU_12MHZ                0x00
#define MS_CPU_10MHZ                0x30
#define MS_CPU_8MHZ                 0xF0

#define MS_IRQ_1S                   0x40
#define MS_IRQ_2S                   0x50
#define MS_IRQ_4S                   0x60
#define MS_IRQ_8S                   0x70

/***************************************************************************************
 * IRQ levels.
 ***************************************************************************************/

#define MS_IRQ_LEVEL_64HZ           1
#define MS_IRQ_LEVEL_1HZ            4
#define MS_IRQ_LEVEL_RTC            5
#define MS_IRQ_LEVEL_MODEM          6
#define MS_IRQ_LEVEL_CALLERID       7

/***************************************************************************************
 * Library variables
 * 
 * Because the library uses the memory at 0xC000-0xC1FF for interrupt handling,
 *  the variables should be placed in RAM starting at 0xC200.
 * The linker directive --data-loc 0xC200 can be used for this.
 ***************************************************************************************/

__at 0xC000 uint8_t ms_screen_buffer[5120];

extern ms_screen_t ms_screen;

extern ms_port_shadow_t ms_port_shadow;

extern ms_keyboard_t ms_keyboard;

/***************************************************************************************
 * Function prototypes.
 ***************************************************************************************/

void ms_init(void);
void ms_init_ports(void);
void ms_init_irq(void);
void ms_init_screen(void);
void ms_init_keyboard(void);

void ms_screen_clear(void);
void ms_screen_update(uint8_t *buffer);
void ms_screen_update_column(uint8_t column, uint8_t *buffer);
void ms_screen_scroll(bool refresh);
void ms_position_cursor(uint8_t x, uint8_t y);
void ms_draw_char(char c, bool refresh);
void ms_draw_string(const char *str, int length, bool refresh);
void ms_put_char(char c, bool refresh);
void ms_put_string(const char *str, bool refresh);
void ms_advance_cursor(bool scroll, bool refresh_on_scroll);
uint8_t ms_screen_type(void);

uint8_t ms_current_device(void);
uint8_t ms_current_bank(void);

bool ms_add_irq_handler(uint8_t level, uint8_t device, uint8_t bank, void (*handler)(void));
uint32_t ms_get_timer(void);

bool ms_keyboard_key_available(void);
uint16_t ms_keyboard_get_key(bool wait);

void ms_power_off(void);
void ms_enable_led(bool on);
void ms_enable_lcd(bool on);
void ms_cpu_speed(uint8_t speed);
void ms_irq_speed(uint8_t speed);

extern const uint8_t ms_screen_font[];

/***************************************************************************************
 * Key scancode definitions.
 ***************************************************************************************/

#define MS_KEY_MENU			0x00
#define MS_KEY_BACK			0x01
#define MS_KEY_PRINT		0x02
#define MS_KEY_F1			0x03
#define MS_KEY_F2			0x04
#define MS_KEY_F3			0x05
#define MS_KEY_F4			0x06
#define MS_KEY_F5			0x07
#define MS_KEY_AT			0x13
#define MS_KEY_SIZE			0x14
#define MS_KEY_SPELLING		0x15
#define MS_KEY_EMAIL		0x16
#define MS_KEY_PG_UP		0x17
#define MS_KEY_BACKTICK     0x20
#define MS_KEY_1			0x21
#define MS_KEY_2			0x22
#define MS_KEY_3			0x23
#define MS_KEY_4			0x24
#define MS_KEY_5			0x25
#define MS_KEY_6			0x26
#define MS_KEY_7			0x27
#define MS_KEY_8			0x30
#define MS_KEY_9			0x31
#define MS_KEY_0			0x32
#define MS_KEY_MINUS		0x33
#define MS_KEY_EQUALS		0x34
#define MS_KEY_BACKSPACE	0x35
#define MS_KEY_BACKSLASH    0x36
#define MS_KEY_PG_DOWN		0x37
#define MS_KEY_TAB			0x40
#define MS_KEY_Q			0x41
#define MS_KEY_W			0x42
#define MS_KEY_E			0x43
#define MS_KEY_R			0x44
#define MS_KEY_T			0x45
#define MS_KEY_Y			0x46
#define MS_KEY_U			0x47
#define MS_KEY_I			0x50
#define MS_KEY_O			0x51
#define MS_KEY_P			0x52
#define MS_KEY_LBRACKET		0x53
#define MS_KEY_RBRACKET		0x54
#define MS_KEY_SEMICOLON	0x55
#define MS_KEY_QUOTE		0x56
#define MS_KEY_ENTER		0x57
#define MS_KEY_CAPS  		0x60
#define MS_KEY_A			0x61
#define MS_KEY_S			0x62
#define MS_KEY_D			0x63
#define MS_KEY_F			0x64
#define MS_KEY_G			0x65
#define MS_KEY_H			0x66
#define MS_KEY_J			0x67
#define MS_KEY_K			0x70
#define MS_KEY_L			0x71
#define MS_KEY_COMMA		0x72
#define MS_KEY_DOT			0x73
#define MS_KEY_SLASH		0x74
#define MS_KEY_UP			0x75
#define MS_KEY_DOWN			0x76
#define MS_KEY_RIGHT		0x77
#define MS_KEY_LSHIFT		0x80
#define MS_KEY_Z			0x81
#define MS_KEY_X			0x82
#define MS_KEY_C			0x83
#define MS_KEY_V			0x84
#define MS_KEY_B			0x85
#define MS_KEY_N			0x86
#define MS_KEY_M			0x87
#define MS_KEY_FN           0x90
#define MS_KEY_SPACE        0x93
#define MS_KEY_RSHIFT       0x96
#define MS_KEY_LEFT         0x97
#define MS_KEY_POWER        0x0F

#define MS_MODIFIER_FN      0x80
#define MS_MODIFIER_SHIFT   0x40
#define MS_MODIFIER_CAPS    0x20

#define MS_KEY_NONE         0xFFFF

/***************************************************************************************
 * Structures for Mailstation OS app identification.
 ***************************************************************************************/

typedef const struct ms_app_name_t {
	uint16_t unknown;	    // Must be 0x0001
	uint16_t name_len;	    // Length of name, not including the terminating null byte
	uint16_t name_start;    // Relative to start of struct, likely always 0x0006
	char name[];		    // The actual app name, only first 11 characters are shown
} ms_app_name_t;

typedef const struct ms_app_icon_t {
	uint16_t icon0_size;	// Must be 0x00AD
	uint16_t icon0_start;	// Start of icon0_width, likely always 0x0008
	uint16_t icon1_size;	// Unused, needs to be zero
	uint16_t icon1_start;	// Unused, needs to be zero

	uint16_t icon0_width;	// Must be 34
	uint8_t icon0_height;	// Must be 34
	uint8_t icon0[][];	    // 34 rows, 5 bytes each, to represent 34x34
} ms_app_icon_t;


#endif // __MAILSTATION_H__
