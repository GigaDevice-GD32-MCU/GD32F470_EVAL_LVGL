/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>

#include "gd32f4xx.h"
#include "gd32f450i_lcd_eval.h"

/*********************
 *      DEFINES
 *********************/
#define LV_BUF_TYPE    2 /* 1 one buf 2 two buf 3 pingpong*/
#define USE_IPA_INT    1

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

#if defined(__ICCARM__)
#define LV_SDRAM_BUFFER _Pragma("location=\"SDRAM\"") _Pragma("data_alignment=32") __no_init
#define LV_BUFFER_ALIGN
#else
#define LV_SDRAM_BUFFER __attribute__((section("SDRAM")))
#define LV_BUFFER_ALIGN __attribute__((aligned(32)))
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
#if(LV_BUF_TYPE == 1)
/* TLI buf */
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_disp[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];
/* LVGL buf */
#define LV_BUF_SIZE (MY_DISP_HOR_RES * 128 * BYTE_PER_PIXEL)
LV_BUFFER_ALIGN uint8_t buf_2_1[LV_BUF_SIZE] __attribute__((section("ADDSRAM")));
#elif(LV_BUF_TYPE == 2)
/* TLI buf */
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_disp[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];
/* LVGL buf */
#define LV_BUF_SIZE (MY_DISP_HOR_RES * 100 * BYTE_PER_PIXEL)
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_2_1[LV_BUF_SIZE];
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_2_2[LV_BUF_SIZE];
#else
/* LVGL buf  TLI buf */
#define LV_BUF_SIZE (MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL)
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_2_1[LV_BUF_SIZE];
LV_SDRAM_BUFFER LV_BUFFER_ALIGN uint8_t buf_2_2[LV_BUF_SIZE];
#endif

#if(LV_BUF_TYPE == 3) || (USE_IPA_INT == 1)
static lv_display_t * g_disp;
#endif
#if(LV_BUF_TYPE != 3) && (USE_IPA_INT == 1)
static volatile uint8_t g_ipa_flag = 0U;
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

#if(LV_BUF_TYPE == 1)
    /* One buffer for partial rendering, IPA copies it to the TLI buf */
    lv_display_set_buffers(disp, buf_2_1, NULL, LV_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
#elif(LV_BUF_TYPE == 2)
    /* Two buffers for partial rendering, IPA copies them to the TLI buf in the background */
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, LV_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
#else
    /* Two screen sized buffers, the TLI shows them alternately */
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, LV_BUF_SIZE, LV_DISPLAY_RENDER_MODE_FULL);
#endif

#if(LV_BUF_TYPE == 3) || (USE_IPA_INT == 1)
    g_disp = disp;
#endif
#if(LV_BUF_TYPE == 3)
    nvic_irq_enable(TLI_IRQn, 2, 0);
#elif(USE_IPA_INT == 1)
    nvic_irq_enable(IPA_IRQn, 2, 0);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /* ipa init */
    rcu_periph_clock_enable(RCU_IPA);
    /* initilize the LCD and layers */
    lcd_init();
#if(LV_BUF_TYPE == 3)
    lcd_layer_init(LCD_LAYER_BACKGROUND, (uint32_t)buf_2_1, MY_DISP_HOR_RES, MY_DISP_VER_RES);
#else
    lcd_layer_init(LCD_LAYER_BACKGROUND, (uint32_t)buf_disp, MY_DISP_HOR_RES, MY_DISP_VER_RES);
