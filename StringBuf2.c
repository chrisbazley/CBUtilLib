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
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
*/

/* ISO library headers */
#include <stddef.h>

/* Local headers */
#include "Internal/CBUtilMisc.h"
#include "StringBuff.h"

void stringbuffer_undo(StringBuffer *const buffer)
{
  assert(buffer != NULL);
  DEBUGF("StringBuff: Undoing last operation on buffer %p ('%s')\n",
         (void *)buffer, strb_cptr(&buffer->storage));

  size_t const pos = strb_tell(&buffer->storage);
  if (buffer->undo_pos < pos)
  {
    /* Undo append by splitting at its starting position. */
    DEBUGF("StringBuff: Undoing append (truncating to %zu)\n",
           buffer->undo_pos);

    stringbuffer_truncate(buffer, buffer->undo_pos);
  }
  else if (buffer->undo_pos > pos)
  {
    /* Restore the character overwritten by the truncation's terminator. */
    DEBUGF("StringBuff: Undoing truncation (extending to %zu)\n",
           buffer->undo_pos);

    strb_restore(&buffer->storage);
    strb_seek(&buffer->storage, buffer->undo_pos);
  }
  else
  {
    DEBUGF("StringBuff: Nothing to undo (length %zu)\n", buffer->undo_pos);
  }

  buffer->undo_pos = strb_tell(&buffer->storage);

  DEBUGF("StringBuff: buffer now contains '%s'\n",
         strb_cptr(&buffer->storage));
}
