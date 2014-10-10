#ifndef _LCD_H_
#define _LCD_H_

#define LCD_PORT GPIOE
#define reset GPIO1
#define cs1   GPIO2
#define rs    GPIO3
#define sclk  GPIO4
#define sid   GPIO5

void transfer_command(int data1);
void transfer_data(int data1);
void initial_lcd(void);
void lcd_address(unsigned char page,unsigned char column);
void test_display(unsigned char data1);
void clear_screen(void);
void lcd_draw_point(int x,int y);
void lcd_draw_image(const unsigned char image[]);

#endif
