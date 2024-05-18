/*
 * CBUtilLib: String dictionary
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
  CJB: 11-Aug-22: The STRDICT_FOR_EACH macro now requires an extra parameter
                  to make it clearer when the limit variable shadows another
                  variable declaration.
                  Use size_t instead of int for array indices.
                  Added a value iterator type.
                  Updated the copyright header.
  CJB: 14-Aug-22: Return Boolean success/failure instead of -1.
                  strdict_contains was subsumed into strdict_find.
                  Added the strdict_find_specific function.
                  strdictviter_remove now returns the removed item's index.
  CJB: 18-May-24: Corrected description of the return value of strdict_remove.
 */

#ifndef StrDict_h
#define StrDict_h

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "StringBuff.h"

typedef struct StrDictItem {
  char const *key;
  void *value;
} StrDictItem;

typedef struct {
  size_t nalloc;
  size_t nitems;
  StringBuffer buffer;
  char const *sought_key;
  StrDictItem *array;
  StrDictItem const *candidate;
} StrDict;
   /*
    * A string dictionary type that associates every item in an ordered
    * list of strings (keys) with a pointer to a value. Duplicate keys are
    * allowed unless the client explicitly takes steps to prevent them.
    * Upper and lower case characters are considered equivalent in keys.
    */

void strdict_init(StrDict */*dict*/);
   /*
    * Initialize a string dictionary.
    */

typedef void StrDictDestructorFn(char const */*key*/,
                                 void */*value*/, void */*arg*/);
   /*
    * Type of function called back to destroy each string dictionary item.
    * The value of 'arg' is that passed to the strdict_destroy function and
    * is expected to point to any additional parameters.
    */

void strdict_destroy(StrDict */*dict*/, StrDictDestructorFn */*destructor*/,
                     void */*arg*/);
   /*
    * Destroy a string dictionary, optionally calling a destructor function
    * (if the callback function pointer is not null).
    */

bool strdict_find(StrDict */*dict*/, char const * /*key*/,
                  size_t */*index*/);
   /*
    * Search for the first item with a given key in a string dictionary.
    * Outputs the index of the item if the dictionary contains the key.
    * Indices are not guaranteed to remain valid after inserting or
    * removing items.
    * Returns: true if the dictionary contains the key, otherwise false.
    */

bool strdict_find_specific(StrDict */*dict*/, char const * /*key*/,
                            void */*value*/, size_t */*index*/);
   /*
    * Search for the first item with a given key and value in a string
    * dictionary. Outputs the index of the item if found.
    * Indices are not guaranteed to remain valid after inserting or
    * removing items.
    * Returns: true if the dictionary contained the key/value pair,
    *          otherwise false.
    */

bool strdict_insert(StrDict */*dict*/, char const * /*key*/, void */*value*/,
                   size_t */*index*/);
   /*
    * Insert an item and value pair into a string dictionary. If the new
    * item's key is not unique then its position is indeterminate relative
    * to any items with equal keys that were already in the dictionary.
    * Outputs the index of the inserted item if successful.
    * Returns: true if successful, otherwise false (out of memory).
    */

static inline size_t strdict_count(StrDict const *const dict)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  return dict->nitems;
}
   /*
    * Count the number of items in a string dictionary.
    * Returns: number of items.
    */

static inline char const *strdict_get_key_at(StrDict const *const dict,
                                             size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  return dict->array[index].key;
}
   /*
    * Get the key currently at a given index in a string dictionary.
    * Returns: the key with the given index.
    */

static inline void *strdict_get_value_at(StrDict const *const dict,
                                         size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  return dict->array[index].value;
}
   /*
    * Get the value currently at a given index in a string dictionary.
    * Returns: pointer to the value with the given index.
    */

void strdict_remove_at(StrDict */*dict*/, size_t /*index*/);
   /*
    * Remove the item currently at a given index from a string dictionary.
    */

void *strdict_remove_value_at(StrDict */*dict*/, size_t /*index*/);
   /*
    * Remove the item currently at a given index from a string dictionary,
    * returning the associated value.
    * Returns: pointer to the removed value.
    */

size_t strdict_bisect_left(StrDict */*dict*/, char const * /*key*/);
   /*
    * Search in a string dictionary for the lowest key not less than a
    * specified key. If all the keys in the dictionary are less than the
    * specified key then the position (one beyond the last key) where an
    * equal key would be inserted is returned. Indices are not
    * guaranteed to remain valid after inserting or removing items.
    * Returns: the (inclusive) position beyond which all items with equal
    *          or higher keys can be found.
    */

size_t strdict_bisect_right(StrDict */*dict*/, char const * /*key*/);
   /*
    * Search in a string dictionary for the lowest key greater than a
    * specified key. If all the keys in the dictionary are less than or
    * equal to the specified key then the position (one beyond the last key)
    * where a greater key would be inserted is returned. Indices are not
    * guaranteed to remain valid after inserting or removing items.
    * Returns: the (exclusive) position before which all items with lower
    *          keys can be found.
    */

