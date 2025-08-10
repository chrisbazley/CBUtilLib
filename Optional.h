/*
 * CBLibrary: _Optional qualifier support
 * Copyright (C) 2025 Christopher Bazley
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

/* Optional.h declares macros for dogfooding a new type qualifier

Dependencies: ANSI C library.
Message tokens: None
History:
  CJB: 25-Apr-25: New header file.
  CJB: 03-May-25: Added strtod, strstr and strchr.
  ACA: 09-Aug-25: Fix the calloc macro's parameter list.  Add fflush.
  ACA: 10-Aug-25: Add setlocale, time.
*/

#ifndef Optional_h
#define Optional_h

#ifdef USE_OPTIONAL

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#undef NULL
#define NULL ((_Optional void *)0)

static inline _Optional FILE *optional_fopen(const char *name, const char *mode)
{
    return fopen(name, mode);
}
#undef fopen
#define fopen(p, n) optional_fopen(p, n)

static inline int optional_fflush(_Optional FILE *stream)
{
    return fflush((FILE *) stream);
}
#undef fflush
#define fflush(stream) optional_fflush(stream)

static inline void optional_free(_Optional void *x)
{
    free((void *)x);
}
#undef free
#define free(x) optional_free(x)

static inline _Optional void *optional_malloc(size_t n)
{
    return malloc(n);
}
#undef malloc
#define malloc(n) optional_malloc(n)

static inline _Optional void *optional_calloc(size_t sz, size_t n)
{
    return calloc(sz, n);
}
#undef calloc
#define calloc(sz, n) optional_calloc(sz, n)

static inline _Optional void *optional_realloc(_Optional void *p, size_t n)
{
    return realloc((void *)p, n);
}
#undef realloc
#define realloc(p, n) optional_realloc(p, n)

static inline long optional_strtol(const char * restrict str, char *_Optional * restrict str_end, int base)
{
    return strtol(str, (char **)str_end, base);
}
#undef strtol
#define strtol(str, str_end, base) optional_strtol(str, str_end, base)

static inline double optional_strtod(const char * restrict str, char *_Optional * restrict str_end)
{
    return strtod(str, (char **)str_end);
}
#undef strtod
#define strtod(str, str_end) optional_strtod(str, str_end)

static inline _Optional char *optional_strstr(const char *str, const char *substr)
{
    return strstr(str, substr);
}
#undef strstr
#define strstr(str, substr) optional_strstr(str, substr)

static inline _Optional char *optional_strchr(const char *str, int ch)
{
    return strchr(str, ch);
}
#undef strchr
#define strchr(str, ch) optional_strchr(str, ch)

static inline _Optional char *optional_setlocale(int cat, _Optional const char *l)
{
    return setlocale(cat, (const char *) l);
}
#undef setlocale
#define setlocale(cat, l)  optional_setlocale(cat, l)

static inline time_t optional_time(_Optional time_t *tp)
{
    return time((time_t *) tp);
}
#undef time
#define time(tp)  optional_time(tp)

#else
#define _Optional
#endif

#endif /* Optional_h */
