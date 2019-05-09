/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

// add your head file here
#include "stm32f4xx_hal.h"
#include "core_cm4.h"

#include <stdint.h>
#include <string.h>
#include "flash.h"

#define KB(x)                        ((x) * 1024)
#define MB(x)                        ((x) * 1024 * 1024)

#define STM32_FLASH_TOTSIZE          (MB(2))
#define STM32_FLASH_TOTBLK           (sizeof(blktbl) / sizeof(blktbl[0]))
#define STM32_FLASH_STARTADDR        (0x8000000)
#define STM32_FLASH_WAIT_TIMEOUT     (50000)

flashhdl_t stm32f429zit6_flash_hdl;
static uint8_t tmpbuf[KB(128)];     // sizeof(tmpbuf2) = @the biggest block size
static blktbl_t blktbl[] = {
    /* bank 1 */
    {STM32_FLASH_STARTADDR,                             KB(16)},
    {STM32_FLASH_STARTADDR + KB(16),                    KB(16)},
    {STM32_FLASH_STARTADDR + KB(16 * 2),                KB(16)},
    {STM32_FLASH_STARTADDR + KB(16 * 3),                KB(16)},
    {STM32_FLASH_STARTADDR + KB(16 * 4),                KB(64)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64),           KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128),     KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128 * 2), KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128 * 3), KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128 * 4), KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128 * 5), KB(128)},
    {STM32_FLASH_STARTADDR + KB(16 * 4 + 64 + 128 * 6), KB(128)},

    /* bank 2 */
    {STM32_FLASH_STARTADDR + MB(1),                             KB(16)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16),                    KB(16)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 2),                KB(16)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 3),                KB(16)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4),                KB(64)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64),           KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128),     KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128 * 2), KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128 * 3), KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128 * 4), KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128 * 5), KB(128)},
    {STM32_FLASH_STARTADDR + MB(1) + KB(16 * 4 + 64 + 128 * 6), KB(128)},
};

static flashres_t init(void)
{
    return flashres_ok;
}

static flashres_t write(uint32_t addr, const void *wbuf, uint32_t wbytes)
{
    uint32_t i;
    uint32_t words = wbytes / 4;
    HAL_StatusTypeDef res;
    const uint32_t *_wbuf = wbuf;

    if(addr % 4 || wbytes % 4)
    {
        return flashres_err;
    }

    HAL_FLASH_Unlock();
    if(FLASH_WaitForLastOperation(STM32_FLASH_WAIT_TIMEOUT) != HAL_OK)
    {
        goto out;
    }
    for(i = 0; i < words; i++)
    {
        if((res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (addr + i * 4),
                                    *(_wbuf + i))) != HAL_OK)
        {
            goto out;
        }
    }

out:
    HAL_FLASH_Lock();
    return res == HAL_OK ? flashres_ok : flashres_err;
}

static flashres_t erase(uint32_t addr)
{
    uint32_t i;
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError;

    for(i = 0; i < STM32_FLASH_TOTBLK; i++)
    {
        if(addr == blktbl[i].startaddr)
        {
            break;
        }
    }
    if(i >= STM32_FLASH_TOTBLK)
    {
        return flashres_err;
    }

    HAL_FLASH_Unlock();

    FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    FlashEraseInit.Sector = i;
    FlashEraseInit.NbSectors = 1;
    FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    if(HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return flashres_err;
    }

    HAL_FLASH_Lock();
    return flashres_ok;
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
    flashhdl_t *hdl = &stm32f429zit6_flash_hdl;

    memset(hdl, 0, sizeof(flashhdl_t));
    hdl->init = init;
    hdl->write = write;
    hdl->read = read;
    hdl->erase = erase;
    hdl->writable = flash_true;
    hdl->totsize = STM32_FLASH_TOTSIZE;
    hdl->totblk = STM32_FLASH_TOTBLK;
    hdl->blksize = 0;
    hdl->startaddr = STM32_FLASH_STARTADDR;
    hdl->tmpbuf = tmpbuf;
    hdl->blkuneq_flag = flash_true;
    hdl->blktbl = blktbl;
}
