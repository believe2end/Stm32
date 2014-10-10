/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>,
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "image.h"


uint16_t exti_line_state;

/* Set STM32 to 168 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
}

static void gpio_setup(void)
{
	/* Enable GPIOD clock. */
	rcc_periph_clock_enable(RCC_GPIOD);

	/* Set GPIO12 (in GPIO port D) to 'output push-pull'. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
}

static void button_setup(void)
{
	/* Enable GPIOA clock. */
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Set GPIO0 (in GPIO port A) to 'input open-drain'. */
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
}

static void spi_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
					/* serial clock */
					GPIO5 |
					/* master in/slave out */
					GPIO6 |
					/* master out/slave in */
					GPIO7);
	gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6 | GPIO7);
	spi_disable_crc(SPI1);
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_32,
			/* high or low for the peripheral device */
			SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
			/* CPHA: Clock phase: read on rising edge of clock */
			SPI_CR1_CPHA_CLK_TRANSITION_2,
			/* DFF: Date frame format (8 or 16 bit) */
			SPI_CR1_DFF_8BIT,
			/* Most or Least Sig Bit First */
			SPI_CR1_MSBFIRST);

	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_enable(SPI1);
}



#define reset GPIO1
#define cs1   GPIO2
#define rs    GPIO3
#define sclk  GPIO4
#define sid   GPIO5
void delay(int i)
{
	int j,k=0;
	while(k++ < i)
		for (j = 0; j != 5000; j++) {
      	  		__asm__("nop");
      	  	}
}
void delay1(int i)
{
	int j,k=0;
	while(k++ < i)
		for (j = 0; j != 500; j++) {
      	  		__asm__("nop");
      	  	}
}

void transfer_command(int data1)
{
	char i;
	gpio_clear(GPIOE,cs1);
	gpio_clear(GPIOE,rs);
	//spi_write(SPI1,data1);
	spi_send(SPI1,data1);

	for(i=0;i<8;i++)
	{
		gpio_clear(GPIOE,sclk);
		if(data1&0x80) gpio_set(GPIOE,sid);
		else gpio_clear(GPIOE,sid);
		delay1(1);
		gpio_set(GPIOE,sclk);
		delay1(1);
		delay1(1);
		data1=data1<<=1;
	}
}

void transfer_data(int data1)
{
	char i;
	gpio_clear(GPIOE,cs1);
	gpio_set(GPIOE,rs);
	
//	spi_write(SPI1,data1);
	spi_send(SPI1,data1);
	for(i=0;i<8;i++)
	{
		gpio_clear(GPIOE,sclk);
		if(data1&0x80) gpio_set(GPIOE,sid);
		else gpio_clear(GPIOE,sid);
		gpio_set(GPIOE,sclk);
		data1=data1<<=1;
	}
}


void lcd_address(unsigned char page,unsigned char column)
{
	column=column - 1;
	page=page - 1;
	transfer_command(0xb0+page);
	transfer_command(((column>>4)&0x0f)+0x10);
	transfer_command(column&0x0f);
}
void clear_screen()
{
	unsigned char i,j;
	for(i=0;i<9;i++) {
		lcd_address(1+i,1);
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
		for(i=0;i<128;i++)
			for(i=0;i <128;i++) {
				transfer_data(data1);
			}			
	}		
}

void initial_lcd()
{
	gpio_clear(GPIOE,reset);
	/* 低电平复位*/
	delay(200);
	gpio_set(GPIOE,reset);
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

int main(void)
{
	int i;
	int j=0;

	clock_setup();

	rcc_peripheral_enable_clock(&RCC_APB2ENR,
				/* SPI 1 */
				RCC_APB2ENR_SPI1EN);

	rcc_periph_clock_enable(RCC_GPIOE);
	gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1|GPIO2|GPIO3|GPIO4|GPIO5);

	button_setup();
	gpio_setup();
	spi_setup();
	initial_lcd(); 

	lcd_address(8,128);
	transfer_data(0x11);

	/* Blink the LED (PD12) on the board. */
	while (1) {
		gpio_toggle(GPIOD, GPIO12);

		/* Upon button press, blink more slowly. */
		exti_line_state = GPIOA_IDR;
		if ((exti_line_state & (1 << 0)) != 0) {
			while ((exti_line_state & (1 << 0)) != 0) 
				exti_line_state = GPIOA_IDR;
			for (i = 0; i < 3000000; i++) {	/* Wait a bit. */
				__asm__("nop");
			}
			clear_screen();
			j++;
			lcd_address(j/128+1,j%128+1);
			transfer_data(0xFF);
		}
		clear_screen();
		j++;

		lcd_address(j/128%8+1,j%128+1);
		transfer_data(0xFF);

		for (i = 0; i < 3000000; i++) {		/* Wait a bit. */
			__asm__("nop");
			__asm__("nop");
		}
	}

	return 0;
}
