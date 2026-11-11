/*!
    \file    touch_panel.c 
    \brief   LCD touch panel functions

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

#include "touch_panel.h"
#include "math.h"
#include <stdlib.h>

/* number of filter reads */
#define FILTER_READ_TIMES           5
/* lost value of filter */
#define FILTER_LOST_VAL             1
/* error range of AD sample value */  
#define AD_ERR_RANGE                6

int16_t touch_ad_x=0,touch_ad_y=0;

volatile uint16_t touch_debug_ad_x = 0U;
volatile uint16_t touch_debug_ad_y = 0U;

void touch_debug_sample(void)
{
    uint16_t sample_x;
    uint16_t sample_y;

    touch_start();
    touch_write(0x00);
    touch_write(CH_X);
    sample_x = touch_read();

    touch_start();
    touch_write(0x00);
    touch_write(CH_Y);
    sample_y = touch_read();

    touch_debug_ad_x = sample_x;
    touch_debug_ad_y = sample_y;
}

/*!
    \brief      set or reset touch screen chip select pin
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void spi_delay(uint16_t i)
{
    __IO uint16_t k;
    for(k=0;k<i;k++);
}

/*!
    \brief      configure touch panel GPIO 
    \param[in]  none 
    \param[out] none
    \retval     none
*/
void touch_panel_gpio_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* GPIO clock enable */
    rcu_periph_clock_enable(SPI_SCK_CLK);
    rcu_periph_clock_enable(SPI_MOSI_CLK);
    rcu_periph_clock_enable(SPI_MISO_CLK);
    rcu_periph_clock_enable(SPI_TOUCH_CS_CLK);
    rcu_periph_clock_enable(TOUCH_PEN_INT_CLK);
    rcu_periph_clock_enable(TOUCH_SPI_CLK);
    
    /* Configure SPI4 pins PF7/PF8/PF9 with alternate function 5. */
    gpio_af_set(GPIOF, GPIO_AF_5, SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE,
                  SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN);

    /* configure chip select(SPI-Touch) pin */
    gpio_mode_set(SPI_TOUCH_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI_TOUCH_CS_PIN);
    gpio_output_options_set(SPI_TOUCH_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_TOUCH_CS_PIN);
    
    /* configure touch pen IRQ pin */
    gpio_mode_set(TOUCH_PEN_INT_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, TOUCH_PEN_INT_PIN);
    
    /* set chip select pin high */
    SPI_TOUCH_CS_HIGH();

    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_64;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(TOUCH_SPI, &spi_init_struct);
    spi_enable(TOUCH_SPI);
}

/*!
    \brief      touch start
    \param[in]  none
    \param[out] none
    \retval     none
*/
void touch_start(void)
{
    SPI_TOUCH_CS_LOW();
}

/*!
    \brief      write data to touch screen
    \param[in]  d: the data to be written
    \param[out] none
    \retval     none
*/
void touch_write(uint8_t d)
{
    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_TBE));
    spi_i2s_data_transmit(TOUCH_SPI, d);
    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_RBNE));
    (void)spi_i2s_data_receive(TOUCH_SPI);
}

/*!
    \brief      read the touch AD value
    \param[in]  None
    \param[out] none
    \retval     the value of touch AD
*/
uint16_t touch_read(void)
{
    uint16_t high_byte;
    uint16_t low_byte;

    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_TBE));
    spi_i2s_data_transmit(TOUCH_SPI, 0U);
    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_RBNE));
    high_byte = spi_i2s_data_receive(TOUCH_SPI);

    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_TBE));
    spi_i2s_data_transmit(TOUCH_SPI, 0U);
    while(RESET == spi_i2s_flag_get(TOUCH_SPI, SPI_FLAG_RBNE));
    low_byte = spi_i2s_data_receive(TOUCH_SPI);

    SPI_TOUCH_CS_HIGH();
    return (uint16_t)(((high_byte << 8) | low_byte) >> 3);
}

/*!
    \brief      read the touch pen interrupt request signal
    \param[in]  none
    \param[out] none
    \retval     the status of touch pen: SET or RESET
      \arg        SET: touch pen is inactive
      \arg        RESET: touch pen is active
*/
FlagStatus touch_pen_irq(void)
{
    return TOUCH_PEN_INT_READ();
}

/*!
    \brief      get the AD sample value of touch location at X coordinate
    \param[in]  none
    \param[out] none
    \retval     channel X+ AD sample value
*/
uint16_t touch_ad_x_get(void)
{
    if(RESET != touch_pen_irq()){
        /* touch pen is inactive */
        return 0;
    }
    touch_start();
    touch_write(0x00);
    touch_write(CH_X);
    return (touch_read());
}

/*!
    \brief      get the AD sample value of touch location at Y coordinate
    \param[in]  none
    \param[out] none
    \retval     channel Y+ AD sample value
*/
uint16_t touch_ad_y_get(void)
{
    if(RESET != touch_pen_irq()){
        /* touch pen is inactive */
        return 0;
    }
    touch_start();
    touch_write(0x00);
    touch_write(CH_Y);
    return (touch_read());
}

/*!
    \brief      get channel X+ AD average sample value
    \param[in]  none
    \param[out] none
    \retval     channel X+ AD average sample value
*/
uint16_t touch_average_ad_x_get(void)
{
    uint8_t i;
    uint16_t temp=0;
    for (i=0;i<8;i++){
        temp += touch_ad_x_get();
        spi_delay(1000);
    }
    temp>>=3;

    touch_debug_ad_x = temp;
    
    return temp;
}

