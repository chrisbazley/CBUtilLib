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

/*
Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 30-Nov-14: Created this header file.
  CJB: 04-Dec-15: Retitled this file
*/

#ifndef FileRWInt_h
#define FileRWInt_h

/* ISO library headers */
#include <stdio.h>
#include <stdbool.h>

bool fread_int32le(long int *num, FILE *in);
/*
 * Reads a 32-bit signed integer from a file using little-endian byte order.
 * Returns: false on failure.
 */

bool fwrite_int32le(long int num, FILE *out);
/*
 * Writes a 32-bit signed integer to a file using little-endian byte order.
 * Returns: false on failure.
 */

#endif
