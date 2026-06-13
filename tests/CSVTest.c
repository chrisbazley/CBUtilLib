/*
 * CBUtilLib test: Parse signed decimal values in comma-separated value format
 * Copyright (C) 2026 Christopher Bazley
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
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* CBUtilLib headers */
#include "CSV.h"

/* Local headers */
#include "Tests.h"

static void test1(void)
{
  /* Integer string parsing */
  typedef const struct
  {
    const char *s;
    size_t n;
    int expected[3];
  } TestCase;

  static TestCase cases[] = {
    {" 23, -476, 12", 3, {23, -476, 12}},
    {"23,-476,12", 3, {23, -476, 12}},
    {"23 -476,12 9, -73", 3, {23, 12, -73}},
    {"23, -476", 2, {23, -476}},
    {"23", 1, {23}},
    {"23, ", 2, {23, 0}},
    {", -476", 2, {0, -476}},
    {",", 2, {0, 0}},
    {"\r", 0, {0}},
    {"\n", 0, {0}},
    {"\r\n", 0, {0}},
    {"\n\r", 0, {0}},
    {"23, -476, 12\r9, -72", 3, {23, -476, 12}},
    {"23, -476, 12\r9, -71", 3, {23, -476, 12}},
    {"23, -476, 12\n\r9, 2", 3, {23, -476, 12}},
    {"23, -476, 12\r\n9, 89", 3, {23, -476, 12}},
    {"0x20, 0X40", 2, {32, 64}},
    {"1E2, 100e-1", 2, {1, 100}},
    {"a1,'b',c2", 3, {0, 0, 0}},
    {"2.999, -5.999", 2, {2, -5}},
    {"32767, -32768", 2, {32767, -32768}},
    {"2147483647, -2147483648", 2, {INT_MAX, INT_MIN}},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    int output[10];
    for (size_t nmemb = 0; nmemb <= ARRAY_SIZE(output); ++nmemb)
    {
      FOR_EACH_ELEM_PTR(int *, op, output)
      {
        *op = INT_MAX;
      }

      _Optional const char *expected_endp = strpbrk(cp->s, "\n\r");
      if (expected_endp != NULL)
      {
        expected_endp += strspn(&*expected_endp, "\n\r");
      }

      _Optional char *endp = &(char){0};
      size_t const n = csv_parse_as_int(cp->s, &endp, output, nmemb);
      assert(n == cp->n);
      assert(endp == expected_endp);

      const size_t nexp = nmemb > cp->n ? cp->n : nmemb;
      size_t i;
      for (i = 0; i < nexp; ++i)
      {
        assert(output[i] == cp->expected[i]);
      }

      for (; i < ARRAY_SIZE(output); ++i)
      {
        assert(output[i] == INT_MAX);
      }
    }
  }
}

static void test2(void)
{
  /* Long integer string parsing */
  typedef const struct
  {
    const char *s;
    size_t n;
    long int expected[3];
  } TestCase;

  static TestCase cases[] = {
    {" 23, -476, 12", 3, {23, -476, 12}},
    {"23,-476,12", 3, {23, -476, 12}},
    {"23 -476,12 9, -73", 3, {23, 12, -73}},
    {"23, -476", 2, {23, -476}},
    {"23", 1, {23}},
    {"23, ", 2, {23, 0}},
    {", -476", 2, {0, -476}},
    {",", 2, {0, 0}},
    {"\r", 0, {0}},
    {"\n", 0, {0}},
    {"\r\n", 0, {0}},
    {"\n\r", 0, {0}},
    {"23, -476, 12\r9, -72", 3, {23, -476, 12}},
    {"23, -476, 12\r9, -71", 3, {23, -476, 12}},
    {"23, -476, 12\n\r9, 2", 3, {23, -476, 12}},
    {"23, -476, 12\r\n9, 89", 3, {23, -476, 12}},
    {"0x20, 0X40", 2, {32, 64}},
    {"1E2, 100e-1", 2, {1, 100}},
    {"a1,'b',c2", 3, {0, 0, 0}},
    {"2.999, -5.999", 2, {2, -5}},
    {"32767, -32768", 2, {32767, -32768}},
    {"2147483647, -2147483648", 2, {2147483647, -2147483648}},
    {"9223372036854775807, -9223372036854775808", 2, {LONG_MAX, LONG_MIN}},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    long int output[10];
    for (size_t nmemb = 0; nmemb <= ARRAY_SIZE(output); ++nmemb)
    {
      FOR_EACH_ELEM_PTR(long int *, op, output)
      {
        *op = LONG_MAX;
      }

      _Optional const char *expected_endp = strpbrk(cp->s, "\n\r");
      if (expected_endp != NULL)
      {
        expected_endp += strspn(&*expected_endp, "\n\r");
      }

      _Optional char *endp = &(char){0};
      size_t const n = csv_parse_as_long(cp->s, &endp, output, nmemb);
      assert(n == cp->n);
      assert(endp == expected_endp);

      const size_t nexp = nmemb > cp->n ? cp->n : nmemb;
      size_t i;
      for (i = 0; i < nexp; ++i)
      {
        assert(output[i] == cp->expected[i]);
      }

      for (; i < ARRAY_SIZE(output); ++i)
      {
        assert(output[i] == LONG_MAX);
      }
    }
  }
}

