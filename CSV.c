/*
 * CBUtilLib: Parse signed decimal values in comma-separated value format
 * Copyright (C) 2006 Christopher Bazley
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
  CJB: 05-Nov-06: Created this source file from code extracted from SFtoSpr.
  CJB: 22-Jun-09: Changes to whitespace only.
  CJB: 30-Sep-09: Titivated formatting and updated to use new enumerated type
                  name and values.
  CJB: 18-Apr-15: Prints records in debug builds. Assertions are now provided
                  by debug.h.
  CJB: 09-Apr-16: Modified format strings to avoid GNU C compiler warnings.
  CJB: 25-Apr-16: Deleted redundant type casts of the output array pointer.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 08-Oct-23: Use strtol and strtod instead of atoi, atof and atod to
                  avoid undefined behaviour if the value is unrepresentable.
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
  CJB: 03-May-25: Fix pedantic warnings when the format specifies type
                  'void *' but the argument has another type.
                  Treat the result of strchr as optional.
*/

/* ISO library headers */
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Local headers */
#include "CSV.h"
#include "Internal/CBUtilMisc.h"

/* -----------------------------------------------------------------------
                         Public library functions
*/

size_t csv_parse_string(const char                *s,
                        _Optional char *_Optional *endp,
                        _Optional void            *output,
                        CSVOutputType              type,
                        size_t                     nmemb)
{
  _Optional const char *end_of_record, *cr, *lf;
  size_t field = 0;

  DEBUGF("CSV: Will parse string from %p, filling %zu members of array %p\n", (void *)s,
        nmemb, output);
  assert(type == CSVOutputType_Int || type == CSVOutputType_Long || type == CSVOutputType_Double);
  assert(s != NULL);

  /* Find the carriage return or linefeed at the end of this record */
  lf = strchr(s, '\n');
  cr = strchr(s, '\r');
  if (lf != NULL && (cr == NULL || lf < cr))
    end_of_record = lf;
  else
    end_of_record = cr;

  if (end_of_record == NULL)
  {
    DEBUGF("CSV: Last record is unterminated\n");
    end_of_record = s + strlen(s);
  }
  DEBUGF("CSV: End of record is character %u at %p\n", *end_of_record,
        (void *)end_of_record);

  DEBUGF("CSV: Record is '%.*s'\n", (int)(end_of_record - s), s);

  /* We handle empty records as a special case because we don't want to
     interpret them as a single field of value 0. */
  if (end_of_record && s < &*end_of_record)
  {
    const char *start_of_field = s;

    /* Read fields from the input string until the end of the current record
       (change the comparison to '<' to ignore trailing commas) */
    while (start_of_field <= &*end_of_record)
    {
      DEBUGF("CSV: Start of field %zu : %p\n", field, (void *)start_of_field);

      /* Find the comma or line ending at the end of this field */
      _Optional const char *next_comma = strchr(start_of_field, ',');
      if (next_comma == NULL || &*end_of_record < &*next_comma)
        next_comma = &*end_of_record;

      DEBUGF("CSV: End of field %zu is character %u at %p\n", field, *next_comma,
            (void *)next_comma);

      if (output != NULL && field < nmemb)
      {
        switch (type)
        {
          case CSVOutputType_Double:
            {
              double *const output_f = &*output;
              output_f[field] = strtod(start_of_field, NULL);
              DEBUGF("CSV: Decoded field %zu as %f\n", field, output_f[field]);
            }
            break;

          case CSVOutputType_Long:
            {
              long *const output_l = &*output;
              output_l[field] = strtol(start_of_field, NULL, 0);
              DEBUGF("CSV: Decoded field %zu as %li\n", field, output_l[field]);
            }
            break;

          case CSVOutputType_Int:
            {
              int *const output_i = &*output;
              long int const tmp = strtol(start_of_field, NULL, 0);
              output_i[field] = (int)LOWEST(INT_MAX, HIGHEST(INT_MIN, tmp));
              DEBUGF("CSV: Decoded field %zu as %i\n", field, output_i[field]);
            }
            break;
        }
      }
      field++;

      start_of_field = &*next_comma + 1;
    } /* next field */

    DEBUGF("CSV: End of record\n");
  }
  else
  {
    DEBUGF("CSV: Empty record\n");
  }

  if (endp == NULL)
    return field; /* Return the number of fields read */

  if (lf && end_of_record == lf)
  {
    /* Check for LF/CR line ending (VDU style). We can't just check the next
       character, in case the following line is blank and the line endings
       are actually CR/LF */
    if (cr == lf + 1)
    {
      DEBUGF("CSV: Line ending is LF,CR\n");
      end_of_record += 2; /* skip over the LF and CR */
    }
    else
    {
      DEBUGF("CSV: Line ending is LF (RISC OS style)\n");
      end_of_record ++; /* skip over the LF */
    }
  }
  else if (cr && end_of_record == cr)
  {
    /* Check for CR/LF line ending (DOS style). We can't just check the next
       character, in case the following line is blank and the line endings
       are actually LF/CR */
    if (lf == cr + 1)
    {
      DEBUGF("CSV: Line ending is CR,LF (DOS style)\n");
      end_of_record += 2; /* skip over the CR and LF */
    }
    else
    {
      DEBUGF("CSV: Line ending is CR (Mac style)\n");
      end_of_record ++; /* skip over the CR */
    }
  }
  else
  {
    DEBUGF("CSV: End of input string\n");
    end_of_record = NULL; /* No more records in the input string */
  }

  /* N.B. I believe that a cast to eliminate the const qualifier is
     legitimate (compare with strrchr() and similar ANSI functions). */
  *endp = (_Optional char *)end_of_record;

  return field; /* Return the number of fields read */
}
