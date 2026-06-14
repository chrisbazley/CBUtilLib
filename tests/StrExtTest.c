/*
 * CBUtilLib test: Extra string functions not part of the ANSI standard
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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* CBUtilLib headers */
#include "StrExtra.h"

/* Local headers */
#include "Tests.h"

static const char
  input[] = "beg 4 a fab bag. don't be eager.",
  srch[] = "abcefg.4r", *const rplc[sizeof srch - 1] = {"Alpha ",   "Bravo ",
                                                        "Charlie ", "Echo ",
                                                        "Foxtrot ", "Golf ",
                                                        "Stop ",    "Four ",
                                                        "Romeo "},
  expected[] =
    "Bravo Echo Golf  Four  Alpha  Foxtrot Alpha Bravo  Bravo Alpha Golf "
    "Stop  don't Bravo Echo  Echo Alpha Golf Echo Romeo Stop ";

static void test1(void)
{
  /* Simple string inflation */
  char output[sizeof expected];

  memset(output, CHAR_MAX, sizeof output);
  size_t count = strinflate(output, sizeof output, input, srch, rplc);
  assert(count == strlen(expected));
  assert(output[count] == '\0');
  assert(strcmp(expected, output) == 0);
}

static void test2(void)
{
  /* Get inflated string size with no buffer */
  size_t count = strinflate(NULL, 100, input, srch, rplc);
  assert(count == strlen(expected));
}

static void test3(void)
{
  /* Insufficient buffer sizes for string inflation */
  char output[sizeof expected] = "";
  for (size_t n = 0; n < sizeof output; ++n)
  {
    memset(output, CHAR_MAX, sizeof output);
    size_t count = strinflate(output, n, input, srch, rplc);
    assert(count >= n);
    assert(count == strlen(expected));

    for (size_t i = n; i < sizeof output; ++i)
    {
      assert(output[i] == CHAR_MAX);
    }

    if (n > 0)
    {
      assert(output[n - 1] == '\0');
      assert(strncmp(expected, output, n - 1) == 0);
    }
  }
}

static void test4(void)
{
  /* Excessive buffer sizes for string inflation */
  char output[2 * sizeof expected] = "";
  for (size_t n = sizeof expected; n <= sizeof output; ++n)
  {
    memset(output, CHAR_MAX, sizeof output);
    size_t count = strinflate(output, n, input, srch, rplc);
    assert(count < n);
    assert(count == strlen(expected));
    for (size_t i = count + 1; i < sizeof output; ++i)
    {
      assert(output[i] == CHAR_MAX);
    }

    assert(output[count] == '\0');
    assert(strcmp(expected, output) == 0);
  }
}

static void test5(void)
{
  /* Duplicate string */
  static const char *const strings[] = {"", "norfolk",
                                        "norfolk\tstreet\rbakery\n"};
  FOR_EACH_ELEM_PTR(const char *const *, s, strings)
  {
    _Optional char *d = strdup(*s);
    assert(d != NULL);
    assert(d != *s);
    assert(strcmp(&*d, *s) == 0);
    free(d);
  }
}

static void test6(void)
{
#ifdef FORTIFY
  /* Duplicate string fail */
  static const char *const s = "foo";

  Fortify_SetAllocationLimit(0);
  _Optional char *d = strdup(s);
  Fortify_SetAllocationLimit(ULONG_MAX);
  assert(d == NULL);

  d = strdup(s);
  assert(d != NULL);
  assert(d != s);
  assert(strcmp(&*d, s) == 0);
  free(d);
#endif
}

typedef enum
{
  LT = -1,
  EQ = 0,
  GT = 1
} TestCaseDiff;

static void test7(void)
{
  /* Case-insensitive string comparison */

  typedef const struct
  {
    const char *s1, *s2;
    TestCaseDiff diff;
  } TestCase;

  static TestCase cases[] = {
    {"CoW", "cow", EQ},   {"horse", "HoRsE", EQ}, {"", "sheep", LT},
    {"HEN", "", GT},      {"HEN", "sheep", LT},   {"hen", "SHEEP", LT},
    {"HEN", "horse", LT}, {"hen", "HORSE", LT},   {"cow", "coward", LT},
    {"HEN", "cow", GT},   {"hen", "COW", GT},     {"HORSE", "cow", GT},
    {"horse", "COW", GT}, {"coward", "cow", GT},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    const int diff = stricmp(cp->s1, cp->s2);
    assert((diff > 0) == (cp->diff > 0));
    assert((diff == 0) == (cp->diff == 0));
    assert((diff < 0) == (cp->diff < 0));
  }
}

static void test8(void)
{
  /* Case-insensitive substring comparison */
  typedef enum
  {
    LT = -1,
    EQ = 0,
    GT = 1
  } TestCaseDiff;

  typedef const struct
  {
    const char *s1, *s2;
    size_t n;
    TestCaseDiff diff;
  } TestCase;

  static TestCase cases[] = {
    {"CoW", "cow", 3, EQ},    {"CoW", "cow", 0, EQ},
    {"CoW", "cow", 4, EQ},    {"horse", "HoRsE", 5, EQ},
    {"cow", "coward", 3, EQ}, {"coward", "cow", 3, EQ},
    {"", "sheep", 5, LT},     {"", "sheep", 6, LT},
    {"", "sheep", 1, LT},     {"HEN", "", 3, GT},
    {"HEN", "", 4, GT},       {"HEN", "", 1, GT},
    {"HEN", "sheep", 5, LT},  {"hen", "SHEEP", 5, LT},
    {"HEN", "horse", 5, LT},  {"hen", "HORSE", 5, LT},
    {"cow", "coward", 6, LT}, {"cow", "coward", 4, LT},
    {"HEN", "cow", 3, GT},    {"hen", "COW", 3, GT},
    {"HORSE", "cow", 5, GT},  {"horse", "COW", 5, GT},
    {"coward", "cow", 6, GT}, {"coward", "cow", 4, GT},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    const int diff = strnicmp(cp->s1, cp->s2, cp->n);
    assert((diff > 0) == (cp->diff > 0));
    assert((diff == 0) == (cp->diff == 0));
    assert((diff < 0) == (cp->diff < 0));
  }
}

void test9(void)
{
  typedef const struct
  {
    const char *s; int c; size_t n;
    const char *tail;
  } TestCase;

  static TestCase cases[] = {
    {"ADFS::App1.$.!Paint",
      '.', 0,
      ""},
    {"ADFS::App1.$.!Paint",
      '.', 1,
      "!Paint"},
    {"ADFS::App1.$.!Paint",
      '.', 2,
      "$.!Paint"},
    {"ADFS::App1.$.!Paint",
      '.', 3,
      "ADFS::App1.$.!Paint"},
    {"ADFS::App1.$.!Paint",
      '.', 4,
      "ADFS::App1.$.!Paint"},
    {"ADFS::App1.$.!Paint",
      UCHAR_MAX + 1 + '.', 1,
      "!Paint"},
    {"ADFS::App1.$.!Paint",
      '.' - UCHAR_MAX - 1, 1,
      "!Paint"},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    char *const tail = strtail(cp->s, cp->c, cp->n);
    assert(strcmp(tail, cp->tail) == 0);
  }
}

void StrExtra_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Simple inflation", test1},
    {"Get inflated string size with no buffer", test2},
    {"Insufficient buffer sizes for string inflation", test3},
    {"Excessive buffer sizes for string inflation", test4},
    {"Duplicate string", test5},
    {"Duplicate string fail", test6},
    {"Case-insensitive string comparison", test7},
    {"Case-insensitive substring comparison", test8},
    {"String tail", test9},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
