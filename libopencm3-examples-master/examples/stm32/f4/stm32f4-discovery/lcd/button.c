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
#include <libopencm3/stm32/f4/rng.h>
#include <math.h>
#include "lcd.h"
#include "image.h"

uint16_t exti_line_state;

/* Set STM32 to 168 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
	/* Enable rng clock */
	rcc_periph_clock_enable(RCC_RNG);
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
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_2,
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

int main(void)
{
	int i;
	int j=0;
//	int rnd;
	unsigned const char *dd[] = {gImage_1_1,gImage_1_2,gImage_1_3,gImage_1_4};
	
	clock_setup();

	rcc_peripheral_enable_clock(&RCC_APB2ENR,
				/* SPI 1 */
				RCC_APB2ENR_SPI1EN);

	rcc_periph_clock_enable(RCC_GPIOE);
	gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1|GPIO2|GPIO3|GPIO4|GPIO5);

	button_setup();
	gpio_setup();
	spi_setup();

	gpio_set_output_options(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_100MHZ,
				GPIO5|GPIO6|GPIO7);
	initial_lcd(); 

	/* Blink the LED (PD12) on the board. */
	while (1) {
	//	rnd = (j<<1)-1-((j*j)>>6);
	//	j %= 128;
		gpio_toggle(GPIOD, GPIO12);

		/* Upon button press, blink more slowly. */
		exti_line_state = GPIOA_IDR;
		if ((exti_line_state & (1 << 0)) != 0) {
			clear_screen();
			while ((exti_line_state & (1 << 0)) != 0) 
				exti_line_state = GPIOA_IDR;
			for (i = 0; i < 3000000; i++) {	/* Wait a bit. */
				__asm__("nop");
			}
		}
		j++;
		lcd_draw_image(dd[j/5%4]);


//		lcd_address(j/128%8+1,j%128+1);
//		transfer_data(0xFF);
//
//		if(j>57 && j<70)
//			lcd_draw_point(j,rnd%64 -13);
//		lcd_draw_point(j,rnd%64);
//
		for (i = 0; i < 3000000; i++) {		/* Wait a bit. */
			__asm__("nop");
		}

//		j++;
	}

	return 0;
}
