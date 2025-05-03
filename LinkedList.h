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

/* LinkedList.h declares functions and types for a double-linked list.

Dependencies: ANSI C library (if !NDEBUG).
Message tokens: None.
History:
  CJB: 16-Dec-14: Created this header file.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
  CJB: 12-Jun-22: New macro definitions to help iterate over linked lists.
                  Replaced function-like macros with inline functions.
  CJB: 11-Aug-22: The LINKEDLIST_FOR_EACH_SAFE macro now requires an extra
                  parameter to make it clear when the next item pointer
                  shadows another variable declaration.
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
  CJB: 26-Apr-25: Remove the _Optional qualifier from linkedlist_for_each's
                  callback function argument, because it makes no sense to
                  require callbacks to handle null values.
*/

#ifndef linkedlist_h
#define linkedlist_h

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

typedef struct LinkedListItem
{
  _Optional struct LinkedListItem *prev;
  _Optional struct LinkedListItem *next;
}
LinkedListItem;
   /*
    * A generic type for linked list items. This is usually a member of a
    * bigger struct containing application-specific data.
    */

typedef struct
{
  _Optional LinkedListItem *head;
  _Optional LinkedListItem *tail;
}
LinkedList;
   /*
    * A linked list type.
    */

void linkedlist_init(LinkedList */*list*/);
   /*
    * Initializes a given linked list. The caller must already have allocated
    * space for the 'list' control structure. The initialized list is empty.
    */

static inline _Optional LinkedListItem *linkedlist_get_head(LinkedList const *const list)
{
  assert(list);
  assert(list->head == NULL || list->head->prev == NULL);
  return list->head;
}
   /*
    * Gets the head item of a list.
    * Returns: pointer to the head item of the list, or NULL if the list is empty.
    */

static inline _Optional LinkedListItem *linkedlist_get_tail(LinkedList const *const list)
{
  assert(list);
  assert(list->tail == NULL || list->tail->next == NULL);
  return list->tail;
}
   /*
    * Gets the tail item of a list.
    * Returns: pointer to the tail item of the list, or NULL if the list is empty.
    */

static inline _Optional LinkedListItem *linkedlist_get_next(LinkedListItem const *const item)
{
  assert(item);
  assert(item->next == NULL || item->next->prev == item);
  return item->next;
}
   /*
    * Gets the next list item after a given item.
    * Returns: pointer to the next item, or NULL if at the tail of the list.
    */

static inline _Optional LinkedListItem *linkedlist_get_prev(LinkedListItem const *const item)
{
  assert(item);
  assert(item->prev == NULL || item->prev->next == item);
  return item->prev;
}
   /*
    * Gets the previous list item before a given item.
    * Returns: pointer to the previous item, or NULL if at the head of the list.
    */

void linkedlist_insert(LinkedList */*list*/,
                       _Optional LinkedListItem */*prev*/,
                       LinkedListItem */*item*/);
   /*
    * Adds an item to a linked list after the specified 'prev' item. If 'prev'
    * is NULL then the item will the added at the head of the list. Storage
    * allocation for the list item is the caller's responsibility.
    */

void linkedlist_remove(LinkedList */*list*/, LinkedListItem */*item*/);
   /*
    * Removes a client data item from a global list. Storage deallocation for
    * the list item is the caller's responsibility.
    */

typedef bool LinkedListCallbackFn(LinkedList */*list*/,
                                  LinkedListItem */*item*/,
                                  void *const /*arg*/);
   /*
    * Type of function called back for each list item. The value of 'arg' is
    * that passed to the linkedlist_for_each function and is expected to
    * point to any additional parameters. It is safe to remove the current
    * item in this function.
    * Returns: true to stop iterating over the list, otherwise false.
    */

  _Optional LinkedListItem *linkedlist_for_each(LinkedList */*list*/,
                                                LinkedListCallbackFn */*callback*/,
                                                void *const /*arg*/);
   /*
    * Calls a given function for each client data item in a linked list, in
    * order from head to tail. The value of 'arg' will be passed to the
    * 'callback' function with the address of each item and is expected to
    * point to any additional parameters required. Can be used to search a
    * list, if a suitable callback function is provided.
    * Returns: address of the list item on which iteration stopped, or
    *          NULL if the callback function never returned true.
    */

bool linkedlist_is_member(const LinkedList */*list*/, const LinkedListItem */*item*/);
   /*
    * Finds out whether or not a given item is a member of a list.
    * Returns: false if the item is not a member of the list, otherwise true.
    */

#define LINKEDLIST_FOR_EACH(list, item) \
  for (LinkedListItem *(item) = (LinkedListItem *)linkedlist_get_head(list); \
       (item) != NULL; \
       (item) = (LinkedListItem *)linkedlist_get_next(item))
   /*
    * Macro to be used for iterating over a linked list (or part of a list) in
    * cases where the current list item is not removed or invalidated within
    * the body of the loop. 'item' has the same scope as the body of the loop
    * and points to the current list item.
    */

#define LINKEDLIST_FOR_EACH_REVERSE(list, item) \
  for (LinkedListItem *(item) = (LinkedListItem *)linkedlist_get_tail(list); \
       (item) != NULL; \
       (item) = (LinkedListItem *)linkedlist_get_prev(item))
   /*
    * Macro to be used for iterating backwards over a linked list
    * (or part of a list) in cases where the current list item is not removed
    * or invalidated within the body of the loop. 'item' has the same scope as
    * the body of the loop and points to the current list item.
    */

#define LINKEDLIST_FOR_EACH_SAFE(list, item, tmp) \
  for (LinkedListItem *(item) = (LinkedListItem *)linkedlist_get_head(list), *(tmp); \
       (tmp) = (item) ? (LinkedListItem *)linkedlist_get_next(item) : (LinkedListItem *)NULL, (item) != NULL; \
       (item) = (tmp))
   /*
    * Macro to be used for iterating over a linked list (or part of a list) in
    * cases where the current list item may be removed or invalidated within the
    * body of the loop. 'item' has the same scope as the body of the loop
    * and points to the current list item.
    */

#endif