/*!
    \brief      get channel Y+ AD average sample value
    \param[in]  none
    \param[out] none
    \retval     channel Y+ AD average sample value
*/
uint16_t touch_average_ad_y_get(void)
{
    uint8_t i;
    uint16_t temp=0;
    for (i=0;i<8;i++){
        temp += touch_ad_y_get();
        spi_delay(1000);
    }
    temp>>=3;

    touch_debug_ad_y = temp;

    return temp;
}

/*!
    \brief      get X coordinate value of touch point on LCD screen
    \param[in]  adx: channel X+ AD average sample value
    \param[out] none
    \retval     X coordinate value of touch point
*/
uint16_t touch_coordinate_x_get(uint16_t adx)
{
    uint32_t sx;

    if (adx >= TOUCH_X_RAW_LEFT) {
        return 0;
    }
    if (adx <= TOUCH_X_RAW_RIGHT) {
        return LCD_X - 1;
    }

    sx = ((uint32_t)(TOUCH_X_RAW_LEFT - adx) * (LCD_X - 1U)) /
         (TOUCH_X_RAW_LEFT - TOUCH_X_RAW_RIGHT);
    return (uint16_t)sx;
}

/*!
    \brief      get Y coordinate value of touch point on LCD screen
    \param[in]  ady: channel Y+ AD average sample value
    \param[out] none
    \retval     Y coordinate value of touch point
*/
uint16_t touch_coordinate_y_get(uint16_t ady)
{
    uint32_t sy;

    if (ady >= TOUCH_Y_RAW_TOP) {
        return 0;
    }
    if (ady <= TOUCH_Y_RAW_BOTTOM) {
        return LCD_Y - 1;
    }

    sy = ((uint32_t)(TOUCH_Y_RAW_TOP - ady) * (LCD_Y - 1U)) /
         (TOUCH_Y_RAW_TOP - TOUCH_Y_RAW_BOTTOM);
    return (uint16_t)sy;
}

/*!
    \brief      get a value (X or Y) for several times. Order these values, 
                remove the lowest and highest and obtain the average value
    \param[in]  channel_select: select channel X or Y
      \arg        CH_X: channel X
      \arg        CH_Y: channel Y
    \param[out] none
    \retval     a value(X or Y) of touch point
*/
uint16_t touch_data_filter(uint8_t channel_select)
{
    uint16_t i=0, j=0; 
    uint16_t buf[FILTER_READ_TIMES]; 
    uint16_t sum=0; 
    uint16_t temp=0;
    /* read data in FILTER_READ_TIMES times */
    for(i=0; i < FILTER_READ_TIMES; i++){
        if(CH_X == channel_select){
            buf[i] = touch_ad_x_get();
        }else{
            /* CH_Y == channel_select */
            buf[i] = touch_ad_y_get();
        }
    }
    /* sort in ascending sequence */
    for(i = 0; i < FILTER_READ_TIMES - 1; i++){
        for(j = i + 1; j < FILTER_READ_TIMES; j++){
            if(buf[i] > buf[j]){
                temp = buf[i]; 
                buf[i] = buf[j]; 
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for(i = FILTER_LOST_VAL; i < FILTER_READ_TIMES - FILTER_LOST_VAL; i++){
        sum += buf[i];
    }
    temp = sum / (FILTER_READ_TIMES - 2 * FILTER_LOST_VAL);
    
    return temp;
}

/*!
    \brief      get the AD sample value of touch location. 
                get the sample value for several times,order these values,remove the lowest and highest and obtain the average value
    \param[in]  channel_select: select channel X or Y
    \param[out] none
      \arg        ad_x: channel X AD sample value
      \arg        ad_y: channel Y AD sample value
    \retval     ErrStatus: SUCCESS or ERROR
*/
ErrStatus touch_ad_xy_get(int16_t *ad_x, int16_t *ad_y)
{
    uint16_t ad_x1=0, ad_y1=0, ad_x2=0, ad_y2=0; 

    ad_x1 = touch_data_filter(CH_X); 
    ad_y1 = touch_data_filter(CH_Y); 
    ad_x2 = touch_data_filter(CH_X); 
    ad_y2 = touch_data_filter(CH_Y);
    
    if((abs(ad_x1 - ad_x2) > AD_ERR_RANGE) || (abs(ad_y1 - ad_y2) > AD_ERR_RANGE)){
        return ERROR;
    }
    *ad_x = (ad_x1 + ad_x2) / 2; 
    *ad_y = (ad_y1 + ad_y2) / 2;
   
    return SUCCESS;
}

/*!
    \brief      detect the touch event
    \param[in]  none
    \param[out] none
    \retval     ErrStatus: SUCCESS or ERROR
*/
ErrStatus touch_scan(void)
{
    uint8_t invalid_count = 0;
    if (RESET == touch_pen_irq()){
        /* touch pen is active */
        while((SUCCESS != touch_ad_xy_get(&touch_ad_x, &touch_ad_y))&& (invalid_count < 20)){
            invalid_count++; 
        }
        
        if(invalid_count >= 20){ 
            touch_ad_x = -1; 
            touch_ad_y = -1;
            return ERROR;
        }
    }else{ 
        touch_ad_x = -1; 
        touch_ad_y = -1;
        return ERROR;
    }
    return SUCCESS;
}
