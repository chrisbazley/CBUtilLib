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
  CJB: 10-Aug-22: First release of this source file.
  CJB: 11-Aug-22: Use size_t instead of int for array indices.
  CJB: 14-Aug-22: Return Boolean success/failure instead of -1.
                  Replace intdict_remove_specific with intdict_find_specific.
  CJB: 29-Aug-22: Fix text of debug output from intdict_find_specific.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
  CJB: 09-Apr-25: Add missing const qualifier in bisect_left().
                  Use a local alias for 'array' in find_specific().
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "IntDict.h"
#include "Internal/CBUtilMisc.h"

enum {
  ArrayInitSize = 4,
  ArrayGrowthFactor = 2,
};

void intdict_init(IntDict *const dict)
{
  DEBUGF("Initializing integer dictionary %p\n", (void *)dict);
  assert(dict);
  *dict = (IntDict){0};
}

void intdict_destroy(IntDict *const dict,
  _Optional IntDictDestructorFn *const destructor, _Optional void *const arg)
{
  DEBUGF("Terminating integer dictionary %p\n", (void *)dict);
  assert(dict);
  assert(dict->nitems <= dict->nalloc);

  if (destructor && dict->array) {
    size_t const nitems = dict->nitems;
    IntDictItem const *const array = &*dict->array;

    for (size_t i = 0; i < nitems; ++i) {
      destructor(array[i].key, array[i].value, arg);
      assert(nitems == dict->nitems);
    }
  }

  free(dict->array);
}

static int compare_key_n_item(const void *const key, const void *const item)
{
  assert(key);
  assert(item);
  IntDict *const dict = (IntDict *)key;
  IntDictItem const *const candidate = item;

  if (dict->array)
  {
    assert(candidate >= dict->array);
    assert(candidate < dict->array + dict->nitems);
  }

  dict->candidate = candidate;

  if (dict->sought_key < candidate->key) {
    return -1;
  }

  if (dict->sought_key > candidate->key) {
    return 1;
  }

  return 0;
}

void intdict_remove_at(IntDict *const dict, size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);

  if (!dict->array) {
    return;
  }
  IntDictItem *const array = &*dict->array;

  DEBUGF("Removing item with key %" PRIIntDictKey
         ", value %p at position %zu in dictionary %p of size %zu\n",
         array[index].key, array[index].value, index,
         (void *)dict, dict->nitems);

  assert(dict->nitems > 0);
  dict->nitems--;

  size_t const nitems = dict->nitems;
  for (size_t i = index; i < nitems; ++i) {
    assert(i + 1 < dict->nalloc);
    array[i] = array[i + 1];
  }

  dict->candidate = NULL;

#ifndef NDEBUG
  if (nitems > 0) {
    for (size_t i = 0; i < dict->nitems - 1; ++i) {
      assert(i + 1 < dict->nalloc);
      DEBUGF("%zu: Key %" PRIIntDictKey ", value %p\n", i,
             array[i].key, array[i].value);

      assert(array[i].key <= array[i + 1].key);
    }
    DEBUGF("%zu: key %" PRIIntDictKey ", value %p\n", dict->nitems - 1,
           array[dict->nitems - 1].key, array[dict->nitems - 1].value);
  }
#endif
}

_Optional void *intdict_remove_value_at(IntDict *const dict, size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  _Optional void *const value = dict->array ? dict->array[index].value : NULL;
  intdict_remove_at(dict, index);
  return value;
}

bool intdict_find(IntDict *const dict, IntDictKey const key,
  _Optional size_t *const pos)
{
  size_t const index = intdict_bisect_left(dict, key);
  if (!dict->array || index >= dict->nitems || dict->array[index].key != key) {
    DEBUGF("Can't find key %" PRIIntDictKey "\n", key);
    return false;
  }

  DEBUGF("Found key %" PRIIntDictKey " at index %zu\n", key, index);
  if (pos) {
    *pos = index;
  }
  return true;
}

bool intdict_find_specific(IntDict *const dict, IntDictKey const key,
  _Optional void *const value, _Optional size_t *const pos)
{
  if (!dict->array) {
    return false;
  }
  IntDictItem const *const array = &*dict->array;

  size_t index = intdict_bisect_left(dict, key);
  if (index >= dict->nitems) {
    DEBUGF("Can't find key %" PRIIntDictKey " because it's too high\n",
           key);
    return false;
  }

  if (array[index].key != key) {
    DEBUGF("Can't find non-existent key %" PRIIntDictKey "\n", key);
    return false;
  }

  while (array[index].key == key &&
         array[index].value != value &&
         index + 1 < dict->nitems) {
    ++index;
  }

  if (array[index].key != key ||
      array[index].value != value) {
    DEBUGF("Can't find non-existent value %p with key %" PRIIntDictKey
           "\n", value, key);
    return false;
  }

  if (pos) {
    *pos = index;
  }
  return true;
}

