/*
 * CBUtilLib test: Command-line argument parser utilities
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
#include "ArgUtils.h"

/* Local headers */
#include "Tests.h"

static void test1(void)
{
  /* Long integer argument parsing */
  typedef const struct
  {
    long int min, max;
    int n, argc;
    const char *argv[10];
    bool success;
    long int value;
  } TestCase;

  static TestCase cases[] = {
    // Argument selection
    {LONG_MIN, LONG_MAX, 0, 1, {"23"}, true, 23},
    {LONG_MIN, LONG_MAX, 1, 2, {"5", "10"}, true, 10},
    {LONG_MIN,
     LONG_MAX,
     4,
     7,
     {"1", "67", "43", "2", "444", "65", "6"},
     true,
     444},
    // Invalid argument index (treated as missing argument)
    {LONG_MIN, LONG_MAX, 1, 1, {"23"}, false, 0},
    // Value out of range
    {1, 2, 0, 1, {"0"}, false, 0},
    {0, 1, 0, 1, {"2"}, false, 0},
    // Switch in place of expected numeric argument
    {LONG_MIN, LONG_MAX, 0, 1, {"-foo"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"- 1"}, false, 0},
    // Interesting number formats
    {LONG_MIN, LONG_MAX, 0, 1, {"010"}, true, 8},
    {LONG_MIN, LONG_MAX, 0, 1, {"0x10"}, true, 16},
    // Invalid number formats
    {LONG_MIN, LONG_MAX, 0, 1, {"2.999"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"13E2"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"1300e-1"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"x13"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"13x"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"1+1"}, false, 0},
    // These should never happen unless argument parsing is broken
    {LONG_MIN, LONG_MAX, 0, 1, {" 13"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {" -13"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"13 "}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"-13 "}, false, 0},
    // Extreme values
    {LONG_MIN, LONG_MAX, 0, 1, {"-2147483648"}, true, -2147483648},
    {LONG_MIN, LONG_MAX, 0, 1, {"2147483647"}, true, 2147483647},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    long int value = 123456;
    assert(cp->argc <= (int)ARRAY_SIZE(cp->argv));

    printf(
      "Expecting %s (%ld) from argument %d of %d (%s) with range %ld,%ld\n",
      cp->success ? "success" : "failure", cp->value, cp->n, cp->argc,
      cp->n < cp->argc ? cp->argv[cp->n] : "?", cp->min, cp->max);

    bool const success =
      get_long_arg("test", &value, cp->min, cp->max, cp->argc, cp->argv, cp->n);

    assert(success == cp->success);
    if (success)
    {
      assert(value == cp->value);
    }
  }
}

static void test2(void)
{
  /* Floating point argument parsing */
  typedef const struct
  {
    double min, max;
    int n, argc;
    const char *argv[10];
    bool success;
    double value;
  } TestCase;

  static TestCase cases[] = {
    // Argument selection
    {-DBL_MAX, DBL_MAX, 0, 1, {"23"}, true, 23},
    {-DBL_MAX, DBL_MAX, 1, 2, {"5", "10"}, true, 10},
    {-DBL_MAX,
     DBL_MAX,
     4,
     7,
     {"1", "67", "43", "2", "444", "65", "6"},
     true,
     444},
    // Invalid argument index (treated as missing argument)
    {-DBL_MAX, DBL_MAX, 1, 1, {"23"}, false, 0},
    // Value out of range
    {1, 2, 0, 1, {"0"}, false, 0},
    {0, 1, 0, 1, {"2"}, false, 0},
    // Switch in place of expected numeric argument
    {-DBL_MAX, DBL_MAX, 0, 1, {"-foo"}, false, 0},
    {-DBL_MAX, DBL_MAX, 0, 1, {"- 1"}, false, 0},
    // Interesting number formats
    {-DBL_MAX, DBL_MAX, 0, 1, {"010"}, true, 8},
    {-DBL_MAX, DBL_MAX, 0, 1, {"0x10"}, true, 16},
    {-DBL_MAX, DBL_MAX, 0, 1, {"2.999"}, true, 2.999},
    {-DBL_MAX, DBL_MAX, 0, 1, {"13E2"}, true, 1300},
    {-DBL_MAX, DBL_MAX, 0, 1, {"1300e-1"}, true, 130},
    // Invalid number formats
    {-DBL_MAX, DBL_MAX, 0, 1, {"x13"}, false, 0},
    {-DBL_MAX, DBL_MAX, 0, 1, {"13x"}, false, 0},
    {-DBL_MAX, DBL_MAX, 0, 1, {"1+1"}, false, 0},
    // These should never happen unless argument parsing is broken
    {LONG_MIN, LONG_MAX, 0, 1, {" 13"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {" -13"}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"13 "}, false, 0},
    {LONG_MIN, LONG_MAX, 0, 1, {"-13 "}, false, 0},
    // Extreme values
    {-DBL_MAX, DBL_MAX, 0, 1, {"-1.79769e+308"}, true, -1.79769e+308},
    {-DBL_MAX, DBL_MAX, 0, 1, {"1.79769e+308"}, true, 1.79769e+308},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    double value = 123.456;
    assert(cp->argc <= (int)ARRAY_SIZE(cp->argv));

    printf("Expecting %s (%g) from argument %d of %d (%s) with range %g,%g\n",
           cp->success ? "success" : "failure", cp->value, cp->n, cp->argc,
           cp->n < cp->argc ? cp->argv[cp->n] : "?", cp->min, cp->max);

    bool const success = get_double_arg("test", &value, cp->min, cp->max,
                                        cp->argc, cp->argv, cp->n);

    assert(success == cp->success);
    if (success)
    {
      assert(fabs(value - cp->value) - DBL_EPSILON);
    }
  }
}

static void test3(void)
{
  /* Switch argument parsing */
  typedef const struct
  {
    const char *arg, *name;
    size_t min;
    bool success;
  } TestCase;

  static TestCase cases[] = {
    {"", "", 1, false},
    // Whole option name matches
    {"Tiger", "Tiger", 5, true},
    // Comparisons are case-sensitive
    {"tiger", "TIGER", 3, false},
    {"TIGER", "tiger", 3, false},
    // When an abbreviated option name is acceptable, it is not required
    {"Tiger", "Tiger", 3, true},
    // When an abbreviated option name is acceptable, it is accepted
    {"Tig", "Tiger", 3, true},
    // When a whole option name matches, the remainder is not ignored
    {"Tiger, tiger, burning bright", "Tiger", 5, false},
    // When an abbreviated option name matches, the remainder is not ignored
    {"Tigress", "Tiger", 3, false},
    // When the abbreviated form is too long, it does not match
    {"Tiger", "Tiger", 6, false}, // questionable
    // These should never happen unless argument parsing is broken
    {" tiger", "tiger", 5, false},
    {"tiger ", "tiger", 5, false},
  };

  FOR_EACH_ELEM_PTR(TestCase *, cp, cases)
  {
    printf(
      "Expecting %s from argument %s matching at least %zu characters of %s\n",
      cp->success ? "success" : "failure", cp->arg, cp->min, cp->name);

    bool const success = is_switch(cp->arg, cp->name, cp->min);
    assert(success == cp->success);
  }
}

void ArgUtils_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Long integer argument parsing", test1},
    {"Floating point argument parsing", test2},
    {"Switch argument parsing", test3},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