static void test3(void)
{
  /* Floating point string parsing */
  typedef const struct
  {
    const char *s;
    size_t n;
    double expected[3];
  } TestCase;

  static TestCase cases[] = {
    {" 23, -476, 12", 3, {23, -476, 12}},
    {"23,-476,12", 3, {23, -476, 12}},
    {"23 -476,12 9, -73", 3, {23, 12, -73}},
    {"23, -476", 2, {23, -476}},
    {"23", 1, {23}},
    {"23, ", 2, {23, 0}},
    {", -476", 2, {0, -476}},
    {",", 2, {0, 0}},
    {"\r", 0, {0}},
    {"\n", 0, {0}},
    {"\r\n", 0, {0}},
    {"\n\r", 0, {0}},
    {"23, -476, 12\r9, -72", 3, {23, -476, 12}},
    {"23, -476, 12\r9, -71", 3, {23, -476, 12}},
    {"23, -476, 12\n\r9, 2", 3, {23, -476, 12}},
    {"23, -476, 12\r\n9, 89", 3, {23, -476, 12}},
    {"0x20, 0X40", 2, {32, 64}},
    {"1E2, 100e-1", 2, {100, 10}},
    {"a1,'b',c2", 3, {0, 0, 0}},
    {"2.999, -5.999", 2, {2.999, -5.999}},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    double output[10];
    for (size_t nmemb = 0; nmemb <= ARRAY_SIZE(output); ++nmemb)
    {
      FOR_EACH_ELEM_PTR(double *, op, output)
      {
        *op = DBL_MAX;
      }

      _Optional const char *expected_endp = strpbrk(cp->s, "\n\r");
      if (expected_endp != NULL)
      {
        expected_endp += strspn(&*expected_endp, "\n\r");
      }

      _Optional char *endp = &(char){0};
      size_t const n = csv_parse_as_double(cp->s, &endp, output, nmemb);
      assert(n == cp->n);
      assert(endp == expected_endp);

      const size_t nexp = nmemb > cp->n ? cp->n : nmemb;
      size_t i;
      for (i = 0; i < nexp; ++i)
      {
        assert(fabs(output[i] - cp->expected[i]) < DBL_EPSILON);
      }

      for (; i < ARRAY_SIZE(output); ++i)
      {
        assert(output[i] == DBL_MAX);
      }
    }
  }
}

