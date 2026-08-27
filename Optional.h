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
  ACA: 09-Aug-25: Fix the calloc macro's parameter list.  Add fflush,
                  reallocarray, and freezero.
  ACA: 10-Aug-25: Add setlocale, time, and getgroups.
  CJB: 07-Jun-26: Add strpbrk.
  CJB: 01-Aug-26: Don't assume that freezero is available unless
                  __OpenBSD__ is defined because
                  __has_include(<readpassphrase.h>) was insufficient.
                  Don't assume that reallocarray is available unless
                  a suitable version of POSIX is available because
                  __has_include(<unistd.h>) was insufficient.
                  Similarly, guard the call to getgroups with an
                  unspecified _POSIX_VERSION.
                  Correct the type of the first parameter of getgroups
                  from size_t to int.
                  Add strdup.
 CJB: 05-Aug-26: Add strtoul and strtok.
 CJB: 18-Aug-26: Add tmpfile.
 CJB: 19-Aug-26: Add interceptors for the remaining common ISO C pointer
                 interfaces with optional arguments or results.
                 Implement const-preservation for functions such as strstr.
 CJB: 26-Aug-26: Preserve Fortify's strdup interceptor when enabled.
 CJB: 26-Aug-26: Preserve the remaining qualified Fortify allocation
                 interceptors and cover newer ISO C allocation interfaces.
 CJB: 27-Aug-26: Use more FORTIFY_INTERCEPTED_... macros to prevent problems
                 such as an unrecognized FILE * in Fortify because fopen was
                 redefined by this header.
 */

#ifndef Optional_h
#define Optional_h

#ifdef USE_OPTIONAL

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#include "StrExtra.h" // for strdup

#undef NULL
#define NULL ((_Optional void *)0)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define OPTIONAL_QCHAR_RESULT(result, str) \
_Generic(1 ? (str) : (char *)0, \
const char *: (_Optional const char *)(result), \
char *: (_Optional char *)(result))
#define OPTIONAL_QVOID_RESULT(result, ptr) \
_Generic(1 ? (ptr) : (void *)0, \
const void *: (_Optional const void *)(result), \
void *: (_Optional void *)(result))
#endif

#ifndef FORTIFY_INTERCEPTED_FOPEN
static inline _Optional FILE *optional_fopen(const char *name, const char *mode)
{
  return fopen(name, mode);
}
#undef fopen
#define fopen(p, n) optional_fopen(p, n)
#endif

#ifndef FORTIFY_INTERCEPTED_TMPFILE
static inline _Optional FILE *optional_tmpfile(void)
{
  return tmpfile();
}
#undef tmpfile
#define tmpfile() optional_tmpfile()
#endif

#ifndef FORTIFY_INTERCEPTED_FREOPEN
static inline _Optional FILE *optional_freopen(const char *restrict name,
                                               const char *restrict mode,
                                               FILE *restrict stream)
{
  return freopen(name, mode, stream);
}
#undef freopen
#define freopen(name, mode, stream) optional_freopen(name, mode, stream)
#endif

#ifndef FORTIFY_INTERCEPTED_FGETS
static inline _Optional char *optional_fgets(char *restrict str, int n,
                                             FILE *restrict stream)
{
  return fgets(str, n, stream);
}
#undef fgets
#define fgets(str, n, stream) optional_fgets(str, n, stream)
#endif

#ifndef FORTIFY_INTERCEPTED_TMPNAM
static inline _Optional char *optional_tmpnam(_Optional char *str)
{
  return tmpnam((char *)str);
}
#undef tmpnam
#define tmpnam(str) optional_tmpnam(str)
#endif

#ifndef FORTIFY_INTERCEPTED_FFLUSH
static inline int optional_fflush(_Optional FILE *stream)
{
  return fflush((FILE *)stream);
}
#undef fflush
#define fflush(stream) optional_fflush(stream)
#endif

#ifndef FORTIFY_INTERCEPTED_SETBUF
static inline void optional_setbuf(FILE *restrict stream,
                                   _Optional char *restrict buf)
{
  setbuf(stream, (char *)buf);
}
#undef setbuf
#define setbuf(stream, buf) optional_setbuf(stream, buf)
#endif

#ifndef FORTIFY_INTERCEPTED_FREE
static inline void optional_free(_Optional void *x)
{
  free((void *)x);
}
#undef free
#define free(x) optional_free(x)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#ifndef FORTIFY_INTERCEPTED_FREE_SIZED
static inline void optional_free_sized(_Optional void *p, size_t n)
{
  free_sized((void *)p, n);
}
#undef free_sized
#define free_sized(p, n) optional_free_sized(p, n)
#endif