static inline void *strdict_find_value(StrDict *const dict,
                                       char const *const key,
                                       size_t *const index)
{
  size_t pos;
  if (!strdict_find(dict, key, &pos)) {
    return NULL;
  }
  if (index) {
    *index = pos;
  }
  return strdict_get_value_at(dict, pos);
}
   /*
    * Search for the first item with a given key in a string dictionary,
    * returning the associated value. Outputs the index of the item if the
    * dictionary contains the key.
    * Returns: pointer to the value associated with the given key, or NULL
    *          if the key was not found.
    */

static inline bool strdict_remove(StrDict *const dict,
                                  char const *const key,
                                  size_t *const index)
{
  size_t pos;
  if (!strdict_find(dict, key, &pos)) {
    return false;
  }
  if (index) {
    *index = pos;
  }
  strdict_remove_at(dict, pos);
  return true;
}
   /*
    * Remove an item with a given key from a string dictionary.
    * If the specified key is not unique then it is indeterminate which item
    * with that key was removed. Outputs the former position of the removed item
    * if successful.
    * Returns: true if the dictionary contained the key, otherwise false.
    */

static inline void *strdict_remove_value(StrDict *const dict,
                                         char const *const key,
                                         size_t *const index)
{
  size_t pos;
  if (!strdict_find(dict, key, &pos)) {
    return NULL;
  }
  if (index) {
    *index = pos;
  }
  return strdict_remove_value_at(dict, pos);
}
   /*
    * Remove an item with a given key from a string dictionary, returning
    * the associated value. If the specified key is not unique then it is
    * indeterminate which item with that key was removed.
    * Returns: pointer to the value associated with the given key, or NULL
    *          if the key was not found.
    */

static inline bool strdict_remove_specific(StrDict *dict,
                                           char const *key,
                                           void *value,
                                           size_t *index)
{
  size_t pos;
  if (!strdict_find_specific(dict, key, value, &pos)) {
    return false;
  }
  strdict_remove_at(dict, pos);
  if (index) {
    *index = pos;
  }
  return true;
}
   /*
    * Remove an item with a given key and value from a string dictionary.
    * Outputs the former index of the removed item if successful.
    * Returns: true if the dictionary contained the key/value pair,
    *          otherwise false.
    */

#define STRDICT_FOR_EACH(dict, index, tmp) \
  for (size_t (index) = 0, (tmp) = strdict_count(dict); \
       (index) < (tmp); \
       ++(index))
   /*
    * Macro to be used for iterating over a string dictionary. The dictionary
    * must not be modified within the body of the loop. Indices are generated
    * in sorted key order. 'index' has the same scope as the body of the loop
    * and points to the current item.
    */

#define STRDICT_FOR_EACH_IN_RANGE(dict, min_key, max_key, index, tmp) \
  for (size_t (index) = strdict_bisect_left((dict), (min_key)), \
         (tmp) = strdict_bisect_right((dict), (max_key)); \
       (index) < (tmp); \
       ++(index))
   /*
    * Macro to be used for iterating over a range of keys within a string
    * dictionary. The dictionary must not be modified within the body of the
    * loop. 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal
    * the same key to iterate over all items with that key). Indices are
    * generated in sorted key order. 'index' has the same scope as the body
    * of the loop and points to the current item.
    */

typedef struct {
  StrDict *dict;
  size_t next_index, end;
} StrDictVIter;
   /*
    * A string dictionary value iterator type that is used to
    * iterate over the values that are stored in a dictionary.
    */

void *strdictviter_init(StrDictVIter */*iter*/, StrDict */*dict*/,
  char const */*min_key*/, char const */*max_key*/);
   /*
    * Initialise an iterator object in preparation for iterating over the
    * values associated with a given range of keys in a string dictionary.
    * 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal the
    * same key to iterate over all values with that key). Modifying the
    * dictionary (except by calling intdictviter_remove) invalidates the
    * iterator.
    * Returns: pointer to the value associated with the first key in the
    *          given range, or NULL if the range is empty.
    */

void *strdictviter_all_init(StrDictVIter */*iter*/, StrDict */*dict*/);
   /*
    * Initialise an iterator object in preparation for iterating over all the
    * values stored in a string dictionary. Modifying the dictionary (except
    * by calling intdictviter_remove) invalidates the iterator.
    * Returns: pointer to the value associated with the first key in
    *          the dictionary, or NULL if the dictionary is empty.
    */

void *strdictviter_advance(StrDictVIter */*iter*/);
   /*
    * Advance an iterator object to get the next value from its associated
    * string dictionary. Values are returned in sorted key order.
    * Returns: pointer to the value associated with the next key, or NULL if
    *          there are no more values.
    */

size_t strdictviter_remove(StrDictVIter */*iter*/);
   /*
    * Remove the current item from the string dictionary associated with an
    * iterator object. Modifying the dictionary by any other method
    * invalidates the iterator.
    * Returns: the former index of the removed item.
    */

#endif
