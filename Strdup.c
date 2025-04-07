/*
 * CBUtilLib: Duplicate a string in a block of malloc'd memory
 * Copyright (c) 1995-1999 UnixLib contributors (Simon Callan, Nick Burrett,
 *                         Nicholas Clark and Peter Burwood)
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
  CJB: 31-Oct-04: Copied function strdup() from source of UnixLib 3.8.
  CJB: 11-Aug-22: Rewrote strdup to avoid an implicit conversion to unsigned,
                  minimize variable scope and mutability.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
 */

/* ISO library headers */
#include <stdlib.h>
#include <string.h>

/* Local headers */
#include "StrExtra.h"
#include "Internal/CBUtilMisc.h"

/* ----------------------------------------------------------------------- */
/*                         Public functions                                */

_Optional char *strdup(_Optional const char *const string)
{
  if (!string)
  {
    return NULL;
  }

  size_t const len = strlen(&*string) + 1;
  _Optional char *const newstr = malloc(len);
  if (newstr == NULL)
  {
    return NULL;
  }
  return memcpy(&*newstr, &*string, len);
}
