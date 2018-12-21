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

#define USE_DEMO_PORT_1           (1)         // this is a simple ram disk demo, you
                                              // can make it to be 0 to disable it
#if (USE_DEMO_PORT_1)

#define RAMDISK_TOTSIZE         (30)
#define RAMDISK_TOTBLK          (3)
#define RAMDISK_BLKSIZE         (10)
#define RAMDISK_STARTADDR       (0)

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

    // user code here

}




