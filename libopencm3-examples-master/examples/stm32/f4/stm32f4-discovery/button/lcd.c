#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f4/spi.h>

#include "lcd.h"

void delay(int i)
{
	int j,k=0;
	while(k++ < i)
		for (j = 0; j != 5000000; j++) {
      	  		__asm__("nop");
      	  	}
}
void delay1(int i)
{
	int j,k=0;
	while(k++ < i)
		for (j = 0; j != 5000000; j++) {
      	  		__asm__("nop");
      	  	}
}

void transfer_command(int data1)
{
	char i;
	gpio_clear(GPIOA,cs1);
	gpio_clear(GPIOA,rs);
	spi_write(SPI1,data1);

	for(i=0;i<8;i++)
	{
		gpio_clear(GPIOA,sclk);
		if(data1&0x80) gpio_set(GPIOA,sid);
		else gpio_clear(GPIOA,sid);
		delay1(1);
		gpio_set(GPIOA,sclk);
		delay1(1);
		delay1(1);
		data1=data1<<=1;
	}
}

void transfer_data(int data1)
{
	char i;
	gpio_clear(GPIOA,cs1);
	gpio_set(GPIOA,rs);
	
	spi_write(SPI1,data1);
	for(i=0;i<8;i++)
	{
		gpio_clear(GPIOA,sclk);
		if(data1&0x80) gpio_set(GPIOA,sid);
		else gpio_clear(GPIOA,sid);
		gpio_set(GPIOA,sclk);
		data1=data1<<=1;
	}
}


void initial_lcd()
{
	gpio_clear(GPIOA,reset);
	/* 低电平复位*/
	delay(200);
	gpio_set(GPIOA,reset);
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