#ifndef FORTIFY_INTERCEPTED_FREE_ALIGNED_SIZED
static inline void optional_free_aligned_sized(_Optional void *p,
                                               size_t alignment, size_t n)
{
  free_aligned_sized((void *)p, alignment, n);
}
#undef free_aligned_sized
#define free_aligned_sized(p, alignment, n) \
optional_free_aligned_sized(p, alignment, n)
#endif
#endif
#endif

#ifdef __OpenBSD__
#ifndef FORTIFY_INTERCEPTED_FREEZERO
static inline void optional_freezero(_Optional void *p, size_t n)
{
  freezero((void *)p, n);
}
#undef freezero
#define freezero(p, n) optional_freezero(p, n)
#endif
#endif

#ifndef FORTIFY_INTERCEPTED_MALLOC
static inline _Optional void *optional_malloc(size_t n)
{
  return malloc(n);
}
#undef malloc
#define malloc(n) optional_malloc(n)
#endif

#ifndef FORTIFY_INTERCEPTED_CALLOC
static inline _Optional void *optional_calloc(size_t sz, size_t n)
{
  return calloc(sz, n);
}
#undef calloc
#define calloc(sz, n) optional_calloc(sz, n)
#endif

#ifndef FORTIFY_INTERCEPTED_REALLOC
static inline _Optional void *optional_realloc(_Optional void *p, size_t n)
{
  return realloc((void *)p, n);
}
#undef realloc
#define realloc(p, n) optional_realloc(p, n)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#ifndef FORTIFY_INTERCEPTED_ALIGNED_ALLOC
static inline _Optional void *optional_aligned_alloc(size_t alignment,
                                                     size_t n)
{
  return aligned_alloc(alignment, n);
}
#undef aligned_alloc
#define aligned_alloc(alignment, n) optional_aligned_alloc(alignment, n)
#endif
#endif

static inline _Optional void *optional_bsearch(const void *key, const void *base,
                                               size_t n, size_t size,
                                               int (*cmp)(const void *,
                                                          const void *))
{
  return (bsearch)(key, base, n, size, cmp);
}
#undef bsearch
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define bsearch(key, base, n, size, cmp) \
OPTIONAL_QVOID_RESULT(optional_bsearch(key, base, n, size, cmp), base)
#else
#define bsearch(key, base, n, size, cmp) \
optional_bsearch(key, base, n, size, cmp)
#endif

#ifndef FORTIFY_INTERCEPTED_GETENV
static inline _Optional char *optional_getenv(const char *name)
{
  return getenv(name);
}
#undef getenv
#define getenv(name) optional_getenv(name)
#endif

#if defined(_POSIX_VERSION) && _POSIX_VERSION >= 202405L
#ifndef FORTIFY_INTERCEPTED_REALLOCARRAY
static inline _Optional void *optional_reallocarray(_Optional void *p, size_t n,
                                                    size_t sz)
{
  return reallocarray((void *)p, n, sz);
}
#undef reallocarray
#define reallocarray(p, n, sz) optional_reallocarray(p, n, sz)
#endif
#endif

static inline long optional_strtol(const char *restrict str,
                                   char *_Optional *restrict str_end, int base)
{
  return strtol(str, (char **)str_end, base);
}
#undef strtol
#define strtol(str, str_end, base) optional_strtol(str, str_end, base)

static inline unsigned long optional_strtoul(const char *restrict str,
                                             char *_Optional *restrict str_end, int base)
{
  return strtoul(str, (char **)str_end, base);
}
#undef strtoul
#define strtoul(str, str_end, base) optional_strtoul(str, str_end, base)

static inline long long optional_strtoll(const char *restrict str,
                                         char *_Optional *restrict str_end,
                                         int base)
{
  return strtoll(str, (char **)str_end, base);
}
#undef strtoll
#define strtoll(str, str_end, base) optional_strtoll(str, str_end, base)

static inline unsigned long long optional_strtoull(
                                                   const char *restrict str, char *_Optional *restrict str_end, int base)
{
  return strtoull(str, (char **)str_end, base);
}
#undef strtoull
#define strtoull(str, str_end, base) optional_strtoull(str, str_end, base)

static inline double optional_strtod(const char *restrict str,
                                     char *_Optional *restrict str_end)
{
  return strtod(str, (char **)str_end);
}
#undef strtod
#define strtod(str, str_end) optional_strtod(str, str_end)

static inline float optional_strtof(const char *restrict str,
                                    char *_Optional *restrict str_end)
{
  return strtof(str, (char **)str_end);
}
#undef strtof
#define strtof(str, str_end) optional_strtof(str, str_end)

static inline long double optional_strtold(const char *restrict str,
                                           char *_Optional *restrict str_end)
{
  return strtold(str, (char **)str_end);
}
#undef strtold
#define strtold(str, str_end) optional_strtold(str, str_end)

