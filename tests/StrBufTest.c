/*
 * CBUtilLib test: String buffer
 * Copyright (C) 2012 Christopher Bazley
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* CBUtilLib headers */
#include "StringBuff.h"

/* Local headers */
#include "Tests.h"

enum
{
  NumberOfAppends = 10,
  NumberOfBuffers = 10
};

static void test1(void)
{
  /* Init/destroy */
  unsigned int i;
  StringBuffer buffer[NumberOfBuffers];

  for (i = 0; i < ARRAY_SIZE(buffer); i++)
    stringbuffer_init(buffer + i);

  for (i = 0; i < ARRAY_SIZE(buffer); i++)
  {
    const size_t len = stringbuffer_get_length(buffer + i);
    const char *s = stringbuffer_get_pointer(buffer + i);
    assert(len == 0);
    assert(s != NULL);
    assert(*s == '\0');
  }

  for (i = 0; i < ARRAY_SIZE(buffer); i++)
    stringbuffer_destroy(buffer + i);
}

static void test2(void)
{
  /* Append strings */
  StringBuffer buffer;
  unsigned int i;
  size_t expected_len = 0;
  static const char *tail[NumberOfAppends] =
  {
    "Appends", "up", "to", "", "\tcharacters", "from", "'tail'",
    "at", "the\r", "end\n"
  };
  char expected_s[NumberOfAppends * 16] = "";

  stringbuffer_init(&buffer);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const bool success = stringbuffer_append_all(&buffer, tail[i]);
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);
    assert(success);

    expected_len += strlen(tail[i]);
    assert(len == expected_len);

    assert(s != NULL);
    strcat(expected_s, tail[i]);
    assert(strcmp(s, expected_s) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test3(void)
{
  /* Append substrings */
  StringBuffer buffer;
  size_t i, expected_len = 0;
  static const char *tail = "The boy stood on the burning deck whence all but he had fled";
  char expected_s[128] = "";

  stringbuffer_init(&buffer);

  for (i = 0; i < NumberOfAppends; i++)
  {
    const bool success = stringbuffer_append(&buffer, tail, i);
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);

    assert(success);

    expected_len += i;
    assert(len == expected_len);

    assert(s != NULL);
    strncat(expected_s, tail, i);
    assert(strcmp(s, expected_s) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test4(void)
{
  /* Append NULL */
  StringBuffer buffer;

  stringbuffer_init(&buffer);

  {
    const bool success = stringbuffer_append(&buffer, NULL, 0);
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);

    assert(success);

    assert(len == 0);

    assert(s != NULL);
    assert(strcmp(s, "") == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test5(void)
{
  /* Truncate */
  StringBuffer buffer;
  bool success;
  size_t i;
  char string[] = "The boy stood on the burning deck whence all but he had fled";

  stringbuffer_init(&buffer);

  /* Truncate empty */
  stringbuffer_truncate(&buffer, 0);

  success = stringbuffer_append_all(&buffer, string);
  assert(success);

  i = NumberOfAppends;
  while (i-- > 0)
  {
    size_t len;
    const char *s;

    stringbuffer_truncate(&buffer, i);

    len = stringbuffer_get_length(&buffer);
    assert(len == i);

    s = stringbuffer_get_pointer(&buffer);
    assert(s != NULL);
    string[i] = '\0';
    assert(strcmp(s, string) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test6(void)
{
  /* Minimize after append */
  StringBuffer buffer;
  unsigned int i;
  size_t expected_len = 0;
  static const char *tail[NumberOfAppends] =
  {
    "Appends", "up", "to", "", "\tcharacters", "from", "'tail'",
    "at", "the\r", "end\n"
  };
  char expected_s[NumberOfAppends * 16] = "";

  stringbuffer_init(&buffer);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const bool success = stringbuffer_append_all(&buffer, tail[i]);
    size_t len;
    const char *s;

    assert(success);
    stringbuffer_minimize(&buffer);

    len = stringbuffer_get_length(&buffer);
    expected_len += strlen(tail[i]);
    assert(len == expected_len);

    s = stringbuffer_get_pointer(&buffer);
    assert(s != NULL);
    strcat(expected_s, tail[i]);
    assert(strcmp(s, expected_s) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test7(void)
{
  /* Minimize after truncate */
  StringBuffer buffer;
  bool success;
  size_t i, len;
  const char *s;
  char string[] = "The boy stood on the burning deck whence all but he had fled";

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, string);
  assert(success);

  i = NumberOfAppends;
  while (i-- > 0)
  {

    stringbuffer_truncate(&buffer, i);
    stringbuffer_minimize(&buffer);

    len = stringbuffer_get_length(&buffer);
    assert(len == i);

    s = stringbuffer_get_pointer(&buffer);
    assert(s != NULL);
    string[i] = '\0';
    assert(strcmp(s, string) == 0);
  }

  /* Append after minimizing length 0 */
  success = stringbuffer_append_all(&buffer, "foo");
  assert(success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 3);

  s = stringbuffer_get_pointer(&buffer);
  assert(s != NULL);
  assert(strcmp(s, "foo") == 0);

  stringbuffer_destroy(&buffer);
}

static void test8(void)
{
  /* Append fail recovery */
  StringBuffer buffer;
  bool success;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foo");
  assert(success);

  Fortify_SetAllocationLimit(0);
  success = stringbuffer_append_all(&buffer, "bar");
  Fortify_SetAllocationLimit(ULONG_MAX);
  assert(!success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 3);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foo") == 0);

  success = stringbuffer_append_all(&buffer, "bar");
  assert(success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 6);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foobar") == 0);

  stringbuffer_destroy(&buffer);
}

static void test9(void)
{
  /* Minimize fail recovery */
  StringBuffer buffer;
  bool success;
  size_t len;
  const char *s, *string = "Man's life is as cheap as beast's";
  const size_t trunc_len = strlen(string) * 2 / 3;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, string);
  assert(success);

  stringbuffer_truncate(&buffer, trunc_len);

  Fortify_SetAllocationLimit(0);
  stringbuffer_minimize(&buffer);
  Fortify_SetAllocationLimit(ULONG_MAX);

  len = stringbuffer_get_length(&buffer);
  assert(len == trunc_len);
  s = stringbuffer_get_pointer(&buffer);
  assert(strncmp(s, string, trunc_len) == 0);
  assert(s[trunc_len] == '\0');

  success = stringbuffer_append_all(&buffer, string + trunc_len);
  assert(success);

  len = stringbuffer_get_length(&buffer);
  assert(len == strlen(string));
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, string) == 0);

  stringbuffer_destroy(&buffer);
}

static void test10(void)
{
  /* Undo append */
  StringBuffer buffer;
  unsigned int i;
  bool success;
  const char *stem = "Podd can ";
  static const char *tail[NumberOfAppends] =
  {
    "eat", "dance", "snore", "swim", "walk", "run", "yawn", "pop", "sleep", "stroll"
  };

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, stem);
  assert(success);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);

    assert(len == strlen(stem));
    assert(strcmp(s, stem) == 0);

    success = stringbuffer_append_all(&buffer, tail[i]);
    assert(success);

    stringbuffer_undo(&buffer);
  }

  stringbuffer_destroy(&buffer);
}

static void test11(void)
{
  /* Undo truncate */
  StringBuffer buffer;
  bool success;
  size_t i;
  const char *string = "The boy stood on the burning deck whence all but he had fled";

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, string);
  assert(success);

  i = NumberOfAppends;
  while (i-- > 0)
  {
    size_t len;
    const char *s;

    len = stringbuffer_get_length(&buffer);
    assert(len == strlen(string));

    s = stringbuffer_get_pointer(&buffer);
    assert(s != NULL);
    assert(strcmp(s, string) == 0);

    stringbuffer_truncate(&buffer, i);

    stringbuffer_undo(&buffer);
  }

  stringbuffer_destroy(&buffer);
}

static void test12(void)
{
  /* Undo append after minimize */
  bool success;
  StringBuffer buffer;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foo");
  assert(success);

  success = stringbuffer_append_all(&buffer, "bar");
  assert(success);

  stringbuffer_minimize(&buffer);
  stringbuffer_undo(&buffer); /* no effect */

  len = stringbuffer_get_length(&buffer);
  assert(len == strlen("foobar"));
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foobar") == 0);

  stringbuffer_destroy(&buffer);
}

static void test13(void)
{
  /* Undo truncate after minimize */
  bool success;
  StringBuffer buffer;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foobar");
  assert(success);

  stringbuffer_truncate(&buffer, strlen("foo"));

  stringbuffer_minimize(&buffer);
  stringbuffer_undo(&buffer); /* no effect */

  len = stringbuffer_get_length(&buffer);
  assert(len == strlen("foo"));
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foo") == 0);

  stringbuffer_destroy(&buffer);
}

static void test14(void)
{
  /* Undo append twice */
  bool success;
  StringBuffer buffer;
  unsigned int i;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foo");
  assert(success);

  success = stringbuffer_append_all(&buffer, "bar");
  assert(success);

  for (i = 0; i < 2; ++i)
  {
    size_t len;
    const char *s;

    stringbuffer_undo(&buffer);

    len = stringbuffer_get_length(&buffer);
    assert(len == strlen("foo"));

    s = stringbuffer_get_pointer(&buffer);
    assert(strcmp(s, "foo") == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test15(void)
{
  /* Undo truncate twice */
  bool success;
  StringBuffer buffer;
  unsigned int i;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foobar");
  assert(success);

  stringbuffer_truncate(&buffer, strlen("foo"));
  stringbuffer_truncate(&buffer, 0);

  for (i = 0; i < 2; ++i)
  {
    size_t len;
    const char *s;

    stringbuffer_undo(&buffer);

    len = stringbuffer_get_length(&buffer);
    assert(len == strlen("foo"));

    s = stringbuffer_get_pointer(&buffer);
    assert(strcmp(s, "foo") == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test16(void)
{
  /* Undo append no chars */
  bool success;
  StringBuffer buffer;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foo");
  assert(success);

  success = stringbuffer_append_all(&buffer, "");
  assert(success);

  stringbuffer_undo(&buffer); /* should have no effect */

  len = stringbuffer_get_length(&buffer);
  assert(len == strlen("foo"));

  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foo") == 0);

  stringbuffer_destroy(&buffer);
}

static void test17(void)
{
  /* Undo truncate no chars */
  bool success;
  StringBuffer buffer;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_append_all(&buffer, "foobar");
  assert(success);

  stringbuffer_truncate(&buffer, strlen("foo"));
  stringbuffer_truncate(&buffer, strlen("foo"));

  stringbuffer_undo(&buffer); /* should have no effect */

  len = stringbuffer_get_length(&buffer);
  assert(len == strlen("foo"));

  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foo") == 0);

  stringbuffer_destroy(&buffer);

}

static void test18(void)
{
  /* Append separated strings */
  StringBuffer buffer;
  unsigned int i;
  size_t expected_len = 0;
  static const char *tail[NumberOfAppends] =
  {
    "Appends", "up", "to", "", "\tcharacters", "from", "'tail'",
    "at", "the\r", "end\n"
  };
  const char sep = '$';
  char expected_s[NumberOfAppends * 16] = "";

  stringbuffer_init(&buffer);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const bool success = stringbuffer_append_separated(&buffer, sep, tail[i]);
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);
    assert(success);

    expected_len += 1 + strlen(tail[i]);
    assert(len == expected_len);

    assert(s != NULL);
    char ssep[] = {sep, '\0'};
    strcat(expected_s, ssep);
    strcat(expected_s, tail[i]);
    assert(strcmp(s, expected_s) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test19(void)
{
  /* Undo append separated */
  StringBuffer buffer;
  unsigned int i;
  bool success;
  const char *stem = "Podd can ";
  static const char *tail[NumberOfAppends] =
  {
    "eat", "dance", "snore", "swim", "walk", "run", "yawn", "pop", "sleep", "stroll"
  };
  const char sep = ',';

  stringbuffer_init(&buffer);

  success = stringbuffer_append_separated(&buffer, sep, stem);
  assert(success);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const size_t len = stringbuffer_get_length(&buffer);
    const char *s = stringbuffer_get_pointer(&buffer);

    assert(len == 1 + strlen(stem));
    assert(*s == sep);
    assert(strcmp(s + 1, stem) == 0);

    success = stringbuffer_append_separated(&buffer, sep, tail[i]);
    assert(success);

    stringbuffer_undo(&buffer);
  }

  stringbuffer_destroy(&buffer);
}

static void test20(void)
{
  /* Minimize after append separated */
  StringBuffer buffer;
  unsigned int i;
  size_t expected_len = 0;
  static const char *tail[NumberOfAppends] =
  {
    "Appends", "up", "to", "", "\tcharacters", "from", "'tail'",
    "at", "the\r", "end\n"
  };
  const char sep = '\t';
  char expected_s[NumberOfAppends * 16] = "";

  stringbuffer_init(&buffer);

  for (i = 0; i < ARRAY_SIZE(tail); i++)
  {
    const bool success = stringbuffer_append_separated(&buffer, sep, tail[i]);
    size_t len;
    const char *s;

    assert(success);
    stringbuffer_minimize(&buffer);

    len = stringbuffer_get_length(&buffer);
    expected_len += 1 + strlen(tail[i]);
    assert(len == expected_len);

    s = stringbuffer_get_pointer(&buffer);
    assert(s != NULL);
    char ssep[] = {sep, '\0'};
    strcat(expected_s, ssep);
    strcat(expected_s, tail[i]);
    assert(strcmp(s, expected_s) == 0);
  }

  stringbuffer_destroy(&buffer);
}

static void test21(void)
{
  /* Append separated fail recovery */
  StringBuffer buffer;
  bool success;
  size_t len;
  const char *s;
  const char sep = '.';

  stringbuffer_init(&buffer);

  success = stringbuffer_append_separated(&buffer, sep, "foo");
  assert(success);

  Fortify_SetAllocationLimit(0);
  success = stringbuffer_append_separated(&buffer, sep, "bar");
  Fortify_SetAllocationLimit(ULONG_MAX);
  assert(!success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 4);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, ".foo") == 0);

  success = stringbuffer_append_separated(&buffer, sep, "bar");
  assert(success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 8);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, ".foo.bar") == 0);

  stringbuffer_destroy(&buffer);
}

static void test22(void)
{
  /* Append formatted string */
  StringBuffer buffer;

  stringbuffer_init(&buffer);

  for (int c = 'a'; c <= 'z'; ++c)
  {
    bool success = stringbuffer_printf(&buffer, "%c", c);
    assert(success);
  }

  size_t len = stringbuffer_get_length(&buffer);
  assert(len == 'z' - 'a' + 1);
  char const *s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "abcdefghijklmnopqrstuvwxyz") == 0);

  stringbuffer_undo(&buffer);

  size_t len2 = stringbuffer_get_length(&buffer);
  assert(len2 == len - 1);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "abcdefghijklmnopqrstuvwxy") == 0);

  stringbuffer_destroy(&buffer);
}

