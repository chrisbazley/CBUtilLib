/*
 * CBUtilLib: Extra string functions not part of the ANSI standard
 * Copyright (C) 2004 Christopher Bazley
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

/* StrExtra.h declares three functions useful for manipulating character arrays.
   Although available on most platforms they are not actually part of the ANSI
   standard.

Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 31-Oct-04: Created this header.
  CJB: 04-Nov-04: Added dependency information.
  CJB: 06-Nov-04: Merged with 'Strnicmp.h' and renamed 'StrCaseIns'.
  CJB: 25-Feb-05: Added declaration of function strdup(). Qualified function
                  declarations as external. Renamed this header file 'StrExtra'.
  CJB: 05-Sep-09: Added declaration of new function strinflate().
  CJB: 26-Oct-18: Added declaration of new function strtail().
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
  CJB: 11-Aug-22: Documented the behaviour of strdup when passed a null pointer.
                  Changed the return type of strinflate from int to size_t.
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
 */

#ifndef StrExtra_h
#define StrExtra_h

/* ISO library headers */
#include <stddef.h> /* (for size_t) */

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

int stricmp(const char * /*s1*/, const char * /*s2*/);
   /*
    * Compares the string pointed to by s1 to the string pointed to by s2.
    * Unlike strcmp this comparison is case-insensitive.
    * Returns: an integer greater than, equal to, or less than zero, depending
    *          whether the string pointed to by s1 is greater than, equal to,
    *          or less than the string pointed to by s2.
    */

int strnicmp(const char * /*s1*/, const char * /*s2*/, size_t /*n*/);
   /*
    * Compares not more than n characters (characters that follow a null
    * character are not compared) from the array pointed to by s1 to the array
    * pointed to by s2. Unlike strncmp this comparison is case-insensitive.
    * Returns: an integer greater than, equal to, or less than zero, depending
    *          whether the string pointed to by s1 is greater than, equal to,
    *          or less than the string pointed to by s2.
    */

_Optional char *strdup(_Optional const char * /*s*/);
   /*
    * Duplicates the string pointed to by s by copying it into a malloc'd block
    * of appropriate size. If the input is a null pointer then the output will
    * also be a null pointer.
    * Returns: a pointer to the new string, or a null pointer if memory
    *          allocation failed. It is the caller's responsibility to free
    *          the block when no longer required.
    */

size_t strinflate(char       */*s1*/,
                  size_t      /*n*/,
                  const char */*s2*/,
                  const char */*srch*/,
                  const char */*rplc*/[]);
   /*
    * Inflates the string pointed to by s2 whilst copying it into the array
    * pointed to by s1. Every character of s2 which matches a character in the
    * string pointed to by srch will be replaced by the string pointed to by
    * the corresponding element of the array pointed to by rplc. If n is zero,
    * nothing is written and s1 may be a null pointer. Otherwise, output
    * characters beyond the n-1st are discarded and a null character is written
    * at the end of the characters actually written into the array.
    * Returns: the number of characters that would have been written had n
    *          been sufficiently large, not counting the terminating null
    *          character.
    */

char *strtail(const char * /*s*/, int /*c*/, size_t /*n*/);
   /*
    * Searches backwards through the string pointed to by s, stopping when
    * it has found n instances of character c (converted to unsigned char)
    * or else when it reaches the start of the string.
    * Returns: a pointer to the character following the last path separator
    *          found, or else the value of path if it was found to contain
    *          fewer than n elements.
    */

#endif