static inline int optional_system(_Optional const char *command)
{
  return system((const char *)command);
}
#undef system
#define system(command) optional_system(command)

static inline int optional_mblen(_Optional const char *str, size_t n)
{
  return mblen((const char *)str, n);
}
#undef mblen
#define mblen(str, n) optional_mblen(str, n)

static inline int optional_mbtowc(_Optional wchar_t *restrict wc,
                                  _Optional const char *restrict str, size_t n)
{
  return mbtowc((wchar_t *)wc, (const char *)str, n);
}
#undef mbtowc
#define mbtowc(wc, str, n) optional_mbtowc(wc, str, n)

static inline int optional_wctomb(_Optional char *str, wchar_t wc)
{
  return wctomb((char *)str, wc);
}
#undef wctomb
#define wctomb(str, wc) optional_wctomb(str, wc)

static inline _Optional void *optional_memchr(const void *str, int ch, size_t n)
{
  return (memchr)(str, ch, n);
}
#undef memchr
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define memchr(str, ch, n) \
OPTIONAL_QVOID_RESULT(optional_memchr(str, ch, n), str)
#else
#define memchr(str, ch, n) optional_memchr(str, ch, n)
#endif

static inline _Optional char *optional_strstr(const char *str,
                                              const char *substr)
{
  return (strstr)(str, substr);
}
#undef strstr
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define strstr(str, substr) \
OPTIONAL_QCHAR_RESULT(optional_strstr(str, substr), str)
#else
#define strstr(str, substr) optional_strstr(str, substr)
#endif

static inline _Optional char *optional_strchr(const char *str, int ch)
{
  return (strchr)(str, ch);
}
#undef strchr
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define strchr(str, ch) OPTIONAL_QCHAR_RESULT(optional_strchr(str, ch), str)
#else
#define strchr(str, ch) optional_strchr(str, ch)
#endif

static inline _Optional char *optional_strrchr(const char *str, int ch)
{
  return (strrchr)(str, ch);
}
#undef strrchr
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define strrchr(str, ch) OPTIONAL_QCHAR_RESULT(optional_strrchr(str, ch), str)
#else
#define strrchr(str, ch) optional_strrchr(str, ch)
#endif

static inline _Optional char *optional_strpbrk(const char *str, const char *brk)
{
  return (strpbrk)(str, brk);
}
#undef strpbrk
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define strpbrk(str, brk) \
OPTIONAL_QCHAR_RESULT(optional_strpbrk(str, brk), str)
#else
#define strpbrk(str, brk) optional_strpbrk(str, brk)
#endif

#ifndef FORTIFY_INTERCEPTED_STRDUP
static inline _Optional char *optional_strdup(const char *str)
{
  return strdup(str);
}
#undef strdup
#define strdup(str) optional_strdup(str)
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) || \
    (defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L)
#ifndef FORTIFY_INTERCEPTED_STRNDUP
static inline _Optional char *optional_strndup(const char *str, size_t n)
{
  return strndup(str, n);
}
#undef strndup
#define strndup(str, n) optional_strndup(str, n)
#endif
#endif

static inline _Optional char *optional_strtok(_Optional char *str, const char *delimiters)
{
  return strtok((char *)str, delimiters);
}
#undef strtok
#define strtok(str, delimiters) optional_strtok(str, delimiters)

#ifndef FORTIFY_INTERCEPTED_SETLOCALE
static inline _Optional char *optional_setlocale(int cat,
                                                 _Optional const char *l)
{
  return setlocale(cat, (const char *)l);
}
#undef setlocale
#define setlocale(cat, l) optional_setlocale(cat, l)
#endif

static inline time_t optional_time(_Optional time_t *tp)
{
  return time((time_t *)tp);
}
#undef time
#define time(tp) optional_time(tp)

#ifndef FORTIFY_INTERCEPTED_GMTIME
static inline _Optional struct tm *optional_gmtime(const time_t *timer)
{
  return gmtime(timer);
}
#undef gmtime
#define gmtime(timer) optional_gmtime(timer)
#endif

#ifndef FORTIFY_INTERCEPTED_LOCALTIME
static inline _Optional struct tm *optional_localtime(const time_t *timer)
{
  return localtime(timer);
}
#undef localtime
#define localtime(timer) optional_localtime(timer)
#endif

#ifdef _POSIX_VERSION
static inline int optional_getgroups(int n, _Optional gid_t gids[n])
{
  return getgroups(n, (gid_t *)gids);
}
#undef getgroups
#define getgroups(n, gids) optional_getgroups(n, gids)
#endif

#else
#define _Optional
#endif

#endif /* Optional_h */
