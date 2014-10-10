#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f4/spi.h>

#include "lcd.h"

static void delay(int i)
{
	int j,k=0;
	while(k++ < i)
		for (j = 0; j <= 300; j++) {
      	  		__asm__("nop");
      	  	}
}

void transfer_command(int data1)
{
	gpio_clear(LCD_PORT,cs1);
	delay(1);
	gpio_clear(LCD_PORT,rs);
	delay(1);
	spi_send(SPI1,data1);
}

void transfer_data(int data1)
{
	gpio_clear(LCD_PORT,cs1);
	delay(1);
	gpio_set(LCD_PORT,rs);
	delay(1);
	spi_send(SPI1,data1);
}
void lcd_address(unsigned char page,unsigned char column)
{
	column = column - 1;
	page = page - 1;
	transfer_command(0xb0+page);
	transfer_command(((column>>4)&0x0f)+0x10);
	transfer_command(column&0x0f);
}

void clear_screen(void)
{
	unsigned char i,j;
	for(i=1;i<9;i++) {
		lcd_address(i,1);
		for(j=0;j<132;j++) {
			transfer_data(0x00);
		}
	}
}
void test_display(unsigned char data1)
{
	int i,j;
	for(j=0;j<8;j++) {
		lcd_address(j+1,0);
		for(i=0;i <128;i++) {
			transfer_data(data1);
		}			
	}		
}

void initial_lcd(void)
{
	gpio_clear(LCD_PORT,reset);
	/* 低电平复位*/
	delay(200);
	gpio_set(LCD_PORT,reset);
	/* 复位完毕*/
	delay(50);
	transfer_command(0xe2); 
	/* 软复位*/
	delay(5);
	transfer_command(0x2c); /* 升压步聚 1*/
	delay(5);
	transfer_command(0x2e); /* 升压步聚 2*/
	delay(5);
	transfer_command(0x2f); /* 升压步聚 3*/
	delay(5);
	transfer_command(0x24); /*粗调对比度 , 可设置范围0x20 ~ 0x27*/
	transfer_command(0x81); /*微调对比度*/
	transfer_command(0x0f); /*0x22,微调对比度的值 , 可设置范围0x00 ~ 0x3f*/
	transfer_command(0xa2); /*1/9 偏压比(bias ) */
	transfer_command(0xc8); /*行扫描顺序:从上到下 */
	transfer_command(0xa0); /*列扫描顺序:从左到右 */
	transfer_command(0x40); /*起始行:第一行开始*/
	transfer_command(0xaf); /*开显示*/
}

/*
 * x:0-127
 * y:0-63
 */
void lcd_draw_point(int x,int y)
{
	lcd_address((y>>3)+1,x+1);
	transfer_data(1<<(y%8));
}

void lcd_draw_image(const unsigned char image[])
{
	unsigned int pos=0;
	while(pos < 1024){
		lcd_address((pos>>7)+1,pos%128+1);
		transfer_data((int)image[pos]);
		pos++;
	}
}

