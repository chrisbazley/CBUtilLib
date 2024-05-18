/*
 * CBUtilLib: Functions to read & write integers to/from streams
 * Copyright (C) 2014 Christopher Bazley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* History:
  CJB: 30-Nov-14: Created this source file
  CJB: 01-Jan-15: Apply Fortify to standard library I/O function calls.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 04-Dec-15: Retitled this file
  CJB: 21-Apr-16: Substituted format specifier %zu for %lu to avoid the need
                  to cast the matching parameters.
  CJB: 30-May-16: Cast pointer parameters to void * to match %p.
  CJB: 19-Aug-19: Modified fread_int32le not to rely on undefined
                  behaviour caused by unrepresentable results of
                  left-shifting a signed integer type.
  CJB: 11-Aug-22: Make sign conversion explicit in fread_int32le.
*/

/* ISO library headers */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/* Local headers */
#include "FileRWInt.h"
#include "Internal/CBUtilMisc.h"

/* ----------------------------------------------------------------------- */
/*                         Public functions                                */

bool fread_int32le(long int *num, FILE *in)
{
  unsigned char bytes[4];
  bool success = false;
  size_t n;

  assert(num != NULL);
  assert(in != NULL);

  n = fread(bytes, sizeof(bytes), 1, in);
  if (n != 1) {
    DEBUGF("FileRWInt: fread from %p failed (%zu)\n", (void *)in, n);
  } else {
    /* Assemble a 32 bit integer from 4 bytes, assuming
       little-endian order (least significant byte first) */
    uint32_t const unum = bytes[0] | ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);

    int32_t snum = 0;

    if (unum <= INT32_MAX) {
      snum = (int32_t)unum;
    } else {
      /* Beware that -INT32_MIN may be unrepresentable as int32_t. */
      uint32_t const neg = -unum;
      if (neg <= INT32_MAX) {
        snum = -(int32_t)neg;
      } else {
        snum = INT32_MIN;
      }
    }

    *num = snum;
    success = true;
    DEBUGF("FileRWInt: Read %ld from file %p\n", *num, (void *)in);
  }

  return success;
}

/* ----------------------------------------------------------------------- */

bool fwrite_int32le(long int num, FILE *out)
{
  bool success = false;
  size_t n;

  assert(out != NULL);

  /* Disassemble a 32 bit integer into 4 bytes, using
     little-endian order (least significant byte first) */
  uint32_t const unum = (uint32_t)num;
  unsigned char bytes[4];
  bytes[0] = unum;
  bytes[1] = unum >> 8;
  bytes[2] = unum >> 16;
  bytes[3] = unum >> 24;

  n = fwrite(bytes, sizeof(bytes), 1, out);
  if (n != 1)
  {
    DEBUGF("FileRWInt: fwrite to %p failed (%zu)\n", (void *)out, n);
  }
  else
  {
    success = true;
    DEBUGF("FileRWInt: Wrote %ld to file %p\n", num, (void *)out);
  }

  return success;
}
