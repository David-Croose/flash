/*******************************************************************************

                         an universal flash drive frame
                 for the flashes must erase to let 0 turn to be 1

*******************************************************************************/

#include "flash.h"

#define CHECK(x)                     if((x)) { goto err; }
#define LOCK(x)                      do { if((x)->lock) { (x)->lock(); } } while(0)
#define UNLOCK(x)                    do { if((x)->unlock) { (x)->unlock(); } } while(0)

/**
 * flash initialization
 * @param wbytes: how many bytes to write
 * @return: the operation result, see @flashres_t
 */
flashres_t flash_init(flashhdl_t *hdl)
{
    if(!hdl)
    {
        return flashres_err;
    }
    if(!hdl->totsize || !hdl->totblk || !hdl->blksize
       || (hdl->totblk * hdl->blksize != hdl->totsize))
    {
        return flashres_err;
    }
    hdl->endaddr = hdl->startaddr + hdl->totsize - 1;
    if(!hdl->init || !hdl->read)
    {
        return flashres_err;
    }

    if(!hdl->write)
    {
        hdl->writable = flash_false;
        hdl->erase = NULL;
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
	uint32_t relwaddr;     // relative write address. [0, hdl->totsize)
	uint32_t blkseq;	   // block sequence. [0, hdl->totblk - 1]
	uint32_t blkofs;	   // block offset. how many bytes before @waddr in a block
	uint32_t blkremain;    // block remain. how many bytes after @waddr in a block. @blkremain
                           // + @blkofs = hdl->blksize
    flashres_t res;
    uint8_t *buf = hdl->tmpbuf;
    const uint8_t *_wbuf = wbuf;

	if(!hdl || waddr < hdl->startaddr || waddr > hdl->endaddr || !wbuf)
    {
        return flashres_err;
    }
    if(!wbytes)
    {
        return flashres_ok;
    }

    UNLOCK(hdl);

	relwaddr = waddr - hdl->startaddr;
	blkseq = relwaddr / (hdl->blksize);
	blkofs = relwaddr % (hdl->blksize);
	blkremain = hdl->blksize - blkofs;
	if(wbytes < blkremain)
    {
        blkremain = wbytes;
    }

	while(1)
	{
        // read whole @blkseq block
		res = hdl->read(blkseq * (hdl->blksize) + hdl->startaddr, hdl->tmpbuf, hdl->blksize);
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
			res = hdl->erase(blkseq * (hdl->blksize) + hdl->startaddr);
            CHECK(res);
			for(i = 0; i < blkremain; i++)
			{
                buf[i + blkofs] = _wbuf[i];
			}
            // cover the @tmpbuf to flash @blkseq block
            res = hdl->write(blkseq * (hdl->blksize) + hdl->startaddr, hdl->tmpbuf, hdl->blksize);
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
		   	wbuf += blkremain;
			waddr += blkremain;
		   	wbytes -= blkremain;
			if(wbytes > hdl->blksize)
            {
                blkremain = hdl->blksize;
            }
			else
            {
                blkremain = wbytes;
            }
		}
	}

    LOCK(hdl);
    return flashres_ok;
err:
    LOCK(hdl);
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
	if(!hdl || raddr < hdl->startaddr || raddr > hdl->endaddr || !rbuf)
    {
        return flashres_err;
    }
    if(!rbytes)
    {
        return flashres_ok;
    }

    return hdl->read(raddr, rbuf, rbytes);
}
