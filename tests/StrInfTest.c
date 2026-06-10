/*
 * CBUtilLib test: String inflation
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
  /* Simple test */
  char output[sizeof expected];

  memset(output, CHAR_MAX, sizeof output);
  size_t count = strinflate(output, sizeof output, input, srch, rplc);
  assert(count == strlen(expected));
  assert(output[count] == '\0');
  assert(strcmp(expected, output) == 0);
}

static void test2(void)
{
  /* Get size with no buffer */
  size_t count = strinflate(NULL, 0, input, srch, rplc);
  assert(count == strlen(expected));
}

static void test3(void)
{
  /* Insufficient buffer sizes */
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
  /* Excessive buffer sizes */
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

void strinflate_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Simple", test1},
    {"Get size with no buffer", test2},
    {"Insufficient buffer sizes", test3},
    {"Excessive buffer sizes", test4},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
