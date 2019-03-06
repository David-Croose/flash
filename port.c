/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

  this file contains two demo ports: USE_DEMO_PORT_1 and USE_DEMO_PORT_2, both
  of them uses a RAM room as their memory, but they work like a real flash
  chip, in other words, user must perform an erase operation to let bit 0
  turns to be 1, write it direct will fail. what's the differences between them
  is the former's all block size are the same while the latter not

*******************************************************************************/

// add your head file here
// #include "user_hardware.h"

#include <stdint.h>
#include <string.h>
#include "flash.h"

// user configure here
#define USE_DEMO_PORT_1           (1)
#define USE_DEMO_PORT_2           (0)

#if (USE_DEMO_PORT_1)

#define RAMDISK_TOTSIZE         (10 * 1024)
#define RAMDISK_TOTBLK          ((RAMDISK_TOTSIZE) / (RAMDISK_BLKSIZE))
#define RAMDISK_BLKSIZE         (2 * 1024)
#define RAMDISK_STARTADDR       (0x8000000)

flashhdl_t ramdisk;
static uint8_t rambuf[RAMDISK_TOTSIZE];
static uint8_t tmpbuf[RAMDISK_BLKSIZE];

static flashres_t init(void)
{
    return flashres_ok;
}

static flashres_t write(uint32_t addr, const void *wbuf, uint32_t wbytes)
{
    uint32_t i, j;
    uint8_t *p;
    const uint8_t *_wbuf = wbuf;
    uint8_t mask;

    for(i = 0, p = &rambuf[addr - RAMDISK_STARTADDR];
        i < wbytes;
        i++, p++, _wbuf++)
    {
        for(mask = 1, j = 0; j < 8; j++, mask <<= 1)
        {
            if((*_wbuf) & mask)
            {
                if(!((*p) & mask))
                {
                    // if the flash bit is 0, we can not write it to 1
                    return flashres_err;
                }

                (*p) |= mask;
            }
            else
            {
                (*p) &= ~mask;
            }
        }
    }

    return flashres_ok;
}

static flashres_t erase(uint32_t addr)
{
    memset(&rambuf[addr - RAMDISK_STARTADDR], 0xFF, RAMDISK_BLKSIZE);
    return flashres_ok;
}

static flashres_t read(uint32_t addr, void *rbuf, uint32_t rbytes)
{
    memcpy(rbuf, &rambuf[addr - RAMDISK_STARTADDR], rbytes);
    return flashres_ok;
}

#endif

#if (USE_DEMO_PORT_2)

#define RAMDISK2_TOTSIZE         (300)
#define RAMDISK2_TOTBLK          (sizeof(blktbl) / sizeof(blktbl[0]))
#define RAMDISK2_STARTADDR       (0x800)

flashhdl_t ramdisk2;
static uint8_t rambuf2[RAMDISK2_TOTSIZE];
static uint8_t tmpbuf2[80];     // sizeof(tmpbuf2) = @the biggest block size
static blktbl_t blktbl[] = {
        {RAMDISK2_STARTADDR,       80},
        {RAMDISK2_STARTADDR + 80,  60},
        {RAMDISK2_STARTADDR + 140, 60},
        {RAMDISK2_STARTADDR + 200, 50},
        {RAMDISK2_STARTADDR + 250, 50},
};

static flashres_t init2(void)
{
    return flashres_ok;
}

static flashres_t write2(uint32_t addr, const void *wbuf, uint32_t wbytes)
{
    uint32_t i, j;
    uint8_t *p;
    const uint8_t *_wbuf = wbuf;
    uint8_t mask;

    for(i = 0, p = &rambuf2[addr - RAMDISK2_STARTADDR];
        i < wbytes;
        i++, p++, _wbuf++)
    {
        for(mask = 1, j = 0; j < 8; j++, mask <<= 1)
        {
            if((*_wbuf) & mask)
            {
                if(!((*p) & mask))
                {
                    // if the flash bit is 0, we can not write it to 1
                    return flashres_err;
                }

                (*p) |= mask;
            }
            else
            {
                (*p) &= ~mask;
            }
        }
    }

    return flashres_ok;
}

static flashres_t erase2(uint32_t addr)
{
    uint32_t i;

    for(i = 0; i < RAMDISK2_TOTBLK; i++)
    {
        if(addr == blktbl[i].startaddr)
        {
            break;
        }
    }
    if(i >= RAMDISK2_TOTBLK)
    {
        return flashres_err;
    }
    memset(&rambuf2[blktbl[i].startaddr - RAMDISK2_STARTADDR], 0xFF, blktbl[i].size);
    return flashres_ok;
}

static flashres_t read2(uint32_t addr, void *rbuf, uint32_t rbytes)
{
    memcpy(rbuf, &rambuf2[addr - RAMDISK2_STARTADDR], rbytes);
    return flashres_ok;
}

#endif

/**
 * the user flashhdl_t structure initialization function
 */
void flash_structure_init(void)
{
#if (USE_DEMO_PORT_1)
    memset(&ramdisk, 0, sizeof(ramdisk));
    ramdisk.init = init;
    ramdisk.write = write;
    ramdisk.read = read;
    ramdisk.erase = erase;
    ramdisk.writable = flash_true;
    ramdisk.totsize = RAMDISK_TOTSIZE;
    ramdisk.totblk = RAMDISK_TOTBLK;
    ramdisk.blksize = RAMDISK_BLKSIZE;
    ramdisk.startaddr = RAMDISK_STARTADDR;
    ramdisk.tmpbuf = tmpbuf;
#endif

#if (USE_DEMO_PORT_2)
    memset(&ramdisk2, 0, sizeof(ramdisk2));
    ramdisk2.init = init2;
    ramdisk2.write = write2;
    ramdisk2.read = read2;
    ramdisk2.erase = erase2;
    ramdisk2.writable = flash_true;
    ramdisk2.totsize = RAMDISK2_TOTSIZE;
    ramdisk2.totblk = RAMDISK2_TOTBLK;
    ramdisk2.blksize = 0;
    ramdisk2.startaddr = RAMDISK2_STARTADDR;
    ramdisk2.tmpbuf = tmpbuf2;
    ramdisk2.blkuneq_flag = flash_true;
    ramdisk2.blktbl = blktbl;
#endif

    // user code here

}
