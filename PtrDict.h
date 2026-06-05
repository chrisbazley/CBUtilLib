/*
 * CBUtilLib: Pointer dictionary
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
  CJB: 31-May-26: First release of this header file.
 */

#ifndef ptrdict_h
#define ptrdict_h

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

typedef struct
{
  IntDict _private;
} PtrDict;
/*
 * a pointer dictionary type that associates every item in an ordered
 * list of pointers (keys) with a pointer to a value. Duplicate keys are
 * allowed unless the client explicitly takes steps to prevent them.
 */

static inline void ptrdict_init(PtrDict *const dict)
{
  assert(dict);
  return intdict_init(&dict->_private);
}
/*
 * Initialize a pointer dictionary.
 */

typedef void PtrDictDestructorFn(void * /*key*/, _Optional void * /*value*/,
                                 void * /*arg*/);
/*
 * Type of function called back to destroy each pointer dictionary item.
 * The value of 'arg' is that passed to the ptrdict_destroy function and
 * is expected to point to any additional parameters.
 */

typedef struct {
  _Optional PtrDictDestructorFn *destructor,
  void *arg;
} PtrDictPrivateDestructorArg;
/*
 * For internal use only.
 */

static void ptrdict_private_destructor(intptr_t const key,
                                       _Optional void *const value,
                                       void *const arg)
{
  PtrDictPrivateDestructorArg const *const wrapper = arg;
  assert(wrapper);
  if (wrapper->destructor)
  {
    wrapper->destructor((void *)key, value, wrapper->arg);
  }
}
/*
 * For internal use only.
 */

static inline void ptrdict_destroy(PtrDict *const dict,
                                   _Optional PtrDictDestructorFn *const destructor,
                                   void *const arg)
{
  assert(dict);
  PtrDictPrivateDestructorArg wrapper = { destructor, arg };
  return intdict_destroy(&dict->_private,
                         destructor ? ptrdict_private_destructor : NULL,
                         &wrapper);
}
/*
 * Destroy a pointer dictionary, optionally calling a destructor function
 * (if the callback function pointer is not null).
 */

static inline bool ptrdict_find(PtrDict *const dict, void *const key,
                                _Optional size_t *const index)
{
  assert(dict);
  assert(key);
  return intdict_find(&dict->_private, (intptr_t)key, index);
}

/*
 * Search for the first item with a given key in a pointer dictionary.
 * Outputs the index of the item if the dictionary contains the key.
 * Indices are not guaranteed to remain valid after inserting or
 * removing items.
 * Returns: true if the dictionary contains the key, otherwise false.
 */

static inline bool ptrdict_find_specific(PtrDict *const dict, void *const key,
                                         _Optional void * const value,
                                         _Optional size_t * const index)
{
  assert(dict);
  assert(key);
  return intdict_find_specific(&dict->_private, (intptr_t)key, value, index);
}

/*
 * Search for the first item with a given key and value in a pointer
 * dictionary. Outputs the index of the item if found.
 * Indices are not guaranteed to remain valid after inserting or
 * removing items.
 * Returns: true if the dictionary contained the key/value pair,
 *          otherwise false.
 */

static inline bool ptrdict_insert(PtrDict *const dict, void * const key,
                                  _Optional void * const value, _Optional size_t * const index)
{
  assert(dict);
  assert(key);
  return intdict_insert(&dict->_private, (intptr_t)key, value, index);
}

/*
 * Insert an item and value pair into a pointer dictionary. If the new
 * item's key is not unique then its position is indeterminate relative
 * to any items with equal keys that were already in the dictionary.
 * Outputs the index of the inserted item if successful.
 * Returns: true if successful, otherwise false (out of memory).
 */

static inline size_t ptrdict_count(PtrDict const *const dict)
{
  assert(dict);
  return intdict_count(&dict->_private);
}
/*
 * Count the number of items in a pointer dictionary.
 * Returns: number of items.
 */

