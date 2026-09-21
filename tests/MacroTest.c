/*
 * CBUtilLib test: Generally useful macro definitions
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
  CJB: 21-Sep-26: Move the generic macro tests from CBLibrary and extend them
                  to cover the definitions now exported by CBUtilLib.
*/

#include <stdlib.h>
#include <string.h>

#include "MacroUtils.h"
#include "Tests.h"

static _Optional const char *null_string = NULL;

static void test_values(void)
{
  int values[] = {1, 2, 3};
  assert(ARRAY_SIZE(values) == 3);
  assert(WORD_ALIGN(5) == 8);
  assert(WORD_ALIGN_SZ(8) == 8);
  assert(SCALE(50, 20) == 10);
  assert(LOWEST(2, 3) == 2);
  assert(HIGHEST(2, 3) == 3);
  assert(CLAMP(5, 1, 4) == 4);
  assert(SIGNED_R_SHIFT(8, 1) == 4);
  assert(SIGNED_R_SHIFT(4, -1) == 8);
  assert(SIGNED_L_SHIFT(4, 1) == 8);
  assert(SIGNED_L_SHIFT(8, -1) == 4);
  assert(strcmp(STRINGIFY(test), "test") == 0);

  int a = 1;
  int b = 2;
  SWAP(a, b);
  assert(a == 2);
  assert(b == 1);

  unsigned int flags = 1u;
  SET_BITS(flags, 2u);
  assert(TEST_BITS(flags, 3u));
  CLEAR_BITS(flags, 1u);
  assert(flags == 2u);

  unsigned int difference;
  ABSDIFF(difference, 3u, 8u);
  assert(difference == 5u);
}

static void test_strings(void)
{
  char buffer[4];
  STRCPY_SAFE(buffer, "string");
  assert(strcmp(buffer, "str") == 0);

  assert(strcmp(STRING_OR_NULL("text"), "text") == 0);
  assert(strcmp(STRING_OR_NULL(null_string), "") == 0);
}

static void test_allocation(void)
{
  _Optional void *block = malloc(1);
  assert(block != NULL);
  FREE_SAFE(block);
  assert(block == NULL);
}

static void test_container_of(void)
{
  struct container {
    int member;
  } containers[2] = {{1}, {2}};
  int *members[] = {&containers[0].member, &containers[1].member};
  int **next = members;

  struct container *const container =
    CONTAINER_OF(*next++, struct container, member);
  assert(container == &containers[0]);
  assert(next == &members[1]);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
  const struct container const_container = {3};
  const struct container *const recovered =
    CONTAINER_OF(&const_container.member, const struct container, member);
  assert(recovered == &const_container);
#endif
}

void MacroUtils_tests(void)
{
  test_values();
  test_strings();
  test_allocation();
  test_container_of();
}
