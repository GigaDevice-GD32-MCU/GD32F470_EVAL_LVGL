/*!
    \file    touch_panel.h 
    \brief   LCD touch panel driver header file

    \version 2023-10-16, V0.0.0, firmware for GD32F5xx
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef TOUCH_PANEL_H
#define TOUCH_PANEL_H

#include "gd32f4xx.h"

#define TOUCH_X_RAW_LEFT    3700U
#define TOUCH_X_RAW_RIGHT   100U
#define TOUCH_Y_RAW_TOP     3680U
#define TOUCH_Y_RAW_BOTTOM  150U

#define  LCD_X      480
#define  LCD_Y      272

#define CH_X        0x90 
#define CH_Y        0xD0

/* touch panel hardware SPI peripheral (SPI4, pins reused as AF5) */
#define  TOUCH_SPI                 SPI4
#define  TOUCH_SPI_CLK             RCU_SPI4

/* SPI4 SCK pin: PF7 (AF5) */
#define  SPI_SCK_CLK               RCU_GPIOF
#define  SPI_SCK_PIN               GPIO_PIN_7
#define  SPI_SCK_PORT              GPIOF

/* SPI4 MOSI pin: PF9 (AF5) */
#define  SPI_MOSI_CLK              RCU_GPIOF
#define  SPI_MOSI_PIN              GPIO_PIN_9
#define  SPI_MOSI_PORT             GPIOF

/* SPI4 MISO pin: PF8 (AF5) */
#define  SPI_MISO_CLK              RCU_GPIOF
#define  SPI_MISO_PIN              GPIO_PIN_8
#define  SPI_MISO_PORT             GPIOF

/* touch screen chip select pin (software controlled GPIO): PF6 */
#define  SPI_TOUCH_CS_CLK          RCU_GPIOF
#define  SPI_TOUCH_CS_PIN          GPIO_PIN_6
#define  SPI_TOUCH_CS_PORT         GPIOF
#define  SPI_TOUCH_CS_LOW()        gpio_bit_reset(SPI_TOUCH_CS_PORT, SPI_TOUCH_CS_PIN)
#define  SPI_TOUCH_CS_HIGH()       gpio_bit_set(SPI_TOUCH_CS_PORT, SPI_TOUCH_CS_PIN)

/* LCD touch interrupt request pin: PI3 */
#define  TOUCH_PEN_INT_CLK         RCU_GPIOI
#define  TOUCH_PEN_INT_PIN         GPIO_PIN_3
#define  TOUCH_PEN_INT_PORT        GPIOI
#define  TOUCH_PEN_INT_READ()      gpio_input_bit_get(TOUCH_PEN_INT_PORT, TOUCH_PEN_INT_PIN)

/* LCD touch busy pin: PH7 */
#define  TOUCH_BUSY_CLK            RCU_GPIOH
#define  TOUCH_BUSY_PIN            GPIO_PIN_7
#define  TOUCH_BUSY_PORT           GPIOH
#define  TOUCH_BUSY_READ()         gpio_input_bit_get(TOUCH_BUSY_PORT, TOUCH_BUSY_PIN)


/* configure touch panel GPIO */
void touch_panel_gpio_config(void);
/* touch start */
void touch_start(void);
/* write data to touch screen */
void touch_write(uint8_t d);
/* read the touch AD value */
uint16_t touch_read(void);
/* read the touch pen interrupt request signal */
FlagStatus touch_pen_irq(void);
/* get the AD sample value of touch location at X coordinate */
uint16_t touch_ad_x_get(void);
/* get the AD sample value of touch location at Y coordinate */
uint16_t touch_ad_y_get(void);
/* get channel X+ AD average sample value */
uint16_t touch_average_ad_x_get(void);
/* get channel Y+ AD average sample value */
uint16_t touch_average_ad_y_get(void);

/* Current averaged raw ADC values for touch calibration. */
extern volatile uint16_t touch_debug_ad_x;
extern volatile uint16_t touch_debug_ad_y;
void touch_debug_sample(void);

/* get X coordinate value of touch point on LCD screen */
uint16_t touch_coordinate_x_get(uint16_t adx);
/* get Y coordinate value of touch point on LCD screen */
uint16_t touch_coordinate_y_get(uint16_t ady);

/* get a value (X or Y) for several times. Order these values, remove the lowest and highest and obtain the average value */
uint16_t touch_data_filter(uint8_t channel_select);
/* get the AD sample value of touch location */
ErrStatus touch_ad_xy_get(int16_t *ad_x, int16_t *ad_y);
/* detect the touch event */
ErrStatus touch_scan(void);

#endif /* TOUCH_PANEL_H */
