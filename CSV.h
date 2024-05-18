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

/* CSV.h declares a function for parsing ASCII strings that contain numeric
   values in the industry standard comma-separated value format.

Dependencies: ANSI C library.
Message tokens: None
History:
  CJB: 05-Nov-06: Created this header file from scratch.
  CJB: 05-Oct-09: Renamed the enumerated type 'parse_csv_type' and its values.
  CJB: 26-Jun-10: Made definition of deprecated type and constant names
                  conditional upon definition of CBLIB_OBSOLETE.
  CJB: 25-Jan-15: Documentation correction.
  CJB: 11-Dec-20: Removed redundant use of the 'extern' keyword.
  CJB: 08-Oct-23: Added type-safe veneer functions, e.g. csv_parse_as_int.
                  Use strtol and strtod instead of atoi, atof and atod to
                  avoid undefined behaviour if the value is unrepresentable.
*/

#ifndef CSV_h
#define CSV_h

/* ISO library headers */
#include <stddef.h>

typedef enum
{
  CSVOutputType_Int,
  CSVOutputType_Long,
  CSVOutputType_Double
}
CSVOutputType;

size_t csv_parse_string(const char *s, char **endp, void *output,
  CSVOutputType type, size_t nmemb);
   /*
    * Parses a string 's' that contains comma-separated numeric values and
    * assigns these values to 'nmemb' members of the 'output' array. The 'type'
    * argument specifies the type of the elements of this array. Internally,
    * the ANSI library function 'strtol' or 'strtod' is used to convert
    * the values. The input string will be read as far as the next line-ending;
    * this may be carriage return ('\r'), line feed ('\n'), or combinations
    * thereof. A pointer to the first character of the next line (i.e. the next
    * record) will be stored in '*endp', unless 'endp' is NULL. If the end of
    * the input string is reached then NULL will instead be stored in '*endp'.
    * You can call this function with NULL instead of a pointer to an output
    * array, to find out how many fields a given record has.
    * Returns: The number of values that would have been read into the 'output'
    *          array if it had been specified and 'nmemb' was big enough.
    */

static inline size_t csv_parse_as_int(const char *const s, char **const endp,
                                   int *const output, size_t const nmemb)
{
  return csv_parse_string(s, endp, output, CSVOutputType_Int, nmemb);
}

static inline size_t csv_parse_as_long(const char *const s, char **const endp,
                                       long int *const output, size_t const nmemb)
{
  return csv_parse_string(s, endp, output, CSVOutputType_Long, nmemb);
}

static inline size_t csv_parse_as_double(const char *const s, char **const endp,
                                         double *const output, size_t const nmemb)
{
  return csv_parse_string(s, endp, output, CSVOutputType_Double, nmemb);
}

/* Deprecated type and enumeration constant names */
#define parse_csv_type    CSVOutputType
#define CSV_OUTPUT_TYPE_I CSVOutputType_Int
#define CSV_OUTPUT_TYPE_L CSVOutputType_Long
#define CSV_OUTPUT_TYPE_F CSVOutputType_Double

#endif /* CSV_h */
