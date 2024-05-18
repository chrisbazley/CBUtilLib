/*
 * CBUtilLib: Look up fixed point (co)sine values in pre-calculated tables
 * Copyright (C) 2007 Christopher Bazley
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
  CJB: 10-Jan-07: Created this source file (for SFeditor).
       15-Jun-09: Got rid of 'lower precision in wider context' warnings by
                  casting type of certain sub-expressions to 'int'.
  CJB: 08-Sep-09: Stop relying on the definition of type 'TrigTable' hiding a
                  pointer and stop using the reserved identifier
                  'TrigTable__str' as a structure tag and type name. Removed
                  signature from head of structure and associated run-time
                  checks.
  CJB: 13-Oct-09: Deleted superfluous inclusion of "msgtrans.h".
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 18-Apr-16: Cast pointer parameters to void * to match %p.
  CJB: 06-Jun-20: Minor refactoring.
  CJB: 11-Aug-22: Assert valid parameter values and make sign conversion
                  explicit in TrigTable_make.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
 */

/* ISO library headers */
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/* Local headers */
#include "TrigTable.h"
#include "Internal/CBUtilMisc.h"

struct TrigTable
{
  int multiplier;
  int quarter_turn;
  int sine_values[];
};

static double to_deg(TrigTable const *const table, int const angle)
{
  assert(table != NULL);
  return ((double)angle * 360) / (int)(table->quarter_turn * 4);
}

static int look_up_sine(const TrigTable *const table, int angle)
{
  /* Looks up the sine of an angle from a table of pre-calculated values. */
  assert(table != NULL);

  if (abs(angle) >= table->quarter_turn * 4)
  {
    angle = angle % (table->quarter_turn * 4);
  }

  if (angle < 0)
  {
    angle += table->quarter_turn * 4;
  }

  bool neg = false;
  if (angle >= table->quarter_turn * 2)
  {
    /* 2nd half of sine curve (PI to 2 PI, in radians) is a mirror image of
       the curve for 0 to PI, but negative. */
    angle -= table->quarter_turn * 2;
    neg = true;
  }

  if (angle > table->quarter_turn)
  {
    /* 2nd quarter of sine curve (PI/2 to PI) is a mirror image of 0 to PI/2 */
    angle = table->quarter_turn - angle + table->quarter_turn;
  }

  assert(angle >= 0 && angle <= table->quarter_turn);
  return neg ? -table->sine_values[angle] : table->sine_values[angle];
}

/* ----------------------------------------------------------------------- */
/*                         Public functions                                */

TrigTable *TrigTable_make(int const multiplier, int const quarter_turn)
{
  DEBUGF("Generating sine look-up table of size %d with scaler %d\n",
        quarter_turn + 1, multiplier);
  assert(multiplier > 0);
  assert(quarter_turn > 0);

  /* Allocate memory for a table of sine values */
  TrigTable *const table = malloc(offsetof(TrigTable, sine_values) +
                 ((unsigned)quarter_turn + 1) * sizeof(table->sine_values[0]));
  if (table == NULL)
    return NULL;

  /* Initialise table header */
  table->quarter_turn = quarter_turn;
  table->multiplier = multiplier;

  /* Generate a table of pre-calculated sine values */
  for (int index = 0; index <= quarter_turn; index++)
  {
    double radians = (index * 2.0 * PI) / (int)(quarter_turn * 4);
    table->sine_values[index] = (int)floor(sin(radians) * (double)multiplier +
                                           0.5);
  }
  return table;
}

/* ----------------------------------------------------------------------- */

void TrigTable_destroy(TrigTable *const table)
{
  DEBUGF("Destroying trig. table at %p\n", (void *)table);
  free(table);
}

/* ----------------------------------------------------------------------- */

int TrigTable_look_up_cosine(const TrigTable *const table, int const angle)
{
  int const cosine = look_up_sine(table, angle + table->quarter_turn);

  DEBUGF("Cosine of angle %d (%f°) in trig. table at %p is %d (%f)\n",
         angle, to_deg(table, angle), (void *)table,
         cosine, (double)cosine / table->multiplier);

  return cosine;
}

/* ----------------------------------------------------------------------- */

int TrigTable_look_up_sine(const TrigTable *const table, int angle)
{
  int const sine = look_up_sine(table, angle);

  DEBUGF("Sine of angle %d (%f°) in trig. table at %p is %d (%f)\n",
         angle, to_deg(table, angle), (void *)table,
         sine, (double)sine / table->multiplier);

  return sine;
}
