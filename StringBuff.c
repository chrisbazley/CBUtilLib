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
  CJB: 04-Apr-12: Created this source file.
  CJB: 26-Nov-14: Truncate and append operations now remove any pending
                  undo even if they have no other effect.
  CJB: 30-Nov-14: Added stringbuffer_prepare_append and
                  stringbuffer_finish_append functions.
  CJB: 07-Dec-14: Fixed debug output from stringbuffer_prepare_append.
                  stringbuffer_append no longer always claims it succeeded
                  (since stringbuffer_prepare_append).
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 21-Apr-16: Cast pointer parameters to void * to match %p.
                  Substituted format specifier %zu for %lu to avoid the need
                  to cast the matching parameters.
  CJB: 07-Aug-18: Added the stringbuffer_append_separated function.
  CJB: 05-Feb-19: Added the stringbuffer_append_all function.
  CJB: 06-Jun-20: Debugging output is less verbose by default.
  CJB: 10-Aug-22: Converted the most trivial functions into inline functions.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 24-Sep-23: Added functions to append a formatted string.
                  Moved undo function to a separate file.
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
  CJB: 03-May-25: Fix pedantic warnings when the format specifies type
                  'void *' but the argument has another type.
                  No longer assume that vsnprintf will accept NULL.
*/

/* ISO library headers */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>

/* Local headers */
#include "StringBuff.h"
#include "Internal/CBUtilMisc.h"

#define GROWTH_FACTOR 2

static bool realloc_buffer(StringBuffer *const buffer,
  size_t const new_size)
{
  _Optional char *new_buffer = NULL;
  bool success = true;

  assert(buffer != NULL);
  if (new_size == 0)
  {
    free(buffer->buffer);
    DEBUGF("StringBuff: freed %p (%zu bytes)\n",
           (void *)buffer->buffer, buffer->buffer_size);
  }
  else
  {
    new_buffer = realloc(buffer->buffer, new_size);
    if (new_buffer != NULL)
    {
      DEBUGF("StringBuff: reallocated %p (%zu bytes) at %p (%zu bytes)\n",
             (void *)buffer->buffer, buffer->buffer_size,
             (void *)new_buffer, new_size);
    }
    else
    {
      DEBUGF("StringBuff: failed to reallocate %p (%zu to %zu bytes)\n",
             (void *)buffer->buffer, buffer->buffer_size,
             new_size);
      success = false;
    }
  }

  if (success)
  {
    buffer->buffer = new_buffer;
    buffer->buffer_size = new_size;
  }

  return success;
}

static bool ensure_size(StringBuffer *const buffer, size_t const min_size)
{
  bool success = true;

  assert(buffer != NULL);
  if (min_size > buffer->buffer_size)
  {
    const size_t new_size = HIGHEST(min_size,
                                    buffer->buffer_size * GROWTH_FACTOR);
    success = realloc_buffer(buffer, new_size);
  }

  return success;
}

static void set_len(StringBuffer *const buffer, size_t const new_len)
{
  assert(buffer != NULL);
  assert(new_len < buffer->buffer_size);

  buffer->undo_len = buffer->string_len;
  buffer->string_len = new_len;

  if (buffer->buffer)
  {
    /* To undo truncation we need to reinstate the character that was
      overwritten by a nul terminator. */
    buffer->undo_char = buffer->buffer[new_len];
    buffer->buffer[new_len] = '\0';
  }
}

void stringbuffer_init(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Initializing buffer %p\n", (void *)buffer);

  buffer->buffer_size = 0;
  buffer->string_len = 0;
  buffer->buffer = NULL;
  buffer->undo_len = 0;
}

_Optional char *stringbuffer_prepare_append(StringBuffer *const buffer,
  size_t *const min_size)
{
  _Optional char *free_ptr = NULL;
  size_t size;

  assert(buffer != NULL);
  assert(min_size != NULL);
  DEBUG_VERBOSEF("StringBuff: Preparing to append %zu bytes to buffer %p ('%s')\n",
         *min_size, (void *)buffer, STRING_OR_NULL(buffer->buffer));

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
  }

  /* Ensure the string buffer is big enough for the existing string and
     the number of bytes to be appended. */
  size = *min_size ? *min_size : 1; /* can't return null and succeed */
  if (ensure_size(buffer, buffer->string_len + size) && buffer->buffer)
  {
    free_ptr = &*buffer->buffer + buffer->string_len;
    *min_size = buffer->buffer_size - buffer->string_len;
  }
  else
  {
    *min_size = 0;
  }

  DEBUG_VERBOSEF("StringBuff: %zu bytes prepared at %p\n",
         *min_size, (void *)free_ptr);

  return free_ptr;
}

