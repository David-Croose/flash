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

// user configure here
// total test times
#define TOT_TEST_TS            (1000)

uint64_t get_system_time(void)
{
    struct timeb t;

    ftime(&t);
    return 1000 * t.time + t.millitm;
}

int main(void)
{
    flashhdl_t *flash_hdl = &ramdisk2;
    uint8_t *wbuf;
    uint8_t *rbuf;
    uint32_t index, len;
    uint32_t i = 0, j, addr;
    uint32_t errcnt = 0, totcnt = TOT_TEST_TS;
    flashres_t res;

    printf("start test ...\n");
    res = flash_init(flash_hdl);
    if(res)
    {
        printf("init failed, res=%d\n", res);
        goto err;
    }

    wbuf = malloc(flash_hdl->totsize);
    rbuf = malloc(flash_hdl->totsize);
    if(!wbuf || !rbuf)
    {
        printf("malloc failed!\n");
        goto err;
    }
    memset(wbuf, 0, flash_hdl->totsize);
    memset(rbuf, 0, flash_hdl->totsize);

    for(i = 0; i < totcnt; i++)
    {
        usleep(1 * 1000);
        srand(get_system_time());
        for(index = 0, j = 0; j < flash_hdl->totsize; j++)
        {
            wbuf[index++] = rand() % (0xFF + 1);
        }
        addr = rand() % (flash_hdl->totsize) + flash_hdl->startaddr;
        len = rand() % (flash_hdl->endaddr - addr + 1 + 1);

        printf("test %08dth, wbuf=[ ", i);
        for(j = 0; j < len; j++)
        {
            printf("%02X ", wbuf[j]);
        }
        printf("], addr=%d, len=%d\n", addr, len);

        res = flash_write(flash_hdl, addr, wbuf, len);
        if(res)
        {
            printf("write failed, res=%d\n", res);
            goto err;
        }
        res = flash_read(flash_hdl, addr, rbuf, len);
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
            printf(", rbuf != wbuf, errcnt=%d\n", errcnt);
        }
        else
        {
            printf("\n");
        }
    }

    printf("\ntest done, test_count=%d(total_count=%d), error_count=%d\n", i, totcnt, errcnt);
    printf("done\n");
    free(wbuf);
    free(rbuf);
    return 0;

err:
    printf("\ntest done, test_count=%d(total_count=%d), error_count=%d\n", i, totcnt, errcnt);
    printf("done with error!\n");
    free(wbuf);
    free(rbuf);
    return -1;
}
