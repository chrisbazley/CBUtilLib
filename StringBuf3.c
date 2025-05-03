/*
 * CBUtilLib: String buffer
 * Copyright (C) 2012 Christopher Bazley
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
  CJB: 24-Sep-23: New function to append a formatted string.
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
*/

/* ISO library headers */
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

/* Local headers */
#include "StringBuff.h"
#include "Internal/CBUtilMisc.h"

bool stringbuffer_vprintf(StringBuffer *const buffer,
  const char *const format, va_list args)
{
  bool success = true;

  assert(buffer != NULL);
  assert(format != NULL);
  DEBUG_VERBOSEF("StringBuff: Appending string formatted according to '%s' to buffer %p ('%s')\n",
                 format, (void *)buffer, STRING_OR_NULL(buffer->buffer));

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
  }

  /* Find the length of the tail string (if any was provided). */
  va_list args_copy;
  va_copy(args_copy, args);

  int const extra_chars = vsnprintf(&(char){'\0'}, 0, format, args);
  if (extra_chars < 0)
  {
    return false;
  }

  /* Allocate space for the number of characters to be appended and a
     null terminator. */
  size_t min_size = (unsigned)extra_chars + 1;
  _Optional char * const free_ptr = stringbuffer_prepare_append(buffer, &min_size);
  if (free_ptr != NULL)
  {
    assert(buffer->buffer != NULL);

    /* Copy characters from the tail string to the end of the existing
       string. */
    int const n = vsnprintf(&*free_ptr, min_size, format, args_copy);
    assert(n == extra_chars);
    NOT_USED(n);

    /* Record the new string length and append a null terminator. */
    stringbuffer_finish_append(buffer, (unsigned)extra_chars);
  }
  else
  {
    success = false;
  }

  va_end(args_copy);
  return success;
}
