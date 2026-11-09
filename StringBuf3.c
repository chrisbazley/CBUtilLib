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
  CJB: 07-Jun-26: Stop leaking a copy of the va_list if vsnprintf fails.
*/

/* ISO library headers */
#include <stdarg.h>
#include <stdbool.h>

/* Local headers */
#include "Internal/CBUtilMisc.h"
#include "StringBuff.h"

bool stringbuffer_vprintf(StringBuffer *const buffer, const char *const format,
                          va_list args)
{
  bool success = false;

  assert(buffer != NULL);
  assert(format != NULL);
  DEBUG_VERBOSEF("StringBuff: Appending string formatted according to '%s' to "
                 "buffer %p ('%s')\n",
                 format, (void *)buffer, strb_cptr(&buffer->storage));

  size_t const pos = strb_tell(&buffer->storage);
  if (strb_vputf(&buffer->storage, format, args) == 0)
  {
    strb_delto(&buffer->storage, SIZE_MAX);
    int const err = strb_split(&buffer->storage);
    assert(err == 0); /* Formatting already reserved the new end. */
    NOT_USED(err);
    buffer->undo_pos = pos;
    success = true;
  }

  return success;
}
