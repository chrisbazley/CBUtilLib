/*
 * CBUtilLib test: Read & write integers to/from streams
 * Copyright (C) 2019 Christopher Bazley
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

/* ISO library headers */
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* CBUtilLib headers */
#include "FileRWInt.h"

/* Local headers */
#include "Tests.h"

#define PATH "<Wimp$ScrapDir>.RWIntTest"
enum {
  TEST = 345769078,
  DUMMY = 1234567,
};

static const long int cases[] = {
  INT32_MAX,
  INT32_MIN,
  INT32_MAX - 1,
  INT32_MIN + 1,
  0,
  1,
  -1,
  TEST
};

static FILE *test_fopen(const char *const filename, const char *const mode)
{
  _Optional FILE *const f = fopen(filename, mode);
  if (f == NULL) perror("Failed to open file");
  assert(f != NULL);
  return &*f;
}

static void test1(void)
{
  /* Read */
  for (size_t i = 0; i < ARRAY_SIZE(cases); ++i) {
    FILE *f = test_fopen(PATH, "wb");

    unsigned long int const uint = (unsigned long)cases[i];
    uint8_t c = (uint8_t)uint;
    assert(fputc(c, f) == c);
    c = (uint8_t)(uint >> 8);
    assert(fputc(c, f) == c);
    c = (uint8_t)(uint >> 16);
    assert(fputc(c, f) == c);
    c = (uint8_t)(uint >> 24);
    assert(fputc(c, f) == c);

    assert(!fclose(f));
    f = test_fopen(PATH, "rb");

    long int num = DUMMY;
    assert(fread_int32le(&num, f));
    assert(ftell(f) == sizeof(uint32_t));
    assert(!ferror(f));
    assert(!feof(f));
    assert(num == cases[i]);
    assert(!fclose(f));
    remove(PATH);
  }
}

static void test2(void)
{
  /* Write */
  for (size_t i = 0; i < ARRAY_SIZE(cases); ++i) {
    FILE *f = test_fopen(PATH, "wb");

    assert(fwrite_int32le(cases[i], f));
    assert(ftell(f) == sizeof(uint32_t));
    assert(!ferror(f));
    assert(!feof(f));

    assert(!fclose(f));
    f = test_fopen(PATH, "rb");

    unsigned long int const uint = (unsigned long)cases[i];
    assert(fgetc(f) == (uint8_t)uint);
    assert(fgetc(f) == (uint8_t)(uint >> 8));
    assert(fgetc(f) == (uint8_t)(uint >> 16));
    assert(fgetc(f) == (uint8_t)(uint >> 24));

    assert(!fclose(f));
    remove(PATH);
  }
}

static void test3(void)
{
#ifdef FORTIFY
  /* Read fail */
  FILE *f = fopen(PATH, "wb");
  if (f == NULL) perror("Failed to open file");
  assert(f != NULL);

  static uint8_t const data[] = {1, 2, 3, 4};
  assert(fwrite(data, sizeof(*data), ARRAY_SIZE(data), f));

  assert(!fclose(f));
  f = fopen(PATH, "rb");
  if (f == NULL) perror("Failed to open file");
  assert(f != NULL);

  long int num = DUMMY;

  Fortify_SetNumAllocationsLimit(0);
  assert(!fread_int32le(&num, f));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(ftell(f) == 0);
  assert(ferror(f));
  assert(!feof(f));
  assert(num == DUMMY);

  assert(!fclose(f));
  remove(PATH);
#endif
}

static void test4(void)
{
#ifdef FORTIFY
  /* Write fail */
  FILE *f = fopen(PATH, "wb");
  if (f == NULL) perror("Failed to open file");
  assert(f != NULL);

  Fortify_SetNumAllocationsLimit(0);
  assert(!fwrite_int32le(TEST, f));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(ftell(f) == 0);
  assert(ferror(f));
  assert(!feof(f));

  if (fclose(f)) { perror("fclose failed"); }
  remove(PATH);
#endif
}

static void test5(void)
{
  /* Round trip */
  for (size_t i = 0; i < ARRAY_SIZE(cases); ++i) {
    FILE *f = test_fopen(PATH, "wb");

    assert(fwrite_int32le(cases[i], f));
    assert(ftell(f) == sizeof(uint32_t));
    assert(!ferror(f));
    assert(!feof(f));

    assert(!fclose(f));
    f = test_fopen(PATH, "rb");

    long int num = DUMMY;
    assert(fread_int32le(&num, f));
    assert(ftell(f) == sizeof(uint32_t));
    assert(!ferror(f));
    assert(!feof(f));
    assert(num == cases[i]);

    assert(!fclose(f));
    remove(PATH);
  }
}

void FileRWInt_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  }
  unit_tests[] =
  {
    { "Read", test1 },
    { "Write", test2 },
    { "Read fail", test3 },
    { "Write fail", test4 },
    { "Round trip", test5 },
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count ++)
  {
    printf("Test %zu/%zu : %s\n",
           1 + count,
           ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
