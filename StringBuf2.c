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
  CJB: 24-Sep-23: Moved undo function to this file.
*/

/* ISO library headers */
#include <stdbool.h>
#include <stddef.h>

/* Local headers */
#include "StringBuff.h"
#include "Internal/CBUtilMisc.h"

void stringbuffer_undo(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Undoing last operation on buffer %p ('%s')\n",
         (void *)buffer, STRING_OR_NULL(buffer->buffer));

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->string_len] == '\0');
  }

  if (buffer->undo_len < buffer->string_len)
  {
    /* Undo append by storing a null terminator at the old place. */
    DEBUGF("StringBuff: Undoing append (truncating to %zu)\n",
           buffer->undo_len);

    buffer->buffer[buffer->undo_len] = '\0';
  }
  else if (buffer->undo_len > buffer->string_len)
  {
    /* Undo truncation by reinstating the character previously overwritten
       by a null terminator. Pre-truncation terminator must be intact. */
    DEBUGF("StringBuff: Undoing truncation (extending to %zu)\n",
           buffer->undo_len);

    assert(buffer->undo_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->undo_len] == '\0');
    buffer->buffer[buffer->string_len] = buffer->undo_char;
  }
  else
  {
    DEBUGF("StringBuff: Nothing to undo (length %zu)\n",
           buffer->undo_len);
  }

  buffer->string_len = buffer->undo_len;

  DEBUGF("StringBuff: buffer now contains '%s'\n",
         STRING_OR_NULL(buffer->buffer));
}
