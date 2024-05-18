/*
 * CBUtilLib: Linked list
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

/* History:
  CJB: 16-Dec-14: Created this source file.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 19-Apr-15: Recategorised per-list-item debugging output as verbose.
  CJB: 17-Jan-16: Debug output when no callback from linkedlist_for_each.
  CJB: 17-Apr-16: Cast pointer parameters to void * to match %p. No longer
                  prints function pointers (no matching format specifier).
  CJB: 05-May-19: Recategorised more debugging output as verbose.
  CJB: 06-Jun-20: Debugging output is less verbose by default.
  CJB: 06-Feb-22: Added 'const' qualifiers to function arguments.
                  Use new macros to iterate over linked lists.
                  Validate whole list after insertion or removal of an item.
  CJB: 11-Aug-22: The LINKEDLIST_FOR_EACH_SAFE macro now requires an extra
                  parameter.
*/

/* ISO library headers */
#include <stddef.h>

/* Local headers */
#include "LinkedList.h"
#include "Internal/CBUtilMisc.h"

/* ----------------------------------------------------------------------- */
/*                       Function prototypes                               */

#ifdef NDEBUG
#define validate_list(list) NOT_USED(list)
#else
static void validate_list(const LinkedList *list);
#endif

/* ----------------------------------------------------------------------- */
/*                         Public functions                                */

void linkedlist_init(LinkedList *const list)
{
  assert(list != NULL);
  DEBUGF("LinkedList: initializing list %p\n", (void *)list);
  list->head = list->tail = NULL;
}

/* ----------------------------------------------------------------------- */

void linkedlist_insert(LinkedList *const list, LinkedListItem *const prev,
  LinkedListItem *const item)
{
  assert(item != NULL);
  DEBUGF("LinkedList: Inserting item %p into list %p after item %p\n",
         (void *)item, (void *)list, (void *)prev);

  assert(!linkedlist_is_member(list, item));
  if (prev != NULL)
    assert(linkedlist_is_member(list, prev));

  LinkedListItem *next;

  if (prev == NULL)
  {
    /* Insert at head */
    next = list->head;
    list->head = item;
  }
  else
  {
    next = prev->next;
    prev->next = item;
  }

  item->prev = prev;
  item->next = next;

  if (next == NULL)
  {
    /* Insert at tail */
    assert(list->tail == prev);
    list->tail = item;
  }
  else
  {
    next->prev = item;
  }
  validate_list(list);
}

/* ----------------------------------------------------------------------- */

void linkedlist_remove(LinkedList *const list, LinkedListItem *const item)
{
  assert(item != NULL);
  DEBUGF("LinkedList: Removing item %p (prev %p, next %p) from list %p\n",
         (void *)item, (void *)item->prev, (void *)item->next, (void *)list);

  assert(linkedlist_is_member(list, item));

  if (item->prev != NULL)
  {
    item->prev->next = item->next;
  }
  else
  {
    assert(list->head == item);
    list->head = item->next;
  }

  LinkedListItem *const prev = item->prev;
  if (item->next != NULL)
  {
    item->next->prev = prev;
  }
  else
  {
    assert(list->tail == item);
    list->tail = prev;
  }
  validate_list(list);
}

/* ----------------------------------------------------------------------- */

LinkedListItem *linkedlist_for_each(LinkedList *const list,
  LinkedListCallbackFn *const callback, void *const arg)
{
  validate_list(list);
  assert(callback != NULL);
  DEBUG_VERBOSEF("LinkedList: Calling function with %p for all items in list %p\n",
         arg, (void *)list);

  LINKEDLIST_FOR_EACH_SAFE(list, foo, bar)
  {
    DEBUG_VERBOSEF("LinkedList: Visiting item %p in list %p\n",
      (void *)foo, (void *)list);

    if (callback(list, foo, arg))
    {
      DEBUG_VERBOSEF("LinkedList: Callback terminated iteration over list %p\n",
        (void *)list);
      return foo;
    }
  }

  DEBUG_VERBOSEF("LinkedList: Iteration over list %p finished with no callback\n",
    (void *)list);

  return NULL;
}

/* ----------------------------------------------------------------------- */

bool linkedlist_is_member(const LinkedList *const list,
  const LinkedListItem *const item)
{
  validate_list(list);
  assert(item != NULL);

  LINKEDLIST_FOR_EACH(list, it)
  {
    if (it == item)
    {
      DEBUG_VERBOSEF("LinkedList: Item %p is a member of list %p\n", (void *)item, (void *)list);
      return true;
    }
  }

  DEBUG_VERBOSEF("LinkedList: Item %p is not a member of list %p\n", (void *)item, (void *)list);
  return false;
}

/* ----------------------------------------------------------------------- */
/*                         Private functions                               */

#ifndef NDEBUG
static void validate_list(const LinkedList *const list)
{
  assert(list != NULL);

  if (list->head != NULL)
  {
    assert(list->head->prev == NULL);
  }

  if (list->tail != NULL)
  {
    assert(list->tail->next == NULL);
  }

  LINKEDLIST_FOR_EACH(list, item)
  {
    if (item->next != NULL)
    {
      assert(item->next->prev == item);
    }
    else
    {
      assert(item == list->tail);
    }
  }
}
#endif
