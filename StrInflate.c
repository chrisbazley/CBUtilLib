/*
 * CBUtilLib: Inflate a string by replacing certain characters with strings
 * Copyright (C) 2009 Christopher Bazley
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
  CJB: 05-Sep-09: Created this source file.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 21-Apr-16: Modified format strings and substituted type size_t for
                  int as the output character count to avoid GNU C compiler
                  warnings.
  CJB: 11-Aug-22: Changed the return type of strinflate from int to size_t.
                  Made conversion of pointer difference from signed to
                  unsigned explicit.
                  Minimized variable scope and mutability.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 03-May-25: Fix pedantic warnings when the format specifies type
                  'void *' but the argument has another type.
                  Treat the result of strchr as optional.
*/

/* ISO library headers */
#include <stddef.h>
#include <string.h>

/* Local headers */
#include "StrExtra.h"
#include "Internal/CBUtilMisc.h"

size_t strinflate(char *const s1, size_t const n, const char *s2,
                  const char *const srch, const char *rplc[])
{
  assert(s2 != NULL);
  assert(s1 != NULL || n == 0);

  size_t count = 0;
  while (*s2 != '\0')
  {
    size_t len = 0;

    /* Find the next character that needs to be inflated */
    DEBUGF("Searching for inflatable characters in '%s'\n", s2);
    const char *const i = strpbrk(s2, srch);
    if (i == NULL)
    {
      /* No more characters need to be inflated, so append the remainder
         of the input string. */
      DEBUGF("No inflatable character found\n");
      len = strlen(s2);
    }
    else
    {
      /* Append the characters leading up to but not including the
         character to be inflated */
      assert(i >= s2);
      len = (size_t)(i - s2);
      DEBUGF("Found inflatable character 0x%x at offset %zu\n", *i, len);
    }

    if (count + len < n)
    {
      DEBUGF("Copying %zu bytes from %p to %p\n", len, (void *)s2, (void *)(s1 + count));
      strncpy(s1 + count, s2, len);
    }
    else
    {
      DEBUGF("Insufficient space in the output buffer (1)\n");
    }
    count += len;

    if (i == NULL)
      break;

    /* Find the index of the inflatable character in the search string */
    _Optional const char *const m = strchr(srch, *i);
    assert(m != NULL);
    assert(m >= srch);
    if (m == NULL)
      break; // Should never happen

    /* Get a pointer to the corresponding replacement string */
    const char *const r = rplc[&*m - srch];

    len = strlen(r);
    if (count + len < n)
    {
      DEBUGF("Appending sequence '%s'\n", r);
      strcpy(s1 + count, r);
    }
    else
    {
      DEBUGF("Insufficient space in the output buffer (2)\n");
    }
    count += len;
    s2 = i + 1; /* skip the character that was inflated */
  }

  if (count < n)
  {
    s1[count] = '\0'; /* append a nul terminator */
    DEBUGF("Inflated string is '%s'\n", s1);
  }

  /* Return the number of characters that would have been written had a
     large enough output buffer been supplied. */
  return count;
}
