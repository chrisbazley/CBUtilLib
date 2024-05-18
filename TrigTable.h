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

/* TrigTable.h declares several functions that allow programs to precalculate
   trigonometric tables to a specified accuracy, and then use these for fast
   look-up of sine and cosine values.

Dependencies: ANSI C library.
Message tokens: None
History:
  CJB: 10-Jan-07: Created this header file from scratch.
  CJB: 08-Sep-09: Stop hiding a pointer behind the definition of type
                  'TrigTable' and stop using a reserved identifier (containing
                  a double underscore) as the structure tag.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
*/

#ifndef TrigTable_h
#define TrigTable_h

typedef struct TrigTable TrigTable;

TrigTable *TrigTable_make(int multiplier, int quarter_turn);
   /*
    * Creates a trigonometric table by multiplying the sine of different
    * angles by the specified 'multiplier' (the magnitude of which dictates
    * the fractional accuracy of the sine values). The size of the table is
    * dictated by 'quarter_turn', which gives the number of sine values that
    * will be available for angles between 0 and 90 degrees.
    * Returns: On successful completion, pointer to a trigonometric table
    *          structure, otherwise null (eg. when not enough space).
    */

void TrigTable_destroy(TrigTable *table);
   /*
    * Frees memory that was previously allocated for a trigonometric table.
    */

int TrigTable_look_up_cosine(const TrigTable *table, int angle);
   /*
    * Looks up the cosine of a specified angle, in a table of pre-calculated
    * values. The 'angle' value is interpreted according to the 'quarter_turn'
    * value specified when the table was generated (i.e. a full revolution
    * would be 4 * quarter_turn, rather than 360).
    * Returns: The cosine of the angle as an integral value (to convert to a
    *          fractional value, you must divide by the 'multiplier').
    */

int TrigTable_look_up_sine(const TrigTable *table, int angle);
   /*
    * Looks up the cosine of a specified angle, in a table of pre-calculated
    * values. The 'angle' value is interpreted according to the 'quarter_turn'
    * value specified when the table was generated (i.e. a full revolution
    * would be 4 * quarter_turn, rather than 360).
    * Returns: The sine of the angle as an integral value (to convert to a
    *          fractional value, you must divide by the 'multiplier').
    */

#endif