static inline void *ptrdict_get_key_at(PtrDict const *const dict,
                                       size_t const index)
{
  assert(dict);
  return intdict_get_key_at(&dict->_private, index);
}
/*
 * Get the key currently at a given index in a pointer dictionary.
 * Returns: the key with the given index.
 */

static inline _Optional void *ptrdict_get_value_at(PtrDict const *const dict,
                                                   size_t const index)
{
  assert(dict);
  return intdict_get_value_at(&dict->_private, index);
}
/*
 * Get the value currently at a given index in a pointer dictionary.
 * Returns: pointer to the value with the given index.
 */

static inline void ptrdict_remove_at(PtrDict *const dict, size_t const index)
{
  assert(dict);
  intdict_remove_at(&dict->_private, index);
}

/*
 * Remove the item currently at a given index from a pointer dictionary.
 */

static inline _Optional void *ptrdict_remove_value_at(PtrDict * const dict,
                                                      size_t const index)
{
  assert(dict);
  return intdict_remove_value_at(&dict->_private, index);
}

/*
 * Remove the item currently at a given index from a pointer dictionary,
 * returning the associated value.
 * Returns: pointer to the removed value.
 */

static inline size_t ptrdict_bisect_left(PtrDict *const dict,
                                         void *const key)
{
  assert(dict);
  assert(key);
  return intdict_bisect_left(&dict->_private, (intptr_t)key);
}

/*
 * Search in a pointer dictionary for the lowest key not less than a
 * specified key. If all the keys in the dictionary are less than the
 * specified key then the position (one beyond the last key) where an
 * equal key would be inserted is returned. Indices are not
 * guaranteed to remain valid after inserting or removing items.
 * Returns: the (inclusive) position beyond which all items with equal
 *          or higher keys can be found.
 */

static inline size_t ptrdict_bisect_right(PtrDict *const dict,
                                         void *const key)
{
  assert(dict);
  assert(key);
  return intdict_bisect_right(&dict->_private, (intptr_t)key);
}

/*
 * Search in a pointer dictionary for the lowest key greater than a
 * specified key. If all the keys in the dictionary are less than or
 * equal to the specified key then the position (one beyond the last key)
 * where a greater key would be inserted is returned. Indices are not
 * guaranteed to remain valid after inserting or removing items.
 * Returns: the (exclusive) position before which all items with lower
 *          keys can be found.
 */

static inline _Optional void *ptrdict_find_value(PtrDict *const dict,
                                                 void * const key,
                                                 _Optional size_t *const index)
{
  assert(dict);
  assert(key);
  return intdict_find_value(&dict->_private, (intptr_t)key, index);
}
/*
 * Search for the first item with a given key in a pointer dictionary,
 * returning the associated value. Outputs the index of the item if the
 * dictionary contains the key.
 * Returns: pointer to the value associated with the given key, or NULL
 *          if the key was not found.
 */

static inline bool ptrdict_remove(PtrDict *const dict, void * const key,
                                  _Optional size_t *const index)
{
  assert(dict);
  assert(key);
  return intdict_remove(&dict->_private, (intptr_t)key, index);
}
/*
 * Remove an item with a given key from a pointer dictionary. If the
 * specified key is not unique then it is indeterminate which item with
 * that key was removed. Outputs the former position of the removed item
 * if successful.
 * Returns: true if the dictionary contained the key, otherwise false.
 */

static inline _Optional void *
ptrdict_remove_value(PtrDict *const dict, void * const key,
                     _Optional size_t *const index)
{
  assert(dict);
  assert(key);
  return intdict_remove_value(&dict->_private, (intptr_t)key, index);
}
/*
 * Remove an item with a given key from a pointer dictionary, returning
 * the associated value. If the specified key is not unique then it is
 * indeterminate which item with that key was removed.
 * Returns: pointer to the value associated with the given key, or NULL
 *          if the key was not found.
 */

