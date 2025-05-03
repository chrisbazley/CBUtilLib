/*
 * CBUtilLib test: Linked list
 * Copyright (C) 2014 Christopher Bazley
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
#include <assert.h>

/* CBUtilLib headers */
#include "LinkedList.h"

/* Local headers */
#include "Tests.h"

enum
{
  NumberOfItems = 8,
  MiddleDivider = 2,
  KeepInterval = 2,
};


static struct
{
  LinkedList *list;
  LinkedListItem *item;
  void *arg;
}
callbacks[NumberOfItems];

static size_t callback_count;

static bool record_callbacks(LinkedList *list, LinkedListItem *item, void *arg)
{
  assert(list != NULL);
  assert(item != NULL);
  callbacks[callback_count].list = list;
  callbacks[callback_count].item = item;
  callbacks[callback_count++].arg = arg;
  return false;
}

static bool stop_iteration(LinkedList *list, LinkedListItem *item, void *arg)
{
  _Optional unsigned int *num_to_visit = arg;
  assert(list != NULL);
  assert(item != NULL);
  assert(num_to_visit != NULL);
  return ++callback_count >= *num_to_visit;
}

static bool remove_in_callback(LinkedList *list, LinkedListItem *item, void *arg)
{
  assert(list != NULL);
  assert(item != NULL);
  NOT_USED(arg);
  if (callback_count++ % KeepInterval)
  {
    linkedlist_remove(list, item);
  }
  return false;
}

static bool never_call_me(LinkedList *list, LinkedListItem *item, void *arg)
{
  NOT_USED(list);
  NOT_USED(item);
  NOT_USED(arg);
  assert("List isn't empty" == NULL);
  return true;
}

static void test1(void)
{
  /* Initialize */
  LinkedList list;

  memset(&list, CHAR_MAX, sizeof(list));

  linkedlist_init(&list);

  linkedlist_for_each(&list, never_call_me, NULL);

  assert(linkedlist_get_head(&list) == NULL);
  assert(linkedlist_get_tail(&list) == NULL);
}

static void test2(void)
{
  /* Insert at head */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    assert(!linkedlist_is_member(&list, &items[i]));
    linkedlist_insert(&list, NULL, &items[i]);
    assert(linkedlist_is_member(&list, &items[i]));
    assert(linkedlist_get_head(&list) == &items[i]);
  }

  /* List contains items[i-1], items[i-2], items[i-3] */
  callback_count = 0;
  int dummy;
  linkedlist_for_each(&list, record_callbacks, &dummy);

  assert(callback_count == ARRAY_SIZE(items));

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].list == &list);
    assert(callbacks[i].item == &items[ARRAY_SIZE(items) - 1 - i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test3(void)
{
  /* Insert at tail */
  LinkedList list;
  LinkedListItem items[NumberOfItems];
  _Optional LinkedListItem *prev = NULL;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    assert(!linkedlist_is_member(&list, &items[i]));
    linkedlist_insert(&list, prev, &items[i]);
    assert(linkedlist_is_member(&list, &items[i]));
    assert(linkedlist_get_tail(&list) == &items[i]);
    prev = &items[i];
  }
  /* List contains items[i-3], items[i-2], items[i-1] */
  callback_count = 0;
  int dummy;
  linkedlist_for_each(&list, record_callbacks, &dummy);

  assert(callback_count == ARRAY_SIZE(items));

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].list == &list);
    assert(callbacks[i].item == &items[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test4(void)
{
  /* Insert in middle */
  LinkedList list;
  LinkedListItem items[NumberOfItems];
  _Optional LinkedListItem *prev = NULL;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  size_t i;
  for (i = 0; i < ARRAY_SIZE(items)/MiddleDivider; ++i)
  {
    linkedlist_insert(&list, prev, &items[i]);
    prev = &items[i];
    linkedlist_insert(&list, prev, &items[ARRAY_SIZE(items) - 1 - i]);
  }
  /* List contains items[0], items[1] ... items[6], items[7] */

  callback_count = 0;
  int dummy;
  linkedlist_for_each(&list, record_callbacks, &dummy);

  assert(callback_count == ARRAY_SIZE(items));

  for (i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].list == &list);
    assert(callbacks[i].item == &items[i]);
    assert(callbacks[i].arg == &dummy);
  }
}

static void test5(void)
{
  /* Remove singleton */
  LinkedList list;
  LinkedListItem item;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(&item, CHAR_MAX, sizeof(item));

  linkedlist_init(&list);
  linkedlist_insert(&list, NULL, &item);
  assert(linkedlist_is_member(&list, &item));
  linkedlist_remove(&list, &item);
  assert(!linkedlist_is_member(&list, &item));

  linkedlist_for_each(&list, never_call_me, NULL);
}

static void test6(void)
{
  /* Remove head */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  size_t i;
  for (i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  /* List contains items[i-1], items[i-2], items[i-3] */

  for (; i > 0; --i)
  {
    int dummy;

    assert(linkedlist_is_member(&list, &items[i-1]));
    linkedlist_remove(&list, &items[i-1]);
    assert(!linkedlist_is_member(&list, &items[i-1]));

    callback_count = 0;
    linkedlist_for_each(&list, record_callbacks, &dummy);

    assert(callback_count == i-1);

    for (size_t j = 0; j < callback_count; ++j)
    {
      assert(i >= 2);
      assert(i - 2 >= j);
      assert(callbacks[j].list == &list);
      assert(callbacks[j].item == &items[i - 2 - j]);
      assert(callbacks[j].arg == &dummy);
    }
  }
}

static void test7(void)
{
  /* Remove tail */
  LinkedList list;
  LinkedListItem items[NumberOfItems];
  _Optional LinkedListItem *prev = NULL;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  size_t i;
  for (i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, prev, &items[i]);
    prev = &items[i];
  }
  /* List contains items[i-3], items[i-2], items[i-1] */

  for (; i > 0; --i)
  {
    assert(linkedlist_is_member(&list, &items[i-1]));
    linkedlist_remove(&list, &items[i-1]);
    assert(!linkedlist_is_member(&list, &items[i-1]));
  }
}

