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
#include <unistd.h>

int main(void)
{
    uint8_t *wbuf;
    uint8_t *rbuf;
    uint32_t index, len;
    uint32_t i, j, addr;
    uint32_t totcnt = 20, errcnt = 0;
    flashres_t res;

    wbuf = malloc(ramdisk.totsize);
    rbuf = malloc(ramdisk.totsize);

    printf("start test ...\n");
    res = flash_init(&ramdisk);
    if(res)
    {
        printf("init failed, res=%d\n", res);
    }

    for(i = 0; i < totcnt; i++)
    {
        sleep(1);
        srand(time(NULL));
        for(index = 0, j = 0; j < ramdisk.totsize; j++)
        {
            wbuf[index++] = rand() % 0xFF;
        }
        len = rand() % (ramdisk.totsize);
        addr = rand() % (ramdisk.totsize) + ramdisk.startaddr;

        printf("test %08dth, wbuf=[ ", i);
        for(j = 0; j < len; j++)
        {
            printf("%02X ", wbuf[j]);
        }
        printf("], addr=%d, len=%d\n", addr, len);

        res = flash_write(&ramdisk, addr, wbuf, len);
        if(res)
        {
            printf("write failed, res=%d\n", res);
            goto err;
        }
        res = flash_read(&ramdisk, addr, rbuf, len);
        if(res)
        {
            printf("read failed, res=%d\n", res);
            goto err;
        }

        printf("                 rbuf=[ ");
        for(j = 0; j < len; j++)
        {
            printf("%02X ", rbuf[j]);
        }
        printf("]");

        if(memcmp(wbuf, rbuf, len))
        {
            errcnt++;
            printf(",rbuf != wbuf, errcnt=%d\n", errcnt);
        }
        else
        {
            printf("\n");
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