#endif
    lcd_layer_enable(LCD_LAYER_BACKGROUND);
    lcd_layer_set(LCD_LAYER_BACKGROUND);
    lcd_transparency_set(255);
    lcd_clear(LCD_COLOR_BLACK);
    tli_enable();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{
    if(!disp_flush_enabled) {
        lv_display_flush_ready(disp_drv);
        return;
    }

#if(LV_BUF_TYPE == 3)
    LV_UNUSED(area);
    /* TLI buf switch */
    lcd_layer_init(LCD_LAYER_BACKGROUND, (uint32_t)px_map, MY_DISP_HOR_RES, MY_DISP_VER_RES);
    tli_reload_config(TLI_REQUEST_RELOAD_EN);
    /* notice lvgl after TLI complete one frame, see TLI_IRQHandler() */
    tli_line_mark_set(MY_DISP_VER_RES - 1);
    tli_interrupt_enable(TLI_INT_LM);
#else
    ipa_destination_parameter_struct  ipa_destination_init_struct;
    ipa_foreground_parameter_struct   ipa_fg_init_struct;

#if(USE_IPA_INT == 1)
    /* wait until the previous IPA transfer has really finished */
    while(1U == g_ipa_flag) {
    }
#endif

    /* configure IPA pixel format convert mode */
    ipa_pixel_format_convert_mode_set(IPA_FGTODE);
    /* configure destination pixel format */
    ipa_destination_init_struct.destination_pf = IPA_DPF_RGB565;
    /* configure destination memory base address */
    ipa_destination_init_struct.destination_memaddr = (uint32_t)buf_disp + BYTE_PER_PIXEL * (MY_DISP_HOR_RES * area->y1 + area->x1);
    /* configure destination pre-defined alpha value RGB */
    ipa_destination_init_struct.destination_pregreen = 0;
    ipa_destination_init_struct.destination_preblue = 0;
    ipa_destination_init_struct.destination_prered = 0;
    ipa_destination_init_struct.destination_prealpha = 0;
    /* configure destination line offset */
    ipa_destination_init_struct.destination_lineoff = MY_DISP_HOR_RES - lv_area_get_width(area);
    /* configure height of the image to be processed */
    ipa_destination_init_struct.image_height = (uint32_t)lv_area_get_height(area);
    /* configure width of the image to be processed */
    ipa_destination_init_struct.image_width = (uint32_t)lv_area_get_width(area);
    /* IPA destination initialization */
    ipa_destination_init(&ipa_destination_init_struct);

    /* configure IPA foreground */
    ipa_fg_init_struct.foreground_memaddr = (uint32_t)px_map;
    ipa_fg_init_struct.foreground_pf = FOREGROUND_PPF_RGB565;
    ipa_fg_init_struct.foreground_alpha_algorithm = IPA_FG_ALPHA_MODE_0;
    /* configure foreground pre-defined alpha value RGB */
    ipa_fg_init_struct.foreground_prealpha = 0;
    ipa_fg_init_struct.foreground_preblue = 0;
    ipa_fg_init_struct.foreground_pregreen = 0;
    ipa_fg_init_struct.foreground_prered = 0;
    /* configure foreground line offset */
    ipa_fg_init_struct.foreground_lineoff = 0;
    /* foreground initialization */
    ipa_foreground_init(&ipa_fg_init_struct);

#if(USE_IPA_INT == 1)
    /* lv_display_flush_ready() is called in IPA_IRQHandler() */
    g_ipa_flag = 1U;
    ipa_interrupt_enable(IPA_INT_FTF | IPA_INT_TAE | IPA_INT_WCF);
    ipa_transfer_enable();
#else
    ipa_transfer_enable();
    /* wait until transfer is done */
    while(IPA_CTL & IPA_CTL_TEN) {
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
#endif
#endif /* LV_BUF_TYPE == 3 */
}

#if(LV_BUF_TYPE == 3)
/*!
    \brief      this function handles TLI line mark interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TLI_IRQHandler(void)
{
    if(RESET != tli_interrupt_flag_get(TLI_INT_FLAG_LM)) {
        tli_interrupt_disable(TLI_INT_LM);
        tli_interrupt_flag_clear(TLI_INT_FLAG_LM);
        /*IMPORTANT!!!
         *Inform the graphics library that you are ready with the flushing*/
        lv_display_flush_ready(g_disp);
    }
}
#elif(USE_IPA_INT == 1)
/*!
    \brief      this function handles IPA exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void IPA_IRQHandler(void)
{
    if((RESET != ipa_interrupt_flag_get(IPA_INT_FLAG_FTF)) ||
       (RESET != ipa_interrupt_flag_get(IPA_INT_FLAG_TAE)) ||
       (RESET != ipa_interrupt_flag_get(IPA_INT_FLAG_WCF))) {
        ipa_interrupt_disable(IPA_INT_FTF | IPA_INT_TAE | IPA_INT_WCF);
        /* clear the error flags too, otherwise the interrupt re-enters forever */
        ipa_interrupt_flag_clear(IPA_INT_FLAG_FTF | IPA_INT_FLAG_TAE | IPA_INT_FLAG_WCF);

        if(1U == g_ipa_flag) {
            g_ipa_flag = 0U;
            /*IMPORTANT!!!
             *Inform the graphics library that you are ready with the flushing*/
            lv_display_flush_ready(g_disp);
        }
    }
}
#endif

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