void stringbuffer_finish_append(StringBuffer *const buffer, size_t const n)
{
  assert(buffer != NULL);
  set_len(buffer, buffer->string_len + n);
  DEBUGF("StringBuff: Finished appending %zu bytes to buffer %p ('%s')\n",
         n, (void *)buffer, STRING_OR_NULL(buffer->buffer));
}

bool stringbuffer_append_separated(StringBuffer *const buffer,
  const char sep, const char *const tail)
{
  assert(buffer != NULL);
  assert(sep != '\0');
  assert(tail != NULL);

  bool success = false;
  const size_t tail_len = strlen(tail);
  size_t buff_size = tail_len + 2;
  _Optional char * const s = stringbuffer_prepare_append(buffer, &buff_size);
  if (s != NULL)
  {
    *s = sep;
    memcpy(&*s + 1, tail, tail_len);
    stringbuffer_finish_append(buffer, 1 + tail_len);
    success = true;
  }
  return success;
}

bool stringbuffer_append(StringBuffer *const buffer,
  _Optional const char *const tail, size_t const n)
{
  size_t tail_len, extra_chars;
  bool success = true;

  assert(buffer != NULL);
  DEBUG_VERBOSEF("StringBuff: Appending '%.*s' to buffer %p ('%s')\n",
         (int)HIGHEST(n, INT_MAX), STRING_OR_NULL(tail), (void *)buffer,
         STRING_OR_NULL(buffer->buffer));

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
  }

  /* Find the length of the tail string (if any was provided). */
  if (n > 0)
  {
    assert(tail != NULL);
    tail_len = strlen(STRING_OR_NULL(tail));
  }
  else
  {
    tail_len = 0;
  }

  /* Calculate the number of characters to append (can't be more than
     than the tail string length). */
  extra_chars = LOWEST(tail_len, n);
  if (extra_chars > 0)
  {
    /* Allocate space for the number of characters to be appended and a
       null terminator. */
    size_t min_size = extra_chars + 1;
    _Optional char * const free_ptr = stringbuffer_prepare_append(buffer, &min_size);
    if (free_ptr != NULL)
    {
      assert(buffer->buffer != NULL);

      /* Copy characters from the tail string to the end of the existing
         string. */
      memcpy(&*free_ptr, STRING_OR_NULL(tail), extra_chars);

      /* Record the new string length and append a null terminator. */
      stringbuffer_finish_append(buffer, extra_chars);
    }
    else
    {
      success = false;
    }
  }
  else
  {
    /* Remove any previously-pending undo. */
    buffer->undo_len = buffer->string_len;
  }

  return success;
}

void stringbuffer_truncate(StringBuffer *const buffer, size_t const len)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Truncating buffer %p ('%s') to %zu bytes\n",
         (void *)buffer, STRING_OR_NULL(buffer->buffer), len);

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->string_len] == '\0');
  }

  /* Attempting to truncate a string that is already shorter than or equal
     to the specified number of characters has no effect. */
  if (len < buffer->string_len)
  {
    /* Record the new string length and store a null terminator at
       the appropriate place (remembering the character overwritten). */
     set_len(buffer, len);
  }
  else
  {
    /* Remove any previously-pending undo. */
    buffer->undo_len = buffer->string_len;
  }
}

void stringbuffer_minimize(StringBuffer *const buffer)
{
  size_t min_size;

  assert(buffer != NULL);
  DEBUGF("StringBuff: Minimizing buffer %p ('%s')\n", (void *)buffer,
         STRING_OR_NULL(buffer->buffer));

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->string_len] == '\0');
    min_size = buffer->string_len + 1; /* +1 for nul terminator */
  }
  else
  {
    min_size = 0;
  }

  /* Guard against trying to undo truncation after minimizing the buffer
     size. This is deliberately simplistic to make the behaviour more
     predictable. */
  buffer->undo_len = buffer->string_len;

  if (min_size < buffer->buffer_size)
  {
    (void)realloc_buffer(buffer, min_size);
  }
}

void stringbuffer_destroy(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Destroying buffer %p ('%s')\n", (void *)buffer,
         STRING_OR_NULL(buffer->buffer));

  (void)realloc_buffer(buffer, 0);
}
