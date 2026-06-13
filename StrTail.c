/*
 * CBUtilLib: Find a given number of elements at the tail of a string
 * Copyright (C) 2018 Christopher Bazley
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
  CJB: 26-Oct-18: Created this source file based on the pathtail()
                  function in CBlibrary.
  CJB: 11-Dec-20: Removed redundant use of the 'extern' keyword.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 13-Jun-26: Fix the behaviour of strtail with n == 0, which was
                  previously undefined.
*/

/* ISO library headers */
#include <stddef.h>
#include <string.h>

/* Local headers */
#include "Internal/CBUtilMisc.h"
#include "StrExtra.h"

char *strtail(const char *const s, const int c, const size_t n)
{
  assert(s != NULL);

  const char *ptr = s + strlen(s); /* terminator */
  if (n == 0)
  {
    return (char *)ptr;
  }

  size_t count = 0;

  while (ptr > s && count < n)
  {
    ptr--; /* scan string backwards from terminator */
    if (*ptr == (unsigned char)c)
      ++count;
  }
  /* N.B. A cast to eliminate the const qualifier from return pointer
     is legitimate (compare with strrchr() and similar ANSI functions). */
  return (char *)(count >= n ? ptr + 1 : s);
}