static void test8(void)
{
  /* Remove middle */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  size_t i;
  for (i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  /* List contains items[i-1], items[i-2], items[i-3] */

  assert(linkedlist_is_member(&list, &items[i/MiddleDivider]));
  linkedlist_remove(&list, &items[i/MiddleDivider]);
  assert(!linkedlist_is_member(&list, &items[i/MiddleDivider]));
}

static void test9(void)
{
  /* Is member of another list */
  LinkedList lists[NumberOfItems];
  LinkedListItem items[NumberOfItems];

  memset(lists, CHAR_MAX, sizeof(lists));
  memset(items, CHAR_MAX, sizeof(items));

  for (size_t i = 0; i < ARRAY_SIZE(lists); ++i)
  {
    linkedlist_init(&lists[i]);
  }

  for (size_t i = 0; i < ARRAY_SIZE(lists); ++i)
  {
    linkedlist_insert(&lists[(i+1) % ARRAY_SIZE(lists)], NULL, &items[i]);
    assert(!linkedlist_is_member(&lists[i], &items[i]));
  }
}

static void test10(void)
{
  /* Reinitialize */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  linkedlist_init(&list);

  linkedlist_for_each(&list, never_call_me, NULL);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    assert(!linkedlist_is_member(&list, &items[i]));
  }
}

static void test11(void)
{
  /* Stop iteration */
  LinkedList list;
  LinkedListItem items[NumberOfItems];
  size_t num_to_visit = ARRAY_SIZE(items) / MiddleDivider;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  callback_count = 0;
  linkedlist_for_each(&list, stop_iteration, &num_to_visit);
  assert(callback_count == num_to_visit);
}

static void test12(void)
{
  /* Remove in callback */
  LinkedList list;
  LinkedListItem items[NumberOfItems];
  int dummy;

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  callback_count = 0;
  linkedlist_for_each(&list, remove_in_callback, NULL);
  assert(callback_count == ARRAY_SIZE(items));

  callback_count = 0;
  linkedlist_for_each(&list, record_callbacks, &dummy);
  assert(callback_count == ARRAY_SIZE(items) / KeepInterval);

  for (size_t i = 0; i < callback_count; ++i)
  {
    assert(callbacks[i].list == &list);
    assert(callbacks[i].item ==
       &items[ARRAY_SIZE(items) - 1 - (i * KeepInterval)]);

    assert(callbacks[i].arg == &dummy);
  }
}

static void test13(void)
{
  /* Get previous */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  size_t i;
  for (i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  size_t j = 0;
  for (_Optional LinkedListItem *p = linkedlist_get_tail(&list);
       p != NULL;
       p = linkedlist_get_prev(&*p), ++j)
  {
    assert(j < ARRAY_SIZE(items));
    assert(p == &items[j]);
  }
  assert(j == ARRAY_SIZE(items));
}

static void test14(void)
{
  /* Get next */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  size_t j = 0;
  for (_Optional LinkedListItem *p = linkedlist_get_head(&list);
       p != NULL;
       p = linkedlist_get_next(&*p), ++j)
  {
    assert(j < ARRAY_SIZE(items));
    assert(p == &items[ARRAY_SIZE(items) - 1 - j]);
  }
  assert(j == ARRAY_SIZE(items));
}

static void test15(void)
{
  /* For each */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  size_t j = 0;
  LINKEDLIST_FOR_EACH(&list, item)
  {
    assert(j < ARRAY_SIZE(items));
    assert(item == &items[ARRAY_SIZE(items) - 1 - j]);
    ++j;
  }
}

static void test16(void)
{
  /* Remove in for each */
  LinkedList list;
  LinkedListItem items[NumberOfItems];

  memset(&list, CHAR_MAX, sizeof(list));
  memset(items, CHAR_MAX, sizeof(items));

  linkedlist_init(&list);

  for (size_t i = 0; i < ARRAY_SIZE(items); ++i)
  {
    linkedlist_insert(&list, NULL, &items[i]);
  }

  size_t j = 0;
  LINKEDLIST_FOR_EACH_SAFE(&list, item, tmp)
  {
    if (j++ % KeepInterval)
    {
      linkedlist_remove(&list, item);
    }
  }

  j = 0;
  LINKEDLIST_FOR_EACH_SAFE(&list, item, tmp)
  {
    assert(item == &items[ARRAY_SIZE(items) - 1 - (j * KeepInterval)]);
    ++j;
  }
}

void LinkedList_tests(void)
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
    { "Remove singleton", test5 },
    { "Remove head", test6 },
    { "Remove tail", test7 },
    { "Remove middle", test8 },
    { "Is member of another list", test9 },
    { "Reinitialize", test10 },
    { "Stop iteration", test11 },
    { "Remove in callback", test12 },
    { "Get previous", test13 },
    { "Get next", test14 },
    { "For each", test15 },
    { "Remove in for each", test16 },
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