static void test23(void)
{
  /* Append fail recovery */
  StringBuffer buffer;
  bool success;
  size_t len;
  const char *s;

  stringbuffer_init(&buffer);

  success = stringbuffer_printf(&buffer, "%s", "foo");
  assert(success);

  Fortify_SetAllocationLimit(0);
  success = stringbuffer_printf(&buffer, "%s", "bar");
  Fortify_SetAllocationLimit(ULONG_MAX);
  assert(!success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 3);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foo") == 0);

  success = stringbuffer_printf(&buffer, "%s", "bar");
  assert(success);

  len = stringbuffer_get_length(&buffer);
  assert(len == 6);
  s = stringbuffer_get_pointer(&buffer);
  assert(strcmp(s, "foobar") == 0);

  stringbuffer_destroy(&buffer);
}

void StringBuffer_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  }
  unit_tests[] =
  {
    { "Init/destroy", test1 },
    { "Append strings", test2 },
    { "Append substrings", test3 },
    { "Append NULL", test4 },
    { "Truncate", test5 },
    { "Minimize after append", test6 },
    { "Minimize after truncate", test7 },
    { "Append fail recovery", test8 },
    { "Minimize fail recovery", test9 },
    { "Undo append", test10 },
    { "Undo truncate", test11 },
    { "Undo append after minimize", test12 },
    { "Undo truncate after minimize", test13 },
    { "Undo append twice", test14 },
    { "Undo truncate twice", test15 },
    { "Undo append no chars", test16 },
    { "Undo truncate no chars", test17 },
    { "Append separated", test18 },
    { "Undo append separated", test19 },
    { "Minimize after append separated", test20 },
    { "Append separated fail recovery", test21 },
    { "Append formatted", test22 },
    { "Append formatted fail recovery", test23 },
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count ++)
  {
    printf("Test %zu/%zu : %s\n",
           1 + count,
           ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);

    Fortify_EnterScope();

    unit_tests[count].test_func();

    Fortify_LeaveScope();
  }
}
