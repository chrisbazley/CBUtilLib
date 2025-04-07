/*
 * CBUtilLib test: String dictionary
 * Copyright (C) 2022 Christopher Bazley
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

/* CBUtilLib headers */
#include "StrExtra.h"
#include "StrDict.h"

/* Local headers */
#include "Tests.h"

enum
{
  NumberOfItems = 6,
  MiddleDivider = 2,
  NumberOfDuplicates = 4,
  NumberOfKeys = 2,
  FortifyAllocationLimit = 100,
  RemoveInterval = 2,
  MagicValue = 1234,
};

static struct CBInfo
{
  const char *key;
  _Optional void *value;
  _Optional void *arg;
}
callbacks[NumberOfItems * NumberOfDuplicates];

static size_t callback_count;

static void record_callbacks(const char *key, _Optional void *value, _Optional void *arg)
{
  assert(callback_count < ARRAY_SIZE(callbacks));
  DEBUGF("Callback %zu: key %p (%s), value %p, arg %p\n", callback_count, (void *)key, key, value, arg);
  callbacks[callback_count++] = (struct CBInfo){.key = key, .value = value, .arg = arg};
}

static void never_call_me(const char *key, _Optional void *value, _Optional void *arg)
{
  NOT_USED(key);
  NOT_USED(value);
  NOT_USED(arg);
  assert("Dictionary isn't empty" == NULL);
}

static char *test_strdup(const char *const s)
{
  _Optional char *const dup = strdup(s);
  assert(dup);
  return &*dup;
}

typedef void remove_t(StrDict *, char const *, _Optional void *, size_t);
typedef void insert_t(StrDict *, char const *, _Optional void *, size_t);

static void remove_key_only(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  NOT_USED(value);
  size_t rem_pos = MagicValue;
  bool success = strdict_remove(dict, dup_key, &rem_pos);
  if (pos != SIZE_MAX) {
    assert(success);
    assert(rem_pos == pos);
  } else {
    assert(!success);
  }
  free(dup_key);
}

static void remove_key_only_no_pos(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  NOT_USED(value);
  bool success = strdict_remove(dict, dup_key, NULL);
  if (pos != SIZE_MAX) {
    assert(success);
  } else {
    assert(!success);
  }
  free(dup_key);
}

static void remove_key_and_get_value(StrDict *const dict, char const *const key, _Optional void *value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  if (pos == SIZE_MAX) {
    value = NULL;
  }
  size_t rem_pos = MagicValue;
  assert(strdict_remove_value(dict, dup_key, &rem_pos) == value);
  if (pos != SIZE_MAX) {
    assert(rem_pos == pos);
  }
  free(dup_key);
}

static void remove_key_and_get_value_no_pos(StrDict *const dict, char const *const key, _Optional void *value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  if (pos == SIZE_MAX) {
    value = NULL;
  }
  assert(strdict_remove_value(dict, dup_key, NULL) == value);
  free(dup_key);
}

static void remove_specific(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  size_t rem_pos = MagicValue;
  bool success = strdict_remove_specific(dict, dup_key, value, &rem_pos);
  if (pos != SIZE_MAX) {
    assert(success);
    assert(rem_pos == pos);
  } else {
    assert(!success);
  }
  free(dup_key);
}

static void remove_specific_no_pos(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  bool success = strdict_remove_specific(dict, dup_key, value, NULL);
  if (pos != SIZE_MAX) {
    assert(success);
  } else {
    assert(!success);
  }
  free(dup_key);
}

static void remove_index(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  NOT_USED(key);
  NOT_USED(value);
  if (pos != SIZE_MAX) {
    strdict_remove_at(dict, pos);
  }
}

static void remove_index_and_get_value(StrDict *const dict, char const *const key, _Optional void *value, size_t const pos)
{
  NOT_USED(key);
  if (pos != SIZE_MAX) {
    assert(strdict_remove_value_at(dict, pos) == value);
  }
}