static inline bool ptrdict_remove_specific(PtrDict *dict, void * key,
                                           _Optional void *value,
                                           _Optional size_t *index)
{
  assert(dict);
  assert(key);
  return intdict_remove_specific(&dict->_private, (intptr_t)key,
                                 value, index);
}
/*
 * Remove an item with a given key and value from a pointer dictionary.
 * Outputs the former index of the removed item if successful.
 * Returns: true if the dictionary contained the key/value pair,
 *          otherwise false.
 */

#define PTRDICT_FOR_EACH(dict, index, tmp)                                     \
  for (size_t(index) = 0, (tmp) = ptrdict_count(dict); (index) < (tmp);        \
       ++(index))
/*
 * Macro to be used for iterating over a pointer dictionary. The
 * dictionary must not be modified within the body of the loop. Indices
 * are generated in sorted key order. 'index' has the same scope as the
 * body of the loop and points to the current item.
 */

#define PTRDICT_FOR_EACH_IN_RANGE(dict, min_key, max_key, index, tmp)          \
  for (size_t(index) = ptrdict_bisect_left((dict), (min_key)),                 \
      (tmp) = ptrdict_bisect_right((dict), (max_key));                         \
       (index) < (tmp); ++(index))
/*
 * Macro to be used for iterating over a range of keys within a pointer
 * dictionary. The dictionary must not be modified within the body of the
 * loop. 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal
 * the same key to iterate over all items with that key). Indices are
 * generated in sorted key order. 'index' has the same scope as the body
 * of the loop and points to the current item.
 */

typedef struct
{
  IntDictVIter _private;
} PtrDictVIter;
/*
 * a pointer dictionary value iterator type that is used to
 * iterate over the values that are stored in a dictionary.
 */

static inline _Optional void *ptrdictviter_init(PtrDictVIter * const iter,
                                                PtrDict * const dict,
                                                void * const min_key,
                                                void * const max_key)
{
  assert(iter);
  assert(dict);
  assert(min_key);
  assert(max_key);
  return intdictviter_init(&iter->_private, &dict->_private,
                           min_key, max_key);
}

/*
 * Initialise an iterator object in preparation for iterating over the
 * values associated with a given range of keys in a pointer dictionary.
 * 'min_key' and 'max_key' are inclusive bounds (i.e. both may equal the
 * same key to iterate over all values with that key). Modifying the
 * dictionary (except by calling ptrdictviter_remove) invalidates the
 * iterator.
 * Returns: pointer to the value associated with the first key in the
 *          given range, or NULL if the range is empty.
 */

static inline _Optional void *ptrdictviter_all_init(PtrDictVIter * const iter,
                                                    PtrDict * const dict)
{
  assert(iter);
  assert(dict);
  return intdictviter_init(&iter->_private, &dict->_private);
}

/*
 * Initialise an iterator object in preparation for iterating over all the
 * values stored in a pointer dictionary. Modifying the dictionary
 * (except by calling ptrdictviter_remove) invalidates the iterator.
 * Returns: pointer to the value associated with the first key in
 *          the dictionary, or NULL if the dictionary is empty.
 */

static inline _Optional void *ptrdictviter_advance(PtrDictVIter * const iter)
{
  assert(iter);
  return intdictviter_advance(&iter->_private);
}

/*
 * Advance an iterator object to get the next value from its associated
 * pointer dictionary. Values are returned in sorted key order.
 * Returns: pointer to the value associated with the next key, or NULL if
 *          there are no more values.
 */

static inline size_t ptrdictviter_remove(PtrDictVIter * const iter)
{
  assert(iter);
  return intdictviter_remove(&iter->_private);
}
/*
 * Remove the current item from the pointer dictionary associated with an
 * iterator object. Modifying the dictionary by any other method
 * invalidates the iterator.
 * Returns: the former index of the removed item.
 */

#endif