static void test4(void)
{
  /* Unsigned char string parsing */
  typedef const struct
  {
    const char *s;
    size_t n;
    double expected[3];
  } TestCase;

  static TestCase cases[] = {
    {" 23, -476, 12", 3, {23, 0, 12}},
    {"23,-476,12", 3, {23, 0, 12}},
    {"23 -476,12 9, -73", 3, {23, 12, 0}},
    {"23, -476", 2, {23, 0}},
    {"23", 1, {23}},
    {"23, ", 2, {23, 0}},
    {", -476", 2, {0, -0}},
    {",", 2, {0, 0}},
    {"\r", 0, {0}},
    {"\n", 0, {0}},
    {"\r\n", 0, {0}},
    {"\n\r", 0, {0}},
    {"23, -476, 12\r9, -72", 3, {23, 0, 12}},
    {"23, -476, 12\r9, -71", 3, {23, 0, 12}},
    {"23, -476, 12\n\r9, 2", 3, {23, 0, 12}},
    {"23, -476, 12\r\n9, 89", 3, {23, 0, 12}},
    {"0x20, 0X40", 2, {32, 64}},
    {"1E2, 100e-1", 2, {1, 100}},
    {"a1,'b',c2", 3, {0, 0, 0}},
    {"2.999, -5.999", 2, {2, 0}},
    {"255", 1, {UCHAR_MAX}},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    unsigned char output[10];
    for (size_t nmemb = 0; nmemb <= ARRAY_SIZE(output); ++nmemb)
    {
      FOR_EACH_ELEM_PTR(unsigned char *, op, output)
      {
        *op = UCHAR_MAX;
      }

      _Optional const char *expected_endp = strpbrk(cp->s, "\n\r");
      if (expected_endp != NULL)
      {
        expected_endp += strspn(&*expected_endp, "\n\r");
      }

      _Optional char *endp = &(char){0};
      size_t const n = csv_parse_as_uchar(cp->s, &endp, output, nmemb);
      assert(n == cp->n);
      assert(endp == expected_endp);

      const size_t nexp = nmemb > cp->n ? cp->n : nmemb;
      size_t i;
      for (i = 0; i < nexp; ++i)
      {
        assert(fabs(output[i] - cp->expected[i]) < DBL_EPSILON);
      }

      for (; i < ARRAY_SIZE(output); ++i)
      {
        assert(output[i] == UCHAR_MAX);
      }
    }
  }
}

static void test5(void)
{
  /* Integer string parsing with null buffer pointer */
  const char *s = "1,2,3\n";

  _Optional char *endp = &(char){0};
  size_t const n = csv_parse_as_int(s, &endp, NULL, 100);
  assert(n == 3);
  assert(endp == s + strlen(s));
}

static void test6(void)
{
  /* Long integer string parsing with null buffer pointer */
  const char *const s = "1,2,3\n";
  _Optional char *endp = &(char){0};
  size_t const n = csv_parse_as_long(s, &endp, NULL, 100);
  assert(n == 3);
  assert(endp == s + strlen(s));
}

static void test7(void)
{
  /* Floating point parsing with null buffer pointer */
  const char *const s = "1,2,3\n";
  _Optional char *endp = &(char){0};
  size_t const n = csv_parse_as_double(s, &endp, NULL, 100);
  assert(n == 3);
  assert(endp == s + strlen(s));
}

static void test8(void)
{
  /* Unsigned char parsing with null buffer pointer */
  const char *const s = "1,2,3\n";
  _Optional char *endp = &(char){0};
  size_t const n = csv_parse_as_uchar(s, &endp, NULL, 100);
  assert(n == 3);
  assert(endp == s + strlen(s));
}

static void test9(void)
{
  /* Integer string parsing with null end pointer */
  const char *const s = "1,2,3\n";
  int output[10];
  size_t const n = csv_parse_as_int(s, NULL, output, ARRAY_SIZE(output));
  assert(n == 3);
}

static void test10(void)
{
  /* Long integer string parsing with null end pointer */
  const char *const s = "1,2,3\n";
  long int output[10];
  size_t const n = csv_parse_as_long(s, NULL, output, ARRAY_SIZE(output));
  assert(n == 3);
}

static void test11(void)
{
  /* Floating point parsing with null end pointer */
  const char *const s = "1,2,3\n";
  double output[10];
  size_t const n = csv_parse_as_double(s, NULL, output, ARRAY_SIZE(output));
  assert(n == 3);
}

static void test12(void)
{
  /* Unsigned char parsing with null end pointer */
  const char *const s = "1,2,3\n";
  unsigned char output[10];
  size_t const n = csv_parse_as_uchar(s, NULL, output, ARRAY_SIZE(output));
  assert(n == 3);
}

void CSV_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Integer string parsing", test1},
    {"Long integer string parsing", test2},
    {"Floating point string parsing", test3},
    {"Unsigned char string parsing", test4},
    {"Integer string parsing with null buffer pointer", test5},
    {"Long integer string parsing with null buffer pointer", test6},
    {"Floating point parsing with null buffer pointer", test7},
    {"Unsigned char parsing with null buffer pointer", test8},
    {"Integer string parsing with null end pointer", test9},
    {"Long integer string parsing with null end pointer", test10},
    {"Floating point parsing with null end pointer", test11},
    {"Unsigned char parsing with null end pointer", test12},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
