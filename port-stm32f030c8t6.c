/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

// add your head file here
#include "stm32f0xx_hal.h"
#include <string.h>
#include "flash.h"

#define KB(x)                        ((x) * 1024)
#define MB(x)                        ((x) * 1024 * 1024)

#define STM32_FLASH_TOTSIZE          (KB(1))
#define STM32_FLASH_BLKSIZE          (KB(1))
#define STM32_FLASH_TOTBLK           (STM32_FLASH_TOTSIZE / STM32_FLASH_BLKSIZE)
#define STM32_FLASH_STARTADDR        (0x0800F000)

flashhdl_t stm32f030c8t6_flash_hdl;
static uint8_t tmpbuf[KB(1)];     // sizeof(tmpbuf) = @the biggest block size

static flashres_t init(void)
{
    return flashres_ok;
}

static flashres_t write(uint32_t addr, const void *wbuf, uint32_t wbytes)
{
    flashres_t res = flashres_ok;
    uint32_t i;
    uint32_t words;

    if ((addr & 0x3) || (wbytes % 4)) {
        return flashres_err;
    }

    HAL_FLASH_Unlock();
    for (words = wbytes / 4, i = 0; i < words; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i*4, ((uint32_t *)wbuf)[i])) {
            res = flashres_err;
            break;
        }

    }
    HAL_FLASH_Lock();
    return res;
}

static flashres_t erase(uint32_t addr)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t err;
    flashres_t res;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = 1;

    HAL_FLASH_Unlock();
    res = HAL_FLASHEx_Erase(&EraseInitStruct, &err) == HAL_OK ? flashres_ok : flashres_err;
    HAL_FLASH_Lock();
    return res;
}

static flashres_t read(uint32_t addr, void *rbuf, uint32_t rbytes)
{
    memcpy(rbuf, (void *)addr, rbytes);
    return flashres_ok;
}

/**
 * the user flashhdl_t structure initialization function
 */
void flash_structure_init(void)
{
    flashhdl_t *hdl = &stm32f030c8t6_flash_hdl;

    memset(hdl, 0, sizeof(flashhdl_t));
    hdl->init = init;
    hdl->write = write;
    hdl->read = read;
    hdl->erase = erase;
    hdl->writable = flash_true;
    hdl->totsize = STM32_FLASH_TOTSIZE;
    hdl->totblk = STM32_FLASH_TOTBLK;
    hdl->blksize = STM32_FLASH_BLKSIZE;
    hdl->startaddr = STM32_FLASH_STARTADDR;
    hdl->tmpbuf = tmpbuf;
}
