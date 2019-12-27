/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

#include "flash.h"

#define CHECK(x)                     if((x)) { goto err; }
#define LOCK(x)                      if(((x)->lock) && ((x)->lock())) { goto err; }
#define UNLOCK(x)                    if((x)->unlock) { (x)->unlock(); }

extern void flash_structure_init(void);

/**
 * absolute address to block sequence
 * @param hdl: the flash handle
 * @param addr: absolute address
 * @return: the block sequence where the @addr lays in
 */
static uint32_t absaddr2blkseq(flashhdl_t *hdl, uint32_t addr)
{
    uint32_t i;

    if(hdl->blkuneq_flag == flash_true)
    {
        for(i = 1; i < hdl->totblk; i++)
        {
            if(addr < hdl->blktbl[i].startaddr)
            {
                break;
            }
        }
        return (i - 1);
    }

    return ((addr - hdl->startaddr) / (hdl->blksize));
}

/**
 * block sequence to absolute address
 * @param hdl: the flash handle
 * @param seq: the block sequence
 * @return: the absolute address of @seq block
 */
static uint32_t blkseq2absaddr(flashhdl_t *hdl, uint32_t seq)
{
    if(hdl->blkuneq_flag == flash_true)
    {
        return hdl->blktbl[seq].startaddr;
    }

    return (seq * hdl->blksize + hdl->startaddr);
}

/**
 * get block inner offset address
 * @param hdl: the flash handle
 * @param addr: absolute address
 * @return: the relative address in the block where @addr lays in
 */
static uint32_t get_blk_inner_ofs(flashhdl_t *hdl, uint32_t addr)
{
    return (addr - blkseq2absaddr(hdl, absaddr2blkseq(hdl, addr)));
}

/**
 * get block size(in bytes)
 * @param hdl: the flash handle
 * @param blksq: block sequence
 * @return: the @blksq block size
 */
static uint32_t get_blksz(flashhdl_t *hdl, uint32_t blksq)
{
    if(hdl->blkuneq_flag == flash_true)
    {
        return (hdl->blktbl[blksq].size);
    }

    return hdl->blksize;
}

/**
 * flash initialization
 * @param hdl: the flash handle
 * @return: the operation result, see @flashres_t
 */
flashres_t flash_init(flashhdl_t *hdl)
{
    uint32_t i, sum;

    flash_structure_init();

    if(!hdl || !hdl->totsize || !hdl->totblk)
    {
        return flashres_err;
    }

    if(hdl->blkuneq_flag == flash_true)
    {
        if(!hdl->blktbl)
        {
            return flashres_err;
        }

        hdl->blksize = 0;   // doesn't need it anymore
        for(sum = 0, i = 0; i < hdl->totblk; i++)
        {
            sum += get_blksz(hdl, i);
        }
        if(sum != hdl->totsize)
        {
            return flashres_err;
        }

        for(i = 0; i < hdl->totblk - 1; i++)
        {
            if((blkseq2absaddr(hdl, i) + get_blksz(hdl, i)) != blkseq2absaddr(hdl, i + 1))
            {
                return flashres_err;
            }
        }
    }
    else
    {
        if(!hdl->blksize || (hdl->totblk * hdl->blksize != hdl->totsize))
        {
            return flashres_err;
        }
    }
    hdl->endaddr = hdl->startaddr + hdl->totsize - 1;

    if(!hdl->init || !hdl->read)
    {
        return flashres_err;
    }
    if(!hdl->write)
    {
        hdl->writable = flash_false;
        hdl->erase = 0;
    }
    else
    {
        hdl->writable = flash_true;
        if(hdl->erase && !hdl->tmpbuf)
        {
            return flashres_err;
        }
    }

    return hdl->init();
}

/**
 * flash write
 * @param hdl: the flash handle
 * @param waddr: the write address
 * @param wbuf: the write buffer
 * @param wbytes: how many bytes to write
 * @return: the operation result, see @flashres_t
 */
flashres_t flash_write(flashhdl_t *hdl, uint32_t waddr, const void *wbuf, uint16_t wbytes)
{
    uint32_t i;
    uint32_t blkseq;       // block sequence. [0, hdl->totblk - 1]
    uint32_t blkofs;       // block offset. how many bytes before @waddr in a block
    uint32_t blkremain;    // block remain. how many bytes after @waddr in a block. @blkremain
                           // + @blkofs = hdl->blksize
    flashres_t res;
    const uint8_t *_wbuf = wbuf;
    uint8_t *buf = hdl->tmpbuf;

    if(!hdl || waddr < hdl->startaddr || (waddr + wbytes > hdl->endaddr + 1) || !_wbuf)
    {
        return flashres_err;
    }
    if(!wbytes)
    {
        return flashres_ok;
    }

    LOCK(hdl);

    blkseq = absaddr2blkseq(hdl, waddr);
    blkofs = get_blk_inner_ofs(hdl, waddr);
    blkremain = get_blksz(hdl, blkseq) - blkofs;
    if(wbytes < blkremain)
    {
        blkremain = wbytes;
    }

    while(1)
    {
        // read whole @blkseq block
        res = hdl->read(blkseq2absaddr(hdl, blkseq), hdl->tmpbuf, get_blksz(hdl, blkseq));
        CHECK(res);

        for(i = 0; i < blkremain; i++)
        {
            if(buf[blkofs + i] != 0xFF)
            {
                break;
            }
        }
        if(i < blkremain)    // if the @blkseq block should be erased
        {
            res = hdl->erase(blkseq2absaddr(hdl, blkseq));  // TODO  the function @blkseq2absaddr can be refine
            CHECK(res);
            for(i = 0; i < blkremain; i++)
            {
                buf[i + blkofs] = _wbuf[i];
            }
            // cover the @tmpbuf to flash @blkseq block
            res = hdl->write(blkseq2absaddr(hdl, blkseq), hdl->tmpbuf, get_blksz(hdl, blkseq));
            CHECK(res);
        }
        else
        {
            res = hdl->write(waddr, _wbuf, blkremain);
            CHECK(res);
        }

        if(wbytes == blkremain)     // if write over
        {
            break;
        }
        else
        {
            blkseq++;
            blkofs = 0;
            _wbuf += blkremain;
            waddr += blkremain;
            wbytes -= blkremain;
            if(wbytes > get_blksz(hdl, blkseq))
            {
                blkremain = get_blksz(hdl, blkseq);
            }
            else
            {
                blkremain = wbytes;
            }
        }
    }

    UNLOCK(hdl);
    return flashres_ok;
err:
    UNLOCK(hdl);
    return flashres_err;
}

/**
 * flash read
 * @param hdl: the flash handle
 * @param waddr: the read address
 * @param wbuf: the read buffer
 * @param wbytes: how many bytes to read
 * @return: the operation result, see @flashres_t
 */
flashres_t flash_read(flashhdl_t *hdl, uint32_t raddr, void *rbuf, uint16_t rbytes)
{
    flashres_t res = flashres_err;

    if(!hdl || raddr < hdl->startaddr || raddr > hdl->endaddr || !rbuf)
    {
        return flashres_err;
    }
    if(!rbytes)
    {
        return flashres_ok;
    }

    LOCK(hdl);

    res = hdl->read(raddr, rbuf, rbytes);

err:
    UNLOCK(hdl);
    return res;
}