static void check_find(StrDict *const dict, char const *key, _Optional void *const value, size_t const pos)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);
  assert(strdict_find(dict, dup_key, NULL) == true);

  size_t found = MagicValue;
  assert(strdict_find(dict, dup_key, &found) == true);
  assert(found == pos);

  assert(strdict_find_value(dict, dup_key, NULL) == value);

  found = MagicValue;
  assert(strdict_find_value(dict, dup_key, &found) == value);
  assert(found == pos);

  assert(strdict_find_specific(dict, dup_key, value, NULL) == true);

  found = MagicValue;
  assert(strdict_find_specific(dict, dup_key, value, &found) == true);
  assert(found == pos);

  assert(strdict_get_key_at(dict, pos) == key);
  assert(strdict_get_value_at(dict, pos) == value);

  free(dup_key);
}

static void check_not_found(StrDict *const dict, char const *key, _Optional void *const value)
{
  // Duplicate the key to verify that the dictionary isn't simply comparing addresses
  char *const dup_key = test_strdup(key);

  assert(strdict_find(dict, dup_key, NULL) == false);

  size_t found = MagicValue;
  assert(strdict_find(dict, dup_key, &found) == false);
  assert(found == MagicValue);

  assert(strdict_find_value(dict, dup_key, NULL) == NULL);

  found = MagicValue;
  assert(strdict_find_value(dict, dup_key, &found) == NULL);
  assert(found == MagicValue);

  assert(strdict_find_specific(dict, dup_key, value, NULL) == false);

  found = MagicValue;
  assert(strdict_find_specific(dict, dup_key, value, &found) == false);
  assert(found == MagicValue);

  free(dup_key);
}

static void remove_singleton_common(remove_t *const remove_cb)
{
  /* Remove singleton */
  StrDict dict;
  int value;
  char const *const key = "Bright";

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  size_t index = MagicValue;
  assert(strdict_insert(&dict, key, &value, &index));
  assert(index == 0);
  check_find(&dict, key, &value, 0);
  assert(strdict_count(&dict) == 1);

  remove_cb(&dict, key, &value, 0);
  check_not_found(&dict, key, &value);
  assert(strdict_count(&dict) == 0);

  remove_cb(&dict, key, &value, SIZE_MAX);
  check_not_found(&dict, key, &value);
  assert(strdict_count(&dict) == 0);

  strdict_destroy(&dict, never_call_me, NULL);
}