bool intdict_insert(IntDict *const dict, IntDictKey const key,
                    _Optional void *const value, _Optional size_t *const index)
{
  DEBUGF("Insert key %" PRIIntDictKey
         " with value %p in dictionary %p of size %zu\n",
         key, value, (void *)dict, dict->nitems);
  assert(dict);
  assert(dict->nitems <= dict->nalloc);

  size_t const ins_index = intdict_bisect_left(dict, key);
  size_t const nitems = dict->nitems;

  if (nitems == dict->nalloc) {
    if (dict->nalloc == SIZE_MAX) {
      DEBUGF("Can't reallocate dictionary at max size\n");
      return false;
    }

    size_t new_size = ArrayInitSize;
    if (dict->nalloc > 0) {
      if (dict->nalloc < SIZE_MAX / ArrayGrowthFactor) {
        new_size = dict->nalloc * ArrayGrowthFactor;
      } else {
        new_size = SIZE_MAX;
      }
    }

    DEBUGF("Reallocating dictionary from %zu to %zu items\n", dict->nalloc,
           new_size);
    _Optional IntDictItem *const new_array = realloc(
      dict->array, new_size * sizeof(*new_array));

    if (!new_array) {
      DEBUGF("Memory allocation failure\n");
      return false;
    }
    dict->nalloc = new_size;
    dict->array = new_array;
  }

  if (!dict->array) {
    return false;
  }
  IntDictItem *const array = &*dict->array;

  DEBUGF("Inserting item with key %" PRIIntDictKey ", value %p at %zu\n",
         key, value, ins_index);
  dict->candidate = NULL;

  for (size_t i = nitems; i > ins_index; --i) {
    assert(i < dict->nalloc);
    assert(i > 0);
    array[i] = array[i - 1];
  }

  assert(ins_index < dict->nalloc);
  array[ins_index] = (IntDictItem){.key = key, .value = value};

  assert(dict->nitems < dict->nalloc);
  dict->nitems++;

#ifndef NDEBUG
  for (size_t i = 0; i < dict->nitems - 1; ++i) {
    assert(i + 1 < dict->nalloc);
    DEBUGF("%zu: Key %" PRIIntDictKey ", value %p\n", i, array[i].key,
          array[i].value);
    assert(array[i].key <= array[i + 1].key);
  }
  DEBUGF("%zu: key %" PRIIntDictKey ", value %p\n", dict->nitems - 1,
         array[dict->nitems - 1].key, array[dict->nitems - 1].value);
#endif

  if (index) {
    *index = ins_index;
  }
  return true;
}

size_t intdict_bisect_left(IntDict *const dict, IntDictKey const key)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  DEBUGF("Searching for smallest key >= %" PRIIntDictKey "\n", key);

  if (!dict->array) {
    return 0;
  }
  IntDictItem const *const array = &*dict->array;

  size_t index = 0;

  if (dict->nitems > 0) {
    if (dict->candidate) {
      if (dict->sought_key == key) {
        DEBUGF("Reuse last result of search for key %" PRIIntDictKey "\n",
               dict->sought_key);
      } else {
        DEBUGF("Discard last search result for key %" PRIIntDictKey "\n",
               dict->sought_key);
        dict->candidate = NULL;
      }
    }

    if (!dict->candidate) {
      dict->sought_key = key;
      (void)bsearch(dict, array, dict->nitems,
          sizeof(array[0]), compare_key_n_item);
    }

    if (dict->candidate) {
      assert(dict->sought_key == key);
      assert(dict->candidate >= array);
      assert(dict->candidate < array + dict->nitems);

      index = (size_t)(dict->candidate - array);
      assert(index < dict->nitems);
      if (array[index].key < key) {
        DEBUGF("Candidate %p was too low: %" PRIIntDictKey " < %"
               PRIIntDictKey "\n",
               (void *)dict->candidate, dict->candidate->key, key);

        /* Search forward for the lowest key greater than or equal to
           the sought key */
        do {
          ++index;
        } while(index < dict->nitems && array[index].key < key);

      } else {
        DEBUGF("Candidate %p was not too low: %" PRIIntDictKey " >= %"
               PRIIntDictKey "\n",
               (void *)dict->candidate, dict->candidate->key, key);

        /* Search backward for the lowest key greater than or equal to
           the sought key */
        while (index > 0 && array[index - 1].key >= key) {
          --index;
        }
      }

      if (index > 0) {
        assert(index - 1 < dict->nitems);
        assert(array[index - 1].key < key);
      }
    } else {
      DEBUGF("No candidate\n");
    }
  }

  DEBUGF("Key %" PRIIntDictKey " belongs at position %zu (current key %"
         PRIIntDictKey ")\n", key, index,
         index < dict->nitems ? array[index].key : INTDICTKEY_MAX);

  assert(index <= dict->nitems);
  return index;
}

size_t intdict_bisect_right(IntDict *const dict, IntDictKey const key)
{
  if (!dict->array) {
    return 0;
  }
  IntDictItem const *const array = &*dict->array;

  size_t index = intdict_bisect_left(dict, key);

  DEBUGF("Searching for lowest key > %" PRIIntDictKey
        " in dictionary of size %zu\n", key, dict->nitems);
  while (index < dict->nitems && array[index].key <= key) {
    ++index;
  }

  return index;
}

