/*
 * CBUtilLib: Integer dictionary
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

/* History:
  CJB: 10-Aug-22: First release of this header file.
  CJB: 11-Aug-22: The INTDICT_FOR_EACH macro now requires an extra parameter
                  to make it clear when the limit variable shadows another
                  variable declaration.
                  Use size_t instead of int for array indices.
                  Added a missing #include for assertions.
                  Added a value iterator type.
                  Updated the copyright header.
  CJB: 14-Aug-22: Return Boolean success/failure instead of -1.
                  intdict_contains was subsumed into intdict_find.
                  Added the intdict_find_specific function.
                  intdictviter_remove now returns the removed item's index.
  CJB: 29-Aug-22: Commented out the last intdictviter_init parameter name.
 */

#ifndef IntDict_h
#define IntDict_h

#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

typedef long int IntDictKey;
#define INTDICTKEY_MIN LONG_MIN
#define INTDICTKEY_MAX LONG_MAX
#define PRIIntDictKey "ld"

typedef struct IntDictItem {
  IntDictKey key;
  void *value;
} IntDictItem;

typedef struct {
  size_t nalloc;
  size_t nitems;
  IntDictKey sought_key;
  IntDictItem *array;
  IntDictItem const *candidate;
} IntDict;
   /*
    * An integer dictionary type that associates every item in an ordered
    * list of integers (keys) with a pointer to a value. Duplicate keys are
    * allowed unless the client explicitly takes steps to prevent them.
    */

void intdict_init(IntDict */*dict*/);
   /*
    * Initialize an integer dictionary.
    */

typedef void IntDictDestructorFn(IntDictKey /*key*/, void */*value*/,
                                 void */*arg*/);
   /*
    * Type of function called back to destroy each integer dictionary item.
    * The value of 'arg' is that passed to the intdict_destroy function and
    * is expected to point to any additional parameters.
    */

void intdict_destroy(IntDict */*dict*/, IntDictDestructorFn */*destructor*/,
                     void */*arg*/);
   /*
    * Destroy an integer dictionary, optionally calling a destructor function
    * (if the callback function pointer is not null).
    */

bool intdict_find(IntDict */*dict*/, IntDictKey /*key*/, size_t */*index*/);
   /*
    * Search for the first item with a given key in an integer dictionary.
    * Outputs the index of the item if the dictionary contains the key.
    * Indices are not guaranteed to remain valid after inserting or
    * removing items.
    * Returns: true if the dictionary contains the key, otherwise false.
    */

bool intdict_find_specific(IntDict */*dict*/, IntDictKey /*key*/,
                           void */*value*/, size_t */*index*/);
   /*
    * Search for the first item with a given key and value in an integer
    * dictionary. Outputs the index of the item if found.
    * Indices are not guaranteed to remain valid after inserting or
    * removing items.
    * Returns: true if the dictionary contained the key/value pair,
    *          otherwise false.
    */

bool intdict_insert(IntDict */*dict*/, IntDictKey /*key*/, void */*value*/,
                    size_t */*index*/);
   /*
    * Insert an item and value pair into an integer dictionary. If the new
    * item's key is not unique then its position is indeterminate relative
    * to any items with equal keys that were already in the dictionary.
    * Outputs the index of the inserted item if successful.
    * Returns: true if successful, otherwise false (out of memory).
    */

static inline size_t intdict_count(IntDict const *const dict)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  return dict->nitems;
}
   /*
    * Count the number of items in an integer dictionary.
    * Returns: number of items.
    */

static inline IntDictKey intdict_get_key_at(IntDict const *const dict,
                                            size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  return dict->array[index].key;
}
   /*
    * Get the key currently at a given index in an integer dictionary.
    * Returns: the key with the given index.
    */

static inline void *intdict_get_value_at(IntDict const *const dict,
                                         size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  return dict->array[index].value;
}
   /*
    * Get the value currently at a given index in an integer dictionary.
    * Returns: pointer to the value with the given index.
    */

void intdict_remove_at(IntDict */*dict*/, size_t /*index*/);
   /*
    * Remove the item currently at a given index from an integer dictionary.
    */

void *intdict_remove_value_at(IntDict */*dict*/, size_t /*index*/);
   /*
    * Remove the item currently at a given index from an integer dictionary,
    * returning the associated value.
    * Returns: pointer to the removed value.
    */

size_t intdict_bisect_left(IntDict */*dict*/, IntDictKey /*key*/);
   /*
    * Search in an integer dictionary for the lowest key not less than a
    * specified key. If all the keys in the dictionary are less than the
    * specified key then the position (one beyond the last key) where an
    * equal key would be inserted is returned. Indices are not
    * guaranteed to remain valid after inserting or removing items.
    * Returns: the (inclusive) position beyond which all items with equal
    *          or higher keys can be found.
    */

size_t intdict_bisect_right(IntDict */*dict*/, IntDictKey /*key*/);
   /*
    * Search in an integer dictionary for the lowest key greater than a
    * specified key. If all the keys in the dictionary are less than or
    * equal to the specified key then the position (one beyond the last key)
    * where a greater key would be inserted is returned. Indices are not
    * guaranteed to remain valid after inserting or removing items.
    * Returns: the (exclusive) position before which all items with lower
    *          keys can be found.
    */

