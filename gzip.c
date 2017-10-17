#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "zlib.h"

int gzip_compress(uint8_t *src, size_t srclen, uint8_t *dest, size_t *destlen,
                  int level) {
  int rc, wbits, memlevel;
  z_stream zstream;

  wbits    = MAX_WBITS;
  memlevel = MAX_MEM_LEVEL - 1;

  while ((ssize_t)srclen < ((1 << (wbits - 1)) - 262)) {
    wbits--;
    memlevel--;
  }

  memset(&zstream, 0, sizeof(z_stream));

  zstream.next_in   = src;
  zstream.avail_in  = srclen;
  zstream.next_out  = dest;
  zstream.avail_out = *destlen;

  rc = deflateInit2(&zstream, level, Z_DEFLATED, wbits + 16, memlevel,
                    Z_DEFAULT_STRATEGY);
  if (rc != Z_OK) {
    return __LINE__;
  }

  rc = deflate(&zstream, Z_FINISH);
  if (rc != Z_STREAM_END) {
    return __LINE__;
  }

  *destlen -= zstream.avail_out;

  rc = deflateEnd(&zstream);
  if (rc != Z_OK) {
    return __LINE__;
  }

  return 0;
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
