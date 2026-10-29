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

/* History:
  CJB: 28-Oct-26: Bundle argument counts and vectors using the ARGUMENTS
                  macro and exactly sized compound literals. Keep the vectors
                  and case tables static and const, without null padding.
                  Reorder case members to reduce structure padding.
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

/* Counted non-null argument vectors; the parser does not use an argv sentinel.
   Use at file scope so both the vectors and their case tables have static
   storage duration. sizeof computes the count without evaluating a vector. */
typedef struct
{
  int argc;
  const char *const *argv;
} Arguments;

#define ARGUMENTS(...)                                                   \
  {                                                                      \
    .argc = (int)(sizeof((const char *const[]){__VA_ARGS__}) /              \
                  sizeof(const char *)),                                 \
    .argv = (const char *const[]){__VA_ARGS__},                            \
  }

/* Long integer argument parsing */
typedef struct
{
  long int min, max, value;
  Arguments args;
  int n;
  bool success;
} LongArgTestCase;

static const LongArgTestCase long_arg_cases[] = {
  // Argument selection
  {LONG_MIN, LONG_MAX, 23, ARGUMENTS("23"), 0, true},
  {LONG_MIN, LONG_MAX, 10, ARGUMENTS("5", "10"), 1, true},
  {LONG_MIN, LONG_MAX, 444,
   ARGUMENTS("1", "67", "43", "2", "444", "65", "6"), 4, true},
  // Invalid argument index (treated as missing argument)
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("23"), 1, false},
  // Value out of range
  {1, 2, 0, ARGUMENTS("0"), 0, false},
  {0, 1, 0, ARGUMENTS("2"), 0, false},
  // Switch in place of expected numeric argument
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("-foo"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("- 1"), 0, false},
  // Interesting number formats
  {LONG_MIN, LONG_MAX, 8, ARGUMENTS("010"), 0, true},
  {LONG_MIN, LONG_MAX, 16, ARGUMENTS("0x10"), 0, true},
  // Invalid number formats
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("2.999"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("13E2"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("1300e-1"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("x13"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("13x"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("1+1"), 0, false},
  // These should never happen unless argument parsing is broken
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS(" 13"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS(" -13"), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("13 "), 0, false},
  {LONG_MIN, LONG_MAX, 0, ARGUMENTS("-13 "), 0, false},
  // Extreme values
  {LONG_MIN, LONG_MAX, -2147483647L - 1, ARGUMENTS("-2147483648"), 0, true},
  {LONG_MIN, LONG_MAX, 2147483647, ARGUMENTS("2147483647"), 0, true},
};

static void test1(void)
{
  FOR_EACH_ELEM_PTR(const LongArgTestCase *, cp, long_arg_cases)
  {
    long int value = 123456;

    printf(
      "Expecting %s (%ld) from argument %d of %d (%s) with range %ld,%ld\n",
      cp->success ? "success" : "failure", cp->value, cp->n, cp->args.argc,
      cp->n < cp->args.argc ? cp->args.argv[cp->n] : "?", cp->min, cp->max);

    bool const success =
      get_long_arg("test", &value, cp->min, cp->max,
                   cp->args.argc, cp->args.argv, cp->n);

    assert(success == cp->success);
    if (success)
    {
      assert(value == cp->value);
    }
  }
}

/* Floating point argument parsing */
typedef struct
{
  double min, max, value;
  Arguments args;
  int n;
  bool success;
} DoubleArgTestCase;

static const DoubleArgTestCase double_arg_cases[] = {
  // Argument selection
  {-DBL_MAX, DBL_MAX, 23, ARGUMENTS("23"), 0, true},
  {-DBL_MAX, DBL_MAX, 10, ARGUMENTS("5", "10"), 1, true},
  {-DBL_MAX, DBL_MAX, 444,
   ARGUMENTS("1", "67", "43", "2", "444", "65", "6"), 4, true},
  // Invalid argument index (treated as missing argument)
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("23"), 1, false},
  // Value out of range
  {1, 2, 0, ARGUMENTS("0"), 0, false},
  {0, 1, 0, ARGUMENTS("2"), 0, false},
  // Switch in place of expected numeric argument
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("-foo"), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("- 1"), 0, false},
  // Interesting number formats
  {-DBL_MAX, DBL_MAX, 8, ARGUMENTS("010"), 0, true},
  {-DBL_MAX, DBL_MAX, 16, ARGUMENTS("0x10"), 0, true},
  {-DBL_MAX, DBL_MAX, 2.999, ARGUMENTS("2.999"), 0, true},
  {-DBL_MAX, DBL_MAX, 1300, ARGUMENTS("13E2"), 0, true},
  {-DBL_MAX, DBL_MAX, 130, ARGUMENTS("1300e-1"), 0, true},
  // Invalid number formats
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("x13"), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("13x"), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("1+1"), 0, false},
  // These should never happen unless argument parsing is broken
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS(" 13"), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS(" -13"), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("13 "), 0, false},
  {-DBL_MAX, DBL_MAX, 0, ARGUMENTS("-13 "), 0, false},
  // Extreme values
  {-DBL_MAX, DBL_MAX, -1.79769e+308, ARGUMENTS("-1.79769e+308"), 0, true},
  {-DBL_MAX, DBL_MAX, 1.79769e+308, ARGUMENTS("1.79769e+308"), 0, true},
};

static void test2(void)
{
  FOR_EACH_ELEM_PTR(const DoubleArgTestCase *, cp, double_arg_cases)
  {
    double value = 123.456;

    printf("Expecting %s (%g) from argument %d of %d (%s) with range %g,%g\n",
           cp->success ? "success" : "failure", cp->value, cp->n, cp->args.argc,
           cp->n < cp->args.argc ? cp->args.argv[cp->n] : "?", cp->min, cp->max);

    bool const success = get_double_arg("test", &value, cp->min, cp->max,
                                        cp->args.argc, cp->args.argv, cp->n);

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
