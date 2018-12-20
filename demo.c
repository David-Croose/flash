/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1
                 
            this file is to demonstrate how to use the flash component

*******************************************************************************/

#include "flash.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
#define CHECK(x)    if((x)) { printf("err in %s:%d\n", __FILE__, __LINE__); goto err; }

    uint8_t *wbuf;
    uint8_t *rbuf;
    uint32_t index, len;
    uint32_t i, j, addr;
    uint32_t totcnt = 1000, errcnt = 0;
    flashres_t res;

    wbuf = malloc(ramdisk.totsize);
    rbuf = malloc(ramdisk.totsize);

    printf("start test\n");
    res = flash_init(&ramdisk);
    CHECK(res);

    for(i = 0; i < totcnt; i++)
    {
        srand(time(NULL));
        for(index = 0, j = 0; j < ramdisk.totsize; j++)
        {
            wbuf[index++] = rand() % 0xFF;
        }
        len = rand() % (ramdisk.totsize);
        addr = rand() % (ramdisk.totsize) + ramdisk.startaddr;

        //=================================================
        /// wbuf[0] = 0xAA;
        /// wbuf[1] = 0xBB;
        /// wbuf[2] = 0xCC;
        /// wbuf[3] = 0xDD;
        /// wbuf[4] = 0xEE;
        /// len = 5;
        /// addr = 0;
        //-------------------------------------------------

        res = flash_write(&ramdisk, addr, wbuf, len);
        CHECK(res);
        res = flash_read(&ramdisk, addr, rbuf, len);
        CHECK(res);

        if(memcmp(wbuf, rbuf, len))
        {
            errcnt++;
        }
    }

    printf("total test count=%d, error count=%d\n", totcnt, errcnt);
    printf("done\n");
    return 0;

err:
    free(wbuf);
    free(rbuf);
    printf("done\n");
    return -1;
}







