/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

// add your head file here
// #include "user_hardware.h"

#include <stdint.h>
#include "flash.h"

#define USE_DEMO_PORT           (1)         // this is a simple ram disk demo, you
                                            // can make it to be 0 to disable it
#if (USE_DEMO_PORT)

#include <string.h>

#define RAMDISK_TOTSIZE         (50)
#define RAMDISK_TOTBLK          (5)
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

flashhdl_t ramdisk = {
    /* flashres_t (*init)(void);                                              */ init,
    /* flashres_t (*write)(uint32_t addr, const void *wbuf, uint32_t wbytes); */ write,
    /* flashres_t (*erase)(uint32_t addr);                                    */ erase,
    /* flashres_t (*read)(uint32_t addr, void *rbuf, uint32_t rbytes);        */ read,
    /* flashres_t (*lock)(void);                                              */ 0,
    /* flashres_t (*unlock)(void);                                            */ 0,
    /* flashbool_t writable;                                                  */ flash_true,
    /* uint32_t totsize;                                                      */ RAMDISK_TOTSIZE,
    /* uint32_t totblk;                                                       */ RAMDISK_TOTBLK,
    /* uint32_t blksize;                                                      */ RAMDISK_BLKSIZE,
    /* uint32_t startaddr;                                                    */ RAMDISK_STARTADDR,
    /* uint32_t endaddr;                                                      */ 0,
    /* void *tmpbuf;                                                          */ tmpbuf,
    /* void *extra;                                                           */ 0,
};

#endif

// you could add your @flashhdl_t variable definition below

