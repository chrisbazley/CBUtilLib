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
  CJB: 07-Jun-26: Placate clang-tidy by not printing pointer values
                  after calling free or realloc.
  CJB: 14-Jun-26: Assign a compound literal in stringbuffer_init to
                  guarantee full initialisation of new instances.
  CJB: 21-Sep-26: Declare variables when they are first assigned.
*/

/* ISO library headers */
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Local headers */
#include "Internal/CBUtilMisc.h"
#include "StringBuff.h"

void stringbuffer_init(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Initializing buffer %p\n", (void *)buffer);

  *buffer = (StringBuffer){
    .storage = STRB_INITIALIZER(0),
    .undo_pos = 0,
  };
}

_Optional char *stringbuffer_prepare_append(StringBuffer *const buffer,
                                            size_t *const min_size)
{
  _Optional char *free_ptr = NULL;

  assert(buffer != NULL);
  assert(min_size != NULL);
  DEBUG_VERBOSEF(
    "StringBuff: Preparing to append %zu bytes to buffer %p ('%s')\n",
    *min_size, (void *)buffer, strb_cptr(&buffer->storage));

  /* Restore the saved boundary before reserving space, then split there
     again. This preserves pending truncation undo even if allocation fails.
     Keep the editing position at the logical end until finish_append. */
  size_t const pos = strb_tell(&buffer->storage);
  size_t const size = *min_size ? *min_size : 1;
  strb_setmode(&buffer->storage, strb_overwrite);
  strb_restore(&buffer->storage);
  free_ptr = strb_write(&buffer->storage, size - 1);
  strb_seek(&buffer->storage, pos);
  strb_split(&buffer->storage);
  *min_size = free_ptr ? size : 0;

  DEBUG_VERBOSEF("StringBuff: %zu bytes prepared at %p\n", *min_size,
                 (void *)free_ptr);

  return free_ptr;
}

void stringbuffer_finish_append(StringBuffer *const buffer, size_t const n)
{
  assert(buffer != NULL);
  size_t const pos = strb_tell(&buffer->storage);
  assert(n <= strb_len(&buffer->storage) - pos);
  strb_seek(&buffer->storage, pos + n);
  strb_split(&buffer->storage);
  buffer->undo_pos = pos;
  DEBUGF("StringBuff: Finished appending %zu bytes to buffer %p ('%s')\n", n,
         (void *)buffer, strb_cptr(&buffer->storage));
}

bool stringbuffer_append_separated(StringBuffer *const buffer, const char sep,
                                   const char *const tail)
{
  assert(buffer != NULL);
  assert(sep != '\0');
  assert(tail != NULL);

  bool success = false;
  const size_t tail_len = strlen(tail);
  size_t buff_size = tail_len + 2;
  _Optional char *const s = stringbuffer_prepare_append(buffer, &buff_size);
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
  size_t tail_len;
  bool success = true;

  assert(buffer != NULL);
  DEBUG_VERBOSEF("StringBuff: Appending '%.*s' to buffer %p ('%s')\n",
                 (int)HIGHEST(n, INT_MAX), STRING_OR_NULL(tail), (void *)buffer,
                 strb_cptr(&buffer->storage));

  assert(strb_tell(&buffer->storage) <= strb_len(&buffer->storage));

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
  size_t extra_chars = LOWEST(tail_len, n);
  if (extra_chars > 0)
  {
    /* Allocate space for the number of characters to be appended and a
       null terminator. */
    size_t min_size = extra_chars + 1;
    _Optional char *const free_ptr =
      stringbuffer_prepare_append(buffer, &min_size);
    if (free_ptr != NULL)
    {
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
    buffer->undo_pos = strb_tell(&buffer->storage);
  }

  return success;
}

void stringbuffer_truncate(StringBuffer *const buffer, size_t const len)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Truncating buffer %p ('%s') to %zu bytes\n",
         (void *)buffer, strb_cptr(&buffer->storage), len);

  size_t const pos = strb_tell(&buffer->storage);
  buffer->undo_pos = pos;
  if (len < pos)
  {
    strb_seek(&buffer->storage, len);
    strb_split(&buffer->storage);
  }
}

void stringbuffer_minimize(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Minimizing buffer %p ('%s')\n", (void *)buffer,
         strb_cptr(&buffer->storage));

  /* Minimizing always discards undo, including when allocation fails. */
  size_t const len = strb_tell(&buffer->storage);
  buffer->undo_pos = len;

  if (len == 0)
  {
    strb_destroy(&buffer->storage);
    strb_init(&buffer->storage, 0);
  }
  else
  {
    /* strb_t has no in-place shrink operation. Rebuild with the smallest
       initial allocation its policy permits, preserving the old value if
       allocation fails. Copy the counted bytes, including embedded nuls. */
    strb_t smaller;
    strb_init(&smaller, len);
    _Optional char *const chars = strb_write(&smaller, len);
    if (chars)
    {
      memcpy(&*chars, strb_cptr(&buffer->storage), len);
      strb_destroy(&buffer->storage);
      buffer->storage = smaller;
    }
    else
    {
      strb_destroy(&smaller);
    }
  }
}

void stringbuffer_destroy(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Destroying buffer %p ('%s')\n", (void *)buffer,
         strb_cptr(&buffer->storage));

  strb_destroy(&buffer->storage);
}
