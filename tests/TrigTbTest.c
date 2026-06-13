/*
 * CBUtilLib test: Trigonometric tables
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
#include <math.h>
#include <stdio.h>

/* CBUtilLib headers */
#include "TrigTable.h"

/* Local headers */
#include "Tests.h"

#define MAX_ERROR (0.5)

static const int multipliers[] = {100, CHAR_MAX, 1000, 1024, SHRT_MAX, INT_MAX},
                 quarter_turns[] = {1, 256, 512};

static void test1(void)
{
  /* Init/destroy */
  TrigTable *tt[ARRAY_SIZE(multipliers) * ARRAY_SIZE(quarter_turns)];

  for (size_t i = 0; i < ARRAY_SIZE(tt); i++)
  {
    _Optional TrigTable *table =
      TrigTable_make(multipliers[i % ARRAY_SIZE(multipliers)],
                     quarter_turns[i / ARRAY_SIZE(multipliers)]);
    assert(tt);
    tt[i] = &*table;

    for (size_t j = 0; j < i; j++)
    {
      assert(table != tt[j]);
    }
  }

  for (size_t i = 0; i < ARRAY_SIZE(tt); i++)
  {
    const int cos = TrigTable_look_up_cosine(tt[i], 0),
              sin = TrigTable_look_up_sine(tt[i], 0);
    assert(cos == multipliers[i % ARRAY_SIZE(multipliers)]);
    assert(sin == 0);
  }

  for (size_t i = 0; i < ARRAY_SIZE(tt); i++)
  {
    TrigTable_destroy(tt[i]);
  }
}

static void test2(void)
{
  TrigTable_destroy(NULL);
}

static void test3(void)
{
#ifdef FORTIFY
  /* Init fail */

  Fortify_SetAllocationLimit(0);
  _Optional TrigTable *tt = TrigTable_make(multipliers[0], quarter_turns[0]);
  Fortify_SetAllocationLimit(ULONG_MAX);
  assert(!tt);

  tt = TrigTable_make(multipliers[0], quarter_turns[0]);
  assert(tt);

  const int cos = TrigTable_look_up_cosine(&*tt, 0),
            sin = TrigTable_look_up_sine(&*tt, 0);
  assert(cos == 1);
  assert(sin == 0);

  TrigTable_destroy(tt);
#endif
}

static void test4(void)
{
  /* Sine */
  FOR_EACH_ELEM_PTR(const int *, m, multipliers)
  {
    double max_error = 0;
    const double allowed_error = MAX_ERROR / *m;

    FOR_EACH_ELEM_PTR(const int *, q, quarter_turns)
    {
      _Optional TrigTable *tt = TrigTable_make(*m, *q);
      assert(tt);

      const int full_turn = *q * 4;

      for (int a = -full_turn; a < full_turn; ++a)
      {
        const int sine = TrigTable_look_up_sine(&*tt, a);

        const double sine2 = (double)sine / *m,
                     expected = sin(2 * acos(-1.0) * a / full_turn),
                     error = fabs(sine2 - expected);

        if (error > max_error)
        {
          max_error = error;
        }

        printf("Angle: %d Sine: %d (%g) Expected: %g Error: %g Allowed: %g\n",
               a, sine, sine2, expected, error, allowed_error);

        assert(error <= allowed_error);
      }
      TrigTable_destroy(tt);
    }
    printf("Max error: %g\n", max_error);
  }
}

static void test5(void)
{
  /* Cosine */
  FOR_EACH_ELEM_PTR(const int *, m, multipliers)
  {
    double max_error = 0;
    const double allowed_error = MAX_ERROR / *m;

    FOR_EACH_ELEM_PTR(const int *, q, quarter_turns)
    {
      _Optional TrigTable *tt = TrigTable_make(*m, *q);
      assert(tt);

      const int full_turn = *q * 4;
      for (int a = -full_turn; a < full_turn; ++a)
      {
        const int cosine = TrigTable_look_up_cosine(&*tt, a);

        const double cosine2 = (double)cosine / *m,
                     expected = cos(2 * acos(-1.0) * a / full_turn),
                     error = fabs(cosine2 - expected);

        if (error > max_error)
        {
          max_error = error;
        }

        printf("Angle: %d Cosine: %d (%g) Expected: %g Error: %g Allowed %g\n",
               a, cosine, cosine2, expected, error, allowed_error);

        assert(error <= allowed_error);
      }
      TrigTable_destroy(tt);
    }
    printf("Max error: %g\n", max_error);
  }
}

void TrigTable_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Init/destroy", test1}, {"Destroy null", test2}, {"Init fail", test3},
    {"Sine", test4},         {"Cosine", test5},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    unit_tests[count].test_func();
  }
}
