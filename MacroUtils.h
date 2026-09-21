/*
 * CBUtilLib: Generally useful macro definitions
 * Copyright (C) 2003 Christopher Bazley
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

/* MacroUtils.h declares macros for common operations that do not depend on
   CBLibrary or RISC OS interfaces.

Dependencies: ISO C library.
Message tokens: None.
History:
  CJB: 20-Feb-04: Ensure strings truncated by STRCPY_SAFE are terminated.
  CJB: 15-May-05: Define SYSTEM_BEEP in terms of putchar.
  CJB: 03-Jul-05: Optimise WORD_ALIGN using bitwise AND.
  CJB: 15-Jul-05: Add ARRAY_SIZE.
  CJB: 30-Sep-09: Parenthesise arguments to LOWEST and HIGHEST.
  CJB: 08-Apr-12: Add STRING_OR_NULL.
  CJB: 18-Apr-15: Add STRINGIFY.
  CJB: 09-Apr-16: Add CHECK_PRINTF and improve WORD_ALIGN bracketing.
  CJB: 31-Oct-18: Consistently bracket macro arguments and results.
  CJB: 21-Sep-19: Add CONTAINER_OF.
  CJB: 25-May-26: Add C23_CONST.
  CJB: 29-May-26: Add CLAMP.
  CJB: 21-Jun-26: Add WORD_ALIGN_SZ.
  CJB: 27-Aug-26: Make CONTAINER_OF diagnose pointer-to-_Optional input.
  CJB: 21-Sep-26: Move generally useful definitions from CBLibrary. Harden
                  CONTAINER_OF with a debug null check and, in C23, exact
                  member pointer type checking including qualifiers.
*/

#ifndef MacroUtils_h
#define MacroUtils_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#include <assert.h>
#endif

/* Copy a string into a character array of known size, truncating it to fit if
 * necessary. Unlike strncpy(), this macro ensures that the copied string is
 * NUL terminated if it has to be truncated.
 */
#define STRCPY_SAFE(string_1, string_2) do { \
  strncpy((string_1), (string_2), sizeof(string_1) - 1); \
  (string_1)[sizeof(string_1) - 1] = '\0'; \
} while (0)

#define SYSTEM_BEEP() putchar('\a')

#define FREE_SAFE(memptr) do { \
  free(memptr); \
  (memptr) = NULL; \
} while (0)

#define WORD_ALIGN(value) (((value) + 3) & ~3)
#define WORD_ALIGN_SZ(value) (((value) + 3u) & ~(size_t)3)
#define NOT_USED(x) ((void)(x))
#define SCALE(value, perc) (((value) * (perc)) / 100)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Swap two lvalues of type int. */
#define SWAP(a, b) do { \
  int temp = (a); \
  (a) = (b); \
  (b) = temp; \
} while (0)

#define LOWEST(a, b) ((a) < (b) ? (a) : (b))
#define HIGHEST(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) LOWEST(HIGHEST(x, lo), hi)

#define ABSDIFF(lvalue, x, y) \
  (lvalue) = (((x) > (y)) ? (x) - (y) : (y) - (x))

#define PI (3.1415926535897896)
#define STRING_OR_NULL(s) ((s) == NULL ? "" : &*(s))

#define TEST_BITS(value, bits) (((value) & (bits)) != 0)
#define CLEAR_BITS(lvalue, bits) (lvalue) &= ~(bits)
#define SET_BITS(lvalue, bits) (lvalue) |= (bits)

#define SIGNED_R_SHIFT(value, shift) \
  ((shift) >= 0 ? (value) >> (shift) : (value) << -(shift))
#define SIGNED_L_SHIFT(value, shift) \
  ((shift) >= 0 ? (value) << (shift) : (value) >> -(shift))

#ifndef STRINGIFY
#define STRINGIFY2(n) #n
#define STRINGIFY(n) STRINGIFY2(n)
#endif

#ifndef CHECK_PRINTF
#ifdef __GNUC__
#define CHECK_PRINTF(string_index, arg_index) \
  __attribute__((format(printf, (string_index), (arg_index))))
#else
#define CHECK_PRINTF(string_index, arg_index)
#endif
#endif

#ifndef CONTAINER_OF_CHECK
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/* Require the pointer type implied by addr to exactly match that of member,
 * including the const and volatile qualification of the pointed-to type.
 */
#define CONTAINER_OF_CHECK(addr, type, member) \
  _Generic((&*(addr)), typeof(&((type *)0)->member): (void)0)
#else
/* Before C23, the conditional operator checks that the pointed-to types are
 * compatible, but its result type merges rather than compares qualifiers.
 */
#define CONTAINER_OF_CHECK(addr, type, member) \
  ((void)sizeof(0 ? &((type *)0)->member : (addr)))
#endif
#endif

#ifndef CONTAINER_OF_ADDR
#ifndef NDEBUG
static inline const volatile char *cb_non_null(
  const volatile void *const addr)
{
  assert(addr != NULL);
  return addr;
}
#define CONTAINER_OF_ADDR(addr) cb_non_null(&*(addr))
#else
#define CONTAINER_OF_ADDR(addr) ((char *)&*(addr))
#endif
#endif

#ifndef CONTAINER_OF
#define CONTAINER_OF(addr, type, member) \
  (CONTAINER_OF_CHECK(addr, type, member), \
   (type *)(CONTAINER_OF_ADDR(addr) - offsetof(type, member)))
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define C23_CONST const
#else
#define C23_CONST
#endif

#endif /* MacroUtils_h */
