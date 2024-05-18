/*
 * CBUtilLib: Case-insensitive string comparison
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
  CJB: 31-Oct-04: Copied function stricmp() from source of UnixLib 3.8.
  CJB: 03-Oct-20: Added assertions to detect null pointer arguments.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
 */

/* ISO library headers */
#include <ctype.h>

/* Local headers */
#include "StrExtra.h"
#include "Internal/CBUtilMisc.h"

/* ----------------------------------------------------------------------- */
/*                         Public functions                                */

int stricmp(const char *s1, const char *s2)
{
  int i, j;

  assert(s1 != NULL);
  assert(s2 != NULL);

  do
  {
    i = *s1++, j = *s2++;
    i = toupper (i);
    j = toupper (j);
  }
  while (i && i == j);

  return (i - j);
}
