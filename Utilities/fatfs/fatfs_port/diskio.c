/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"        /* FatFs lower layer API */
#include "ff.h"
#include "sdcard.h"
#include "gd25qxx.h"
#include "gd32f4xx.h"
#include <stdio.h>
/* 为每个设备定义一个物理编号 */
#define ATA                0     // 预留SD卡使用
#define SPI_FLASH          1     // 外部SPI Flash

#define BLOCKSIZE   512      /* block size in bytes */                                              
#define BUSMODE_4BIT         /* SD 4-bit bus mode, uncommend this macro to choose 1-bit bus mode */
//#define DMA_MODE             /* SD DMA mode, uncommend this macro to choose polling mode */

sd_card_info_struct sd_cardinfo;

/*-----------------------------------------------------------------------*/
/* 获取设备状态                                                          */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
    BYTE pdrv        /* 物理编号 */
)
{
    DSTATUS status = STA_NOINIT;
    DWORD QSPI_ID;
    
    switch (pdrv) {
        case ATA:    /* SD CARD */
            status = RES_OK;
            break;

        case SPI_FLASH:
            QSPI_ID=spi_flash_read_id();
            if(QSPI_ID & 0xC84000){
                status = RES_OK;
            }
            break;
        default:
            status = STA_NOINIT;
    }
    return status;
}

/*-----------------------------------------------------------------------*/
/* 设备初始化                                                            */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
    BYTE pdrv                /* 物理编号 */
)
{
    DSTATUS status = STA_NOINIT;
    uint32_t cardstate = 0;
    
    switch (pdrv) {
        case ATA:               /* SD CARD */
            /* initialize the card */
            status = sd_init();
            if(SD_OK == status){
                status = sd_card_information_get(&sd_cardinfo);
            }else{
                return STA_NOINIT;
            }
            if(SD_OK == status){
                status = sd_card_select_deselect(sd_cardinfo.card_rca);
            }else{
                return STA_NOINIT;
            }
            status = sd_cardstatus_get(&cardstate);
            if(cardstate & 0x02000000){
                /* the card is locked */
                while (1){
                }
            }
            /* configure the bus mode and data transfer mode */
            if(SD_OK == status){
                /* set bus mode */
#ifdef BUSMODE_4BIT
                status = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
#else
                status = sd_bus_mode_config( SDIO_BUSMODE_1BIT );
#endif /* BUSMODE_4BIT */
            }else{
                return STA_NOINIT;
            }
            if(SD_OK == status){
                /* set data transfer mode */
#ifdef DMA_MODE
                status = sd_transfer_mode_config( SD_DMA_MODE );
                /* configure the SDIO NVIC */
                nvic_irq_enable(SDIO_IRQn, 0, 0);
#else
                status = sd_transfer_mode_config( SD_POLLING_MODE );
#endif /* DMA_MODE */
            }else{
                return STA_NOINIT;
            }
            if(SD_OK == status){
                /* initialize success */
                return 0;
            }else{
                return STA_NOINIT;
            }
    
        case SPI_FLASH:         /* SPI Flash */
            /* 初始化SPI Flash */
            spi_flash_init();
            status = RES_OK;
            break;

        default:
            status = STA_NOINIT;
    }
    return status;
}


/*-----------------------------------------------------------------------*/
/* 读扇区：读取扇区内容到指定存储区                                              */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
    BYTE pdrv,        /* 设备物理编号(0..) */
    BYTE *buff,       /* 数据缓存区 */
    DWORD sector,     /* 扇区首地址 */
    UINT count        /* 扇区个数(1..128) */
)
{
    sd_error_enum status = SD_ERROR;
    
    /* check the correctness of the parameters */
    if(NULL == buff){
        return RES_PARERR;
    }
    if(!count){
        return RES_PARERR;
    }

    switch (pdrv) {
        case ATA:    /* SD CARD */
            if(1 == count){
                /* single sector read */
                status = sd_block_read((uint32_t *)(&buff[0]), (uint32_t)(sector<<9), BLOCKSIZE);
            }else{
                /* multiple sectors read */
                status = sd_multiblocks_read((uint32_t *)(&buff[0]), (uint32_t)(sector<<9), BLOCKSIZE, (uint32_t)count);
            }
            if(SD_OK == status){
                return RES_OK;
            }
            return RES_ERROR;
    
        case SPI_FLASH:
            spi_flash_buffer_read(buff, (sector<<12), count<<12);
            return RES_OK;

        default:
            return RES_PARERR;
    }
}

