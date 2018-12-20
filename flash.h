/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

#ifndef _FLASH_H_
#define _FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    flash_false = 0,
    flash_true = 1,
} flashbool_t;

typedef enum {
    flashres_ok = 0,
    flashres_busy = 1,
    flashres_err = 2,
} flashres_t;                 // flash result typedef

typedef struct {
    // the flash operation driver
    flashres_t (*init)(void);
    flashres_t (*write)(uint32_t addr, const void *wbuf, uint32_t wbytes);
    flashres_t (*erase)(uint32_t addr);
    flashres_t (*read)(uint32_t addr, void *rbuf, uint32_t rbytes);
    flashres_t (*lock)(void);
    flashres_t (*unlock)(void);

    // the flash characteristic
    flashbool_t writable;     // you don't need to configure this variable, the routin will set it
                              // automatically by reading @write and @erase
    uint32_t totsize;         // total size(in bytes)
    uint32_t totblk;          // total blocks
    uint32_t blksize;         // block size(in bytes)
    uint32_t startaddr;       // start address
    uint32_t endaddr;         // end address

    // other stuff
    void *tmpbuf;             // a buffer stores a block before erasing that block. you should give
                              // this buffer an enough room equals to @blksize, or this routine may
                              // crash
    void *extra;              // you could use this variable for any purpose, the program doesn't
                              // care about it
} flashhdl_t;                 // flash handle typedef

flashres_t flash_init(flashhdl_t *hdl);
flashres_t flash_write(flashhdl_t *hdl, uint32_t waddr, const void *wbuf, uint16_t wbytes);
flashres_t flash_read(flashhdl_t *hdl, uint32_t raddr, void *rbuf, uint16_t rbytes);

#include "port.h"

#ifdef __cplusplus
}
#endif

#endif