static inline void *intdict_find_value(IntDict *const dict,
                                       IntDictKey const key,
                                       size_t *const index)
{
  size_t pos;
  if (!intdict_find(dict, key, &pos)) {
    return NULL;
  }
  if (index) {
    *index = pos;
  }
  return intdict_get_value_at(dict, pos);
}
   /*
    * Search for the first item with a given key in an integer dictionary,
    * returning the associated value. Outputs the index of the item if the
    * dictionary contains the key.
    * Returns: pointer to the value associated with the given key, or NULL
    *          if the key was not found.
    */

static inline bool intdict_remove(IntDict *const dict,
                                  IntDictKey const key, size_t *const index)
{
  size_t pos;
  if (!intdict_find(dict, key, &pos)) {
    return false;
  }
  intdict_remove_at(dict, pos);
  if (index) {
    *index = pos;
  }
  return true;
}
   /*
    * Remove an item with a given key from an integer dictionary. If the
    * specified key is not unique then it is indeterminate which item with
    * that key was removed. Outputs the former position of the removed item
    * if successful.
    * Returns: true if the dictionary contained the key, otherwise false.
    */

static inline void *intdict_remove_value(IntDict *const dict,
                                         IntDictKey const key,
                                         size_t *const index)
{
  size_t pos;
  if (!intdict_find(dict, key, &pos)) {
    return NULL;
  }
  if (index) {
    *index = pos;
  }
  return intdict_remove_value_at(dict, pos);
}
   /*
    * Remove an item with a given key from an integer dictionary, returning
    * the associated value. If the specified key is not unique then it is
    * indeterminate which item with that key was removed.
    * Returns: pointer to the value associated with the given key, or NULL
    *          if the key was not found.
    */

static inline bool intdict_remove_specific(IntDict *dict,
                                           IntDictKey key,
                                           void *value,
                                           size_t *index)
{
  size_t pos;
  if (!intdict_find_specific(dict, key, value, &pos)) {
    return false;
  }
  intdict_remove_at(dict, pos);
  if (index) {
    *index = pos;
  }
  return true;
}
   /*
    * Remove an item with a given key and value from an integer dictionary.
    * Outputs the former index of the removed item if successful.
    * Returns: true if the dictionary contained the key/value pair,
    *          otherwise false.
    */

#define INTDICT_FOR_EACH(dict, index, tmp) \
  for (size_t (index) = 0, (tmp) = intdict_count(dict); \
       (index) < (tmp); \
       ++(index))
   /*
    * Macro to be used for iterating over an integer dictionary. The
    * dictionary must not be modified within the body of the loop. Indices
    * are generated in sorted key order. 'index' has the same scope as the
    * body of the loop and points to the current item.
    */

#define INTDICT_FOR_EACH_IN_RANGE(dict, min_key, max_key, index, tmp) \
  for (size_t (index) = intdict_bisect_left((dict), (min_key)), \
         (tmp) = intdict_bisect_right((dict), (max_key)); \
       (index) < (tmp); \
       ++(index))
   /*
    * Macro to be used for iterating over a range of keys within an integer
    * dictionary. The dictionary must not be modified within the body of the
    * loop. 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal
    * the same key to iterate over all items with that key). Indices are
    * generated in sorted key order. 'index' has the same scope as the body
    * of the loop and points to the current item.
    */

typedef struct {
  IntDict *dict;
  size_t next_index, end;
} IntDictVIter;
   /*
    * An integer dictionary value iterator type that is used to
    * iterate over the values that are stored in a dictionary.
    */

void *intdictviter_init(IntDictVIter */*iter*/, IntDict */*dict*/,
  IntDictKey /*min_key*/, IntDictKey /*max_key*/);
   /*
    * Initialise an iterator object in preparation for iterating over the
    * values associated with a given range of keys in an integer dictionary.
    * 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal the
    * same key to iterate over all values with that key). Modifying the
    * dictionary (except by calling intdictviter_remove) invalidates the
    * iterator.
    * Returns: pointer to the value associated with the first key in the
    *          given range, or NULL if the range is empty.
    */

void *intdictviter_all_init(IntDictVIter */*iter*/, IntDict */*dict*/);
   /*
    * Initialise an iterator object in preparation for iterating over all the
    * values stored in an integer dictionary. Modifying the dictionary
    * (except by calling intdictviter_remove) invalidates the iterator.
    * Returns: pointer to the value associated with the first key in
    *          the dictionary, or NULL if the dictionary is empty.
    */

void *intdictviter_advance(IntDictVIter */*iter*/);
   /*
    * Advance an iterator object to get the next value from its associated
    * integer dictionary. Values are returned in sorted key order.
    * Returns: pointer to the value associated with the next key, or NULL if
    *          there are no more values.
    */

size_t intdictviter_remove(IntDictVIter */*iter*/);
   /*
    * Remove the current item from the integer dictionary associated with an
    * iterator object. Modifying the dictionary by any other method
    * invalidates the iterator.
    * Returns: the former index of the removed item.
    */

#endif