/*-----------------------------------------------------------------------*/
/* 写扇区：见数据写入指定扇区空间上                                      */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0
DRESULT disk_write (
    BYTE pdrv,              /* 设备物理编号(0..) */
    const BYTE *buff,       /* 欲写入数据的缓存区 */
    DWORD sector,           /* 扇区首地址 */
    UINT count              /* 扇区个数(1..128) */
)
{
    sd_error_enum status = SD_ERROR;
    if (!count) {
        return RES_PARERR;        /* Check parameter */
    }

    switch (pdrv) {
        case ATA:    /* SD CARD */
            if(1 == count){
                /* single sector write */
                status = sd_block_write((uint32_t *)buff, sector<<9, BLOCKSIZE);
            }else{
                /* multiple sectors write */
                status = sd_multiblocks_write((uint32_t *)buff, sector<<9, BLOCKSIZE, (uint32_t)count);
            }
            if(SD_OK == status){
                return RES_OK;
            }
            return RES_ERROR;

        case SPI_FLASH:
            spi_flash_sector_erase(sector<<12);
            spi_flash_buffer_write((uint8_t *)buff, (sector<<12), count<<12);
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
#endif


/*-----------------------------------------------------------------------*/
/* 其他控制                                                              */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* 物理编号 */
    BYTE cmd,         /* 控制指令 */
    void *buff        /* 写入或者读取数据地址指针 */
)
{
    uint32_t capacity;
    DRESULT status = RES_PARERR;
    switch (pdrv) {
        case ATA:    /* SD CARD */
            switch (cmd) 
            {
                // Get R/W sector size (WORD) 
                case GET_SECTOR_SIZE :
                    *(WORD * )buff = BLOCKSIZE;
                    break;
                // Get erase block size in unit of sector (DWORD)
                case GET_BLOCK_SIZE :
                    *(DWORD * )buff = 1;//sd_cardinfo.card_blocksize;
                    break;

                case GET_SECTOR_COUNT:
                    capacity = sd_card_capacity_get();
                    *(DWORD * )buff = capacity*1024/sd_cardinfo.card_blocksize;
                    break;
                case CTRL_SYNC :
                    break;
            }
            return RES_OK;
    
        case SPI_FLASH:
            switch (cmd) {
            /* 扇区数量 */
            case GET_SECTOR_COUNT:
                *(DWORD * )buff = 256;
                break;
            /* 扇区大小  */
            case GET_SECTOR_SIZE :
              *(WORD * )buff = 4096;
                break;
            /* 同时擦除扇区个数 */
            case GET_BLOCK_SIZE :
              *(DWORD * )buff = 1;
                break;
            }
            return RES_OK;
    
        default:
            status = RES_PARERR;
    }
    return status;
}

__attribute__((weak)) DWORD get_fattime(void) {
    /* 返回当前时间戳 */
    return    ((DWORD)(2019 - 1980) << 25)    /* Year 2019 */
            | ((DWORD)1 << 21)                /* Month 1 */
            | ((DWORD)1 << 16)                /* Mday 1 */
            | ((DWORD)0 << 11)                /* Hour 0 */
            | ((DWORD)0 << 5)                 /* Min 0 */
            | ((DWORD)0 >> 1);                /* Sec 0 */
}