static void remove_head_common(remove_t *const remove_cb)
{
  /* Remove head */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_find(&dict, keys[i], &values[i], 0);
    assert(strdict_count(&dict) == NumberOfItems - i);

    remove_cb(&dict, keys[i], &values[i], 0);
    check_not_found(&dict, keys[i], &values[i]);
    assert(strdict_count(&dict) == NumberOfItems - i - 1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(i + 1 + index < NumberOfItems);
      assert(strdict_get_key_at(&dict, index) == keys[i + 1 + index]);
      assert(strdict_get_value_at(&dict, index) == &values[i + 1 + index]);
    }
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void remove_tail_common(remove_t *const remove_cb)
{
  /* Remove tail */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"zebra", "YAK", "Camel", "bison", "Aardvark", ""};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_find(&dict, keys[i], &values[i], NumberOfItems - 1 - i);
    assert(strdict_count(&dict) == NumberOfItems - i);

    remove_cb(&dict, keys[i], &values[i], NumberOfItems - 1 - i);
    check_not_found(&dict, keys[i], &values[i]);
    assert(strdict_count(&dict) == NumberOfItems - i - 1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(index < NumberOfItems);
      assert(strdict_get_key_at(&dict, index) == keys[NumberOfItems - 1 - index]);
      assert(strdict_get_value_at(&dict, index) == &values[NumberOfItems - 1 - index]);
    }
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void remove_middle_common(remove_t *const remove_cb)
{
  /* Remove middle */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  size_t const j = NumberOfItems / MiddleDivider;

  check_find(&dict, keys[j], &values[j], j);
  assert(strdict_count(&dict) == NumberOfItems);

  remove_cb(&dict, keys[j], &values[j], j);
  check_not_found(&dict, keys[j], &values[j]);
  assert(strdict_count(&dict) == NumberOfItems - 1);

  for (size_t index = 0; index < strdict_count(&dict); ++index) {
    assert(index < strdict_count(&dict));
    assert(index < NumberOfItems);
    size_t const k = (index >= j ? index + 1 : index);
    assert(strdict_get_key_at(&dict, index) == keys[k]);
    assert(strdict_get_value_at(&dict, index) == &values[k]);
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems - 1);

  for (size_t i = 0; i < callback_count; ++i)
  {
    size_t const k = (i >= j ? i + 1 : i);
    assert(callbacks[i].key == keys[k]);
    assert(callbacks[i].value == &values[k]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void remove_null_common(remove_t *const remove_cb)
{
  /* Remove head with null value */
  StrDict dict;
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], NULL, NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_find(&dict, keys[i], NULL, 0);
    assert(strdict_count(&dict) == NumberOfItems - i);

    remove_cb(&dict, keys[i], NULL, 0);
    check_not_found(&dict, keys[i], NULL);
    assert(strdict_count(&dict) == NumberOfItems - i - 1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(i + 1 + index < NumberOfItems);
      assert(strdict_get_key_at(&dict, index) == keys[i + 1 + index]);
      assert(strdict_get_value_at(&dict, index) == NULL);
    }
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void insert_head_common(insert_t *const insert_cb)
{
  /* Insert at head */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"zebra", "YAK", "Camel", "bison", "Aardvark", ""};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_not_found(&dict, keys[i], &values[i]);
    assert(strdict_count(&dict) == i);

    insert_cb(&dict, keys[i], &values[i], 0);

    check_find(&dict, keys[i], &values[i], 0);
    assert(strdict_count(&dict) == i+1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(index <= i);
      assert(strdict_get_key_at(&dict, index) == keys[i - index]);
      assert(strdict_get_value_at(&dict, index) == &values[i - index]);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[NumberOfItems - 1 - i]);
    assert(callbacks[i].value == &values[NumberOfItems - 1 - i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void insert_tail_common(insert_t *const insert_cb)
{
  /* Insert at tail */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_not_found(&dict, keys[i], &values[i]);
    assert(strdict_count(&dict) == i);

    insert_cb(&dict, keys[i], &values[i], i);

    check_find(&dict, keys[i], &values[i], i);
    assert(strdict_count(&dict) == i+1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(index <= i);
      assert(strdict_get_key_at(&dict, index) == keys[index]);
      assert(strdict_get_value_at(&dict, index) == &values[index]);
    }
  }


  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void insert_middle_common(insert_t *const insert_cb)
{
  /* Insert in middle */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems/MiddleDivider; ++i)
  {
    check_not_found(&dict, keys[i], &values[i]);
    assert(strdict_count(&dict) == i * 2);

    insert_cb(&dict, keys[i], &values[i], i);

    check_find(&dict, keys[i], &values[i], i);
    assert(strdict_count(&dict) == (i * 2) + 1);

    size_t const j = NumberOfItems - 1 - i;

    check_not_found(&dict, keys[j], &values[j]);
    assert(strdict_count(&dict) == (i * 2) + 1);

    insert_cb(&dict, keys[j], &values[j], i + 1);

    check_find(&dict, keys[j], &values[j], i + 1);
    assert(strdict_count(&dict) == (i * 2) + 2);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      size_t const k = (index <= i ? index : j + (index - i - 1));
      assert(k < NumberOfItems);
      assert(strdict_get_key_at(&dict, index) == keys[k]);
      assert(strdict_get_value_at(&dict, index) == &values[k]);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void insert_null_common(insert_t *const insert_cb)
{
  StrDict dict;
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    insert_cb(&dict, keys[i], NULL, i);

    check_find(&dict, keys[i], NULL, i);
    assert(strdict_count(&dict) == i+1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(index <= i);
      assert(strdict_get_key_at(&dict, index) == keys[index]);
      assert(strdict_get_value_at(&dict, index) == NULL);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == NULL);
    assert(callbacks[i].arg == &dummy);
  }
}

static void try_insert(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  size_t ins_pos;
  bool success = false;
  for (unsigned long limit = 0; limit < FortifyAllocationLimit && !success; ++limit)
  {
    Fortify_SetNumAllocationsLimit(limit);
    ins_pos = MagicValue;
    success = strdict_insert(dict, key, value, &ins_pos);
  }
  Fortify_SetNumAllocationsLimit(ULONG_MAX);
  assert(success);
  assert(pos == ins_pos);
}

static void try_insert_no_pos(StrDict *const dict, char const *const key, _Optional void *const value, size_t const pos)
{
  NOT_USED(pos);
  bool success = false;
  for (unsigned long limit = 0; limit < FortifyAllocationLimit && !success; ++limit)
  {
    Fortify_SetNumAllocationsLimit(limit);
    success = strdict_insert(dict, key, value, NULL);
  }
  Fortify_SetNumAllocationsLimit(ULONG_MAX);
  assert(success);
}

static void test1(void)
{
  /* Initialize */
  StrDict dict;

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);
  assert(strdict_count(&dict) == 0);

  for (size_t index = 0; index < strdict_count(&dict); ++index) {
    assert("not empty" == NULL);
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void test2(void)
{
  insert_head_common(try_insert);
}

static void test3(void)
{
  insert_tail_common(try_insert);
}

static void test4(void)
{
  insert_middle_common(try_insert);
}

static void test5(void)
{
  remove_singleton_common(remove_key_only);
}

static void test6(void)
{
  remove_head_common(remove_key_only);
}

static void test7(void)
{
  remove_tail_common(remove_key_only);
}

static void test8(void)
{
  remove_middle_common(remove_key_only);
}

static void test9(void)
{
  /* Reinitialize */
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  strdict_destroy(&dict, (StrDictDestructorFn *)NULL, NULL);
  strdict_init(&dict);

  assert(strdict_count(&dict) == 0);
  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    check_not_found(&dict, keys[i], &values[i]);
  }

  for (size_t index = 0; index < strdict_count(&dict); ++index) {
    assert("not empty" == NULL);
  }

  strdict_destroy(&dict, (StrDictDestructorFn *)NULL, NULL);
}

static void test10(void)
{
  remove_singleton_common(remove_key_and_get_value);
}

static void test11(void)
{
  remove_head_common(remove_key_and_get_value);
}

static void test12(void)
{
  remove_tail_common(remove_key_and_get_value);
}

static void test13(void)
{
  remove_middle_common(remove_key_and_get_value);
}

static void test14(void)
{
  remove_singleton_common(remove_specific);
}

static void test15(void)
{
  remove_head_common(remove_specific);
}

static void test16(void)
{
  remove_tail_common(remove_specific);
}

static void test17(void)
{
  remove_middle_common(remove_specific);
}

static void test18(void)
{
  remove_singleton_common(remove_index);
}

static void test19(void)
{
  remove_head_common(remove_index);
}

static void test20(void)
{
  remove_tail_common(remove_index);
}

static void test21(void)
{
  remove_middle_common(remove_index);
}

static void test22(void)
{
  remove_singleton_common(remove_index_and_get_value);
}

static void test23(void)
{
  remove_head_common(remove_index_and_get_value);
}

static void test24(void)
{
  remove_tail_common(remove_index_and_get_value);
}

static void test25(void)
{
  remove_middle_common(remove_index_and_get_value);
}

static void test26(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    for (int k = -1; k < 1; ++k)
    {
      char *const bisect_key = test_strdup(keys[i]);
      size_t const len = strlen(bisect_key);
      if (len > 0) {
        bisect_key[len - 1] += k;
      }

      size_t const bisect_index = strdict_bisect_left(&dict, bisect_key);
      assert(bisect_index <= strdict_count(&dict));

      for (size_t j = 0; j < bisect_index; ++j) {
        char const *const key = strdict_get_key_at(&dict, j);
        assert(stricmp(key, bisect_key) < 0);
      }

      for (size_t j = bisect_index; j < strdict_count(&dict); ++j) {
        char const *const key = strdict_get_key_at(&dict, j);
        assert(stricmp(key, bisect_key) >= 0);
      }
      free(bisect_key);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test27(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    for (int k = -1; k < 1; ++k)
    {
      char *const bisect_key = test_strdup(keys[i]);
      size_t const len = strlen(bisect_key);
      if (len > 0) {
        bisect_key[len - 1] += k;
      }

      size_t const bisect_index = strdict_bisect_right(&dict, bisect_key);
      assert(bisect_index <= strdict_count(&dict));

      for (size_t j = 0; j < bisect_index; ++j) {
        char const *const key = strdict_get_key_at(&dict, j);
        assert(stricmp(key, bisect_key) <= 0);
      }

      for (size_t j = bisect_index; j < strdict_count(&dict); ++j) {
        char const *const key = strdict_get_key_at(&dict, j);
        assert(stricmp(key, bisect_key) > 0);
      }
      free(bisect_key);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test28(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    for (size_t j = 0; j <= i; ++j)
    {
      for (int k = -1; k <= 1; ++k)
      {
        for (int l = -1; l <= 1; ++l)
        {
          char *const min_key = test_strdup(keys[j]);
          size_t const min_len = strlen(min_key);
          if (min_len > 0) {
            min_key[min_len - 1] += k;
          }

          char *const max_key = test_strdup(keys[i]);
          size_t const max_len = strlen(max_key);
          if (max_len > 0) {
            max_key[max_len - 1] += l;
          }

          size_t min_index = SIZE_MAX, max_index = 0;

          printf("min_key '%s', max_key '%s'\n", min_key, max_key);

          STRDICT_FOR_EACH_IN_RANGE(&dict, min_key, max_key, index, tmp)
          {
            assert(index < strdict_count(&dict));

            if (index < min_index) {
              min_index = index;
            }

            if (index > max_index) {
              max_index = index;
            }

            char const *const key = strdict_get_key_at(&dict, index);
            assert(stricmp(min_key, key) <= 0);
            assert(stricmp(key, max_key) <= 0);
          }

          printf("min_index %zu, max_index %zu\n", min_index, max_index);

          if (stricmp(min_key, max_key) > 0) {
            assert(min_index > max_index);
          }

          if (min_index <= max_index) {
            for (size_t j = 0; j < min_index; ++j) {
              char const *const key = strdict_get_key_at(&dict, j);
              assert(stricmp(key, min_key) < 0);
            }

            for (size_t j = max_index + 1; j < strdict_count(&dict); ++j) {
              char const *const key = strdict_get_key_at(&dict, j);
              assert(stricmp(key, max_key) > 0);
            }

            assert(min_index == strdict_bisect_left(&dict, min_key));
            assert(max_index == strdict_bisect_right(&dict, max_key) - 1);
          }
          free(min_key);
          free(max_key);
        }
      }
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test29(void)
{
  insert_null_common(try_insert);
}

static void test30(void)
{
  remove_null_common(remove_key_only);
}

static void test31(void)
{
  remove_null_common(remove_key_and_get_value);
}

static void test32(void)
{
  remove_null_common(remove_specific);
}

static void test33(void)
{
  remove_null_common(remove_index);
}

static void test34(void)
{
  remove_null_common(remove_index_and_get_value);
}

static void test35(void)
{
  StrDict dict;
  char const *const keys[NumberOfKeys] = {"James", "Lizzy"};
  int values[NumberOfKeys * NumberOfDuplicates];

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < (NumberOfKeys * NumberOfDuplicates); ++i)
  {
    size_t const j = i % NumberOfKeys;
    size_t const k = i / NumberOfKeys;
    assert(k < NumberOfDuplicates);

    size_t ins_pos = MagicValue;
    assert(strdict_insert(&dict, keys[j], &values[i], &ins_pos));
    assert(strdict_get_key_at(&dict, ins_pos) == keys[j]);
    assert(strdict_get_value_at(&dict, ins_pos) == &values[i]);

    char *const dup_key = test_strdup(keys[j]);
    assert(dup_key);
    size_t find_pos = MagicValue;
    assert(strdict_find(&dict, dup_key, &find_pos));
    assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);

    find_pos = MagicValue;
    _Optional void *const value = strdict_find_value(&dict, dup_key, &find_pos);
    bool found_value = false;
    for (size_t l = 0; l <= k && !found_value; ++l) {
      size_t const allowed_pos = j + (NumberOfKeys * l);
      assert(allowed_pos < (NumberOfKeys * NumberOfDuplicates));
      printf("%zu: Consider %zu: %p\n", i, allowed_pos, (void *)&values[allowed_pos]);
      if (value == &values[allowed_pos]) {
        found_value = true;
      }
    }
    assert(found_value);
    assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);

    assert(strdict_count(&dict) == i+1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      assert(index <= i);
      assert(stricmp(strdict_get_key_at(&dict, index), keys[index / (k+1)]) == 0);
    }
    free(dup_key);
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == (NumberOfKeys * NumberOfDuplicates));

  for (size_t i = 0; i < callback_count; ++i)
  {
    size_t const k = i / NumberOfDuplicates;

    assert(stricmp(callbacks[i].key, keys[k]) == 0);

    bool found_value = false;
    for (size_t l = 0; l < NumberOfDuplicates && !found_value; ++l) {
      size_t const allowed_pos = k + (NumberOfKeys * l);
      assert(allowed_pos < (NumberOfKeys * NumberOfDuplicates));
      printf("%zu: Consider %zu: %p\n", i, allowed_pos, (void *)&values[allowed_pos]);
      if (callbacks[i].value == &values[allowed_pos]) {
        found_value = true;
      }
    }
    assert(found_value);

    assert(callbacks[i].arg == &dummy);
  }
}

static void test36(void)
{
  StrDict dict;
  char const *const keys[NumberOfKeys] = {"James", "Lizzy"};
  int values[NumberOfKeys * NumberOfDuplicates];

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < (NumberOfKeys * NumberOfDuplicates); ++i)
  {
    size_t const j = i % NumberOfKeys;
    strdict_insert(&dict, keys[j], &values[i], NULL);
  }

  for (size_t i = 0; i < (NumberOfKeys * NumberOfDuplicates); ++i)
  {
    size_t const j = i % NumberOfKeys;
    size_t const k = i / NumberOfKeys;
    char *const dup_key = test_strdup(keys[j]);
    assert(dup_key);

    size_t find_pos = MagicValue;
    assert(strdict_find(&dict, dup_key, &find_pos));
    assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);

    assert(strdict_count(&dict) == (NumberOfKeys * NumberOfDuplicates) - i);

    size_t rem_pos = MagicValue;
    assert(strdict_remove(&dict, dup_key, &rem_pos));
    assert(rem_pos == find_pos);

    find_pos = MagicValue;
    bool success = strdict_find(&dict, dup_key, &find_pos);
    if (k == NumberOfDuplicates-1) {
      assert(!success);
    } else {
      assert(success);
      assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);
    }

    assert(strdict_count(&dict) == (NumberOfKeys * NumberOfDuplicates) - i - 1);
    free(dup_key);
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void test37(void)
{
  StrDict dict;
  char const *const keys[NumberOfKeys] = {"James", "Lizzy"};
  int values[NumberOfKeys * NumberOfDuplicates];

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < (NumberOfKeys * NumberOfDuplicates); ++i)
  {
    size_t const j = i % NumberOfKeys;
    strdict_insert(&dict, keys[j], &values[i], NULL);
  }

  for (size_t i = 0; i < (NumberOfKeys * NumberOfDuplicates); ++i)
  {
    size_t const j = i % NumberOfKeys;
    size_t const k = i / NumberOfKeys;
    char *const dup_key = test_strdup(keys[j]);

    size_t find_pos = MagicValue;
    assert(strdict_find(&dict, dup_key, &find_pos));
    assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);

    assert(strdict_count(&dict) == (NumberOfKeys * NumberOfDuplicates) - i);

    assert(strdict_remove_specific(&dict, dup_key, &values[i], NULL));

    find_pos = MagicValue;
    bool success = strdict_find(&dict, dup_key, &find_pos);
    if (k == NumberOfDuplicates-1) {
      assert(!success);
    } else {
      assert(success);
      assert(stricmp(strdict_get_key_at(&dict, find_pos), dup_key) == 0);
    }

    assert(strdict_count(&dict) == (NumberOfKeys * NumberOfDuplicates) - i - 1);

    for (size_t index = 0; index < strdict_count(&dict); ++index) {
      assert(index < strdict_count(&dict));
      for (size_t k = 0; k <= i; ++k) {
        assert(strdict_get_value_at(&dict, index) != &values[k]);
      }
    }

    free(dup_key);
  }

  strdict_destroy(&dict, never_call_me, NULL);
}

static void test38(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  StrDictVIter iter;
  memset(&iter, CHAR_MAX, sizeof(iter));
  assert(strdictviter_all_init(&iter, &dict) == NULL);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  size_t i = 0;
  for (_Optional void *value = strdictviter_all_init(&iter, &dict);
       value != NULL;
       value = strdictviter_advance(&iter))
  {
    assert(value == &values[i++]);
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test39(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  StrDictVIter iter;
  memset(&iter, CHAR_MAX, sizeof(iter));
  assert(strdictviter_init(&iter, &dict, "", "") == NULL);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    for (size_t j = 0; j <= i; ++j)
    {
      for (int k = -1; k <= 1; ++k)
      {
        for (int l = -1; l <= 1; ++l)
        {
          char *const min_key = test_strdup(keys[j]);
          size_t const min_len = strlen(min_key);
          if (min_len > 0) {
            min_key[min_len - 1] += k;
          }

          char *const max_key = test_strdup(keys[i]);
          size_t const max_len = strlen(max_key);
          if (max_len > 0) {
            max_key[max_len - 1] += l;
          }

          size_t vcount = 0;
          _Optional int *got_values[NumberOfItems];

          printf("min_key '%s', max_key '%s'\n", min_key, max_key);

          for (_Optional void *value = strdictviter_init(&iter, &dict, min_key, max_key);
               value != NULL;
               value = strdictviter_advance(&iter))
          {
            got_values[vcount++] = value;
          }

          size_t const min_index = strdict_bisect_left(&dict, min_key),
                       max_index = strdict_bisect_right(&dict, max_key);

          if (max_index >= min_index) {
            assert(vcount == max_index - min_index);
          } else {
            assert(vcount == 0);
          }

          for (size_t i = 0; i < vcount; ++i) {
            assert(&values[min_index + i] == got_values[i]);
          }

          free(min_key);
          free(max_key);
        }
      }
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test40(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  StrDictVIter iter;
  memset(&iter, CHAR_MAX, sizeof(iter));
  assert(strdictviter_all_init(&iter, &dict) == NULL);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  size_t count = 0;
  for (_Optional void *value = strdictviter_all_init(&iter, &dict);
       value != NULL;
       value = strdictviter_advance(&iter), ++count)
  {
    assert(count < NumberOfItems);
    assert(value == &values[count]);

    if (count % RemoveInterval) {
      size_t const pos = strdictviter_remove(&iter);
      assert(pos == (count / RemoveInterval) + 1);
    }
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems / RemoveInterval);

  for (size_t i = 0; i < callback_count; ++i)
  {
    size_t const j = i * RemoveInterval;
    assert(callbacks[i].key == keys[j]);
    assert(callbacks[i].value == &values[j]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test41(void)
{
  StrDict dict;
  int values[NumberOfItems];
  char const *const keys[NumberOfItems] = {"", "Aardvark", "bison", "Camel", "YAK", "zebra"};

  memset(&dict, CHAR_MAX, sizeof(dict));
  strdict_init(&dict);

  for (size_t i = 0; i < NumberOfItems; ++i)
  {
    strdict_insert(&dict, keys[i], &values[i], NULL);
  }

  size_t count = 0;
  STRDICT_FOR_EACH(&dict, index, tmp)
  {
    assert(index < strdict_count(&dict));
    assert(count < NumberOfItems);
    assert(index == count);

    assert(strdict_get_key_at(&dict, index) == keys[count]);
    assert(strdict_get_value_at(&dict, index) == &values[count]);
    ++count;
  }

  callback_count = 0;
  int dummy;
  strdict_destroy(&dict, record_callbacks, &dummy);

  assert(callback_count == NumberOfItems);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].key == keys[i]);
    assert(callbacks[i].value == &values[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test42(void)
{
  insert_head_common(try_insert_no_pos);
}

static void test43(void)
{
  insert_tail_common(try_insert_no_pos);
}

static void test44(void)
{
  insert_middle_common(try_insert_no_pos);
}

static void test45(void)
{
  insert_null_common(try_insert_no_pos);
}

static void test46(void)
{
  remove_singleton_common(remove_specific_no_pos);
}

static void test47(void)
{
  remove_head_common(remove_specific_no_pos);
}

static void test48(void)
{
  remove_tail_common(remove_specific_no_pos);
}

static void test49(void)
{
  remove_middle_common(remove_specific_no_pos);
}

static void test50(void)
{
  remove_null_common(remove_specific_no_pos);
}

static void test51(void)
{
  remove_singleton_common(remove_key_and_get_value_no_pos);
}

static void test52(void)
{
  remove_head_common(remove_key_and_get_value_no_pos);
}

static void test53(void)
{
  remove_tail_common(remove_key_and_get_value_no_pos);
}

static void test54(void)
{
  remove_middle_common(remove_key_and_get_value_no_pos);
}

static void test55(void)
{
  remove_null_common(remove_key_and_get_value_no_pos);
}

static void test56(void)
{
  remove_singleton_common(remove_key_only_no_pos);
}

static void test57(void)
{
  remove_head_common(remove_key_only_no_pos);
}

static void test58(void)
{
  remove_tail_common(remove_key_only_no_pos);
}

static void test59(void)
{
  remove_middle_common(remove_key_only_no_pos);
}

static void test60(void)
{
  remove_null_common(remove_key_only_no_pos);
}

void strdict_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  }
  unit_tests[] =
  {
    { "Initialize", test1 },
    { "Insert at head", test2 },
    { "Insert at tail", test3 },
    { "Insert in middle", test4 },
    { "Remove key from singleton", test5 },
    { "Remove key from head", test6 },
    { "Remove key from tail", test7 },
    { "Remove key from middle", test8 },
    { "Reinitialize", test9 },
    { "Remove key and get value from singleton", test10 },
    { "Remove key and get value from head", test11 },
    { "Remove key and get value from tail", test12 },
    { "Remove key and get value from middle", test13 },
    { "Remove specific value from singleton", test14 },
    { "Remove specific value from head", test15 },
    { "Remove specific value from tail", test16 },
    { "Remove specific value from middle", test17 },
    { "Remove index from singleton", test18 },
    { "Remove index from head", test19 },
    { "Remove index from tail", test20 },
    { "Remove index from middle", test21 },
    { "Remove index and get value from singleton", test22 },
    { "Remove index and get value from head", test23 },
    { "Remove index and get value from tail", test24 },
    { "Remove index and get value from middle", test25 },
    { "Bisect left", test26 },
    { "Bisect right", test27 },
    { "For each in range", test28 },
    { "Insert null value", test29 },
    { "Remove key with null value", test30 },
    { "Remove key and get null value", test31 },
    { "Remove specific null value", test32 },
    { "Remove index of null value", test33 },
    { "Remove index and get null value", test34 },
    { "Insert duplicate", test35 },
    { "Remove duplicate", test36 },
    { "Remove duplicate key with specific value", test37 },
    { "Iterate over all values", test38 },
    { "Iterate over a range of values", test39 },
    { "Iterate over values with removal", test40 },
    { "For each", test41 },
    { "Insert at head without position", test42 },
    { "Insert at tail without position", test43 },
    { "Insert in middle without position", test44 },
    { "Insert null value without position", test45 },
    { "Remove specific value from singleton without position", test46 },
    { "Remove specific value from head without position", test47 },
    { "Remove specific value from tail without position", test48 },
    { "Remove specific value from middle without position", test49 },
    { "Remove specific null value without position", test50 },
    { "Remove key and get value from singleton without position", test51 },
    { "Remove key and get value from head without position", test52 },
    { "Remove key and get value from tail without position", test53 },
    { "Remove key and get value from middle without position", test54 },
    { "Remove key and get null value without position", test55 },
    { "Remove key from singleton without position", test56 },
    { "Remove key from head without position", test57 },
    { "Remove key from tail without position", test58 },
    { "Remove key from middle without position", test59 },
    { "Remove key with null value without position", test60 },
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
