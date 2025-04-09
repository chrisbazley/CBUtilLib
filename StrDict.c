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
  CJB: 10-Aug-22: First release of this source file.
  CJB: 11-Aug-22: Use size_t instead of int for array indices.
  CJB: 14-Aug-22: Return Boolean success/failure instead of -1.
                  Replace strdict_remove_specific with strdict_find_specific.
  CJB: 29-Aug-22: Fix text of debug output from strdict_find_specific.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
  CJB: 09-Apr-25: Add missing const qualifier in bisect_left().
                  Use a local alias for 'array' in find_specific().
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "StrExtra.h"
#include "StrDict.h"
#include "Internal/CBUtilMisc.h"

enum {
  ArrayInitSize = 4,
  ArrayGrowthFactor = 2,
};

void strdict_init(StrDict *const dict)
{
  DEBUGF("Initializing string dictionary %p\n", (void *)dict);
  assert(dict);
  *dict = (StrDict){0};
  stringbuffer_init(&dict->buffer);
}

void strdict_destroy(StrDict *const dict,
  _Optional StrDictDestructorFn *const destructor, _Optional void *const arg)
{
  DEBUGF("Terminating string dictionary %p\n", (void *)dict);
  assert(dict);
  assert(dict->nitems <= dict->nalloc);

  if (destructor && dict->array) {
    size_t const nitems = dict->nitems;
    StrDictItem const *const array = &*dict->array;

    for (size_t i = 0; i < nitems; ++i) {
      destructor(array[i].key, array[i].value, arg);
      assert(nitems == dict->nitems);
    }
  }

  stringbuffer_destroy(&dict->buffer);
  free(dict->array);
}

static int compare_key_n_item(const void *const key, const void *const item)
{
  assert(key);
  assert(item);
  StrDict *const dict = (StrDict *)key;
  assert(dict->sought_key);
  StrDictItem const *const candidate = item;

  if (dict->array)
  {
    assert(candidate >= dict->array);
    assert(candidate < dict->array + dict->nitems);
  }

  dict->candidate = candidate;
  return stricmp(STRING_OR_NULL(dict->sought_key), candidate->key);
}

void strdict_remove_at(StrDict *const dict, size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);

  if (!dict->array) {
    return;
  }
  StrDictItem *const array = &*dict->array;

  DEBUGF("Removing item with key '%s'"
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
      DEBUGF("%zu: Key %p:'%s', value %p\n", i, (void *)array[i].key,
             array[i].key, array[i].value);
      assert(stricmp(array[i].key, array[i + 1].key) <= 0);
    }
    DEBUGF("%zu: key %p:'%s', value %p\n", dict->nitems - 1,
           (void *)array[dict->nitems - 1].key,
           array[dict->nitems - 1].key, array[dict->nitems - 1].value);
  }
#endif
}

_Optional void *strdict_remove_value_at(StrDict *const dict, size_t const index)
{
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  assert(index < dict->nitems);
  _Optional void *const value = dict->array ? dict->array[index].value : NULL;
  strdict_remove_at(dict, index);
  return value;
}

bool strdict_find(StrDict *const dict, char const *const key,
  _Optional size_t *const pos)
{
  size_t const index = strdict_bisect_left(dict, key);
  if (!dict->array || index >= dict->nitems || stricmp(dict->array[index].key, key) != 0) {
    DEBUGF("Can't find key '%s'\n", key);
    return false;
  }

  DEBUGF("Found key '%s' at index %zu\n", key, index);
  if (pos) {
    *pos = index;
  }
  return true;
}

bool strdict_find_specific(StrDict *const dict, char const *const key,
  _Optional void *const value, _Optional size_t *const pos)
{
  if (!dict->array) {
    return false;
  }
  StrDictItem const *const array = &*dict->array;

  size_t index = strdict_bisect_left(dict, key);
  if (index >= dict->nitems) {
    DEBUGF("Can't find key '%s' because it's too high\n", key);
    return false;
  }

  int diff = stricmp(array[index].key, key);

  if (diff != 0) {
    DEBUGF("Can't find non-existent key '%s'\n", key);
    return false;
  }

  while (diff == 0 &&
         array[index].value != value &&
         index + 1 < dict->nitems) {
    ++index;
    assert(index < dict->nitems);
    diff = stricmp(array[index].key, key);
  }

  if (diff != 0 || array[index].value != value) {
    DEBUGF("Can't find non-existent value %p with key '%s'\n", value, key);
    return false;
  }

  if (pos) {
    *pos = index;
  }
  return true;
}

bool strdict_insert(StrDict *const dict, char const *const key,
  _Optional void *const value, _Optional size_t *const index)
{
  /* Don't allow a null key because we want to preserve the original
     pointer in case the client dynamically allocated it, therefore allowing
     null would slow every key comparison. In any case, most library
     functions don't allow null. */
  size_t const ins_index = strdict_bisect_left(dict, key);
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
    _Optional StrDictItem *const new_array = realloc(
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
  StrDictItem *const array = &*dict->array;

  DEBUGF("Inserting item with key '%s', value %p at %zu\n", key, value,
         ins_index);
  dict->candidate = NULL;
  for (size_t i = nitems; i > ins_index; --i) {
    assert(i < dict->nalloc);
    assert(i > 0);
    array[i] = array[i - 1];
  }

  assert(ins_index < dict->nalloc);
  array[ins_index] = (StrDictItem){.key = key, .value = value};

  assert(dict->nitems < dict->nalloc);
  dict->nitems++;

#ifndef NDEBUG
  for (size_t i = 0; i < dict->nitems - 1; ++i) {
    assert(i + 1 < dict->nalloc);
    DEBUGF("%zu: Key %p:'%s', value %p\n", i, (void *)array[i].key,
           array[i].key, array[i].value);
    assert(stricmp(array[i].key, array[i + 1].key) <= 0);
  }
  DEBUGF("%zu: key %p:'%s', value %p\n", dict->nitems - 1,
         (void *)array[dict->nitems - 1].key,
         array[dict->nitems - 1].key, array[dict->nitems - 1].value);
#endif

  if (index) {
    *index = ins_index;
  }
  return true;
}

size_t strdict_bisect_left(StrDict *const dict, char const *const key)
{
  // Assertions here because this is the core implementation of most methods
  assert(key);
  assert(dict);
  assert(dict->nitems <= dict->nalloc);
  DEBUGF("Searching for lowest key >= '%s' in dictionary of size %zu\n",
         key, dict->nitems);

  if (!dict->array) {
    return 0;
  }
  StrDictItem const *const array = &*dict->array;

  size_t index = 0;

  if (dict->nitems > 0) {
    if (dict->candidate) {
      /* Can't just do a pointer comparison here because there could be a
         different string at a recycled address. Not sure how worthwhile
         this optimization is. */
      if (stricmp(STRING_OR_NULL(dict->sought_key), key) == 0) {
        DEBUGF("Reuse last result of search for key '%s'\n",
               dict->sought_key);
      } else {
        assert(dict->sought_key);
        DEBUGF("Discard last search result for '%s'\n", dict->sought_key);
        dict->candidate = NULL;
      }
    }

    if (!dict->candidate) {
      stringbuffer_truncate(&dict->buffer, 0);
      if (stringbuffer_append_all(&dict->buffer, key)) {
        dict->sought_key = stringbuffer_get_pointer(&dict->buffer);
      } else {
        dict->sought_key = key;
      }
      if (array) {
        (void)bsearch(dict, array, dict->nitems,
            sizeof(array[0]), compare_key_n_item);
      }
    }

    if (dict->candidate) {
      assert(dict->sought_key);
      assert(stricmp(STRING_OR_NULL(dict->sought_key), key) == 0);
      assert(dict->candidate >= array);
      assert(dict->candidate < array + dict->nitems);

      index = (size_t)(dict->candidate - array);
      assert(index < dict->nitems);
      if (stricmp(array[index].key, key) < 0) {
        DEBUGF("Candidate %p at index %zu with value %p was too low: '%s' < '%s'\n",
               (void *)dict->candidate, index, dict->candidate->value,
               dict->candidate->key, key);

        /* Search forward for the lowest key greater than or equal to
           the sought key */
        do {
          ++index;
        } while(index < dict->nitems &&
                stricmp(array[index].key, key) < 0);

      } else {
        DEBUGF("Candidate %p at index %zu with value %p was not too low: '%s' >= '%s'\n",
               (void *)dict->candidate, index, dict->candidate->value,
               dict->candidate->key, key);

        /* Search backward for the lowest key greater than or equal to
           the sought key */
        while (index > 0 && stricmp(array[index - 1].key, key) >= 0) {
          --index;
        }
      }

      if (index > 0) {
        assert(index - 1 < dict->nitems);
        assert(stricmp(array[index - 1].key, key) < 0);
      }
    } else {
      DEBUGF("No candidate\n");
    }

    if (dict->sought_key == key) {
      dict->sought_key = NULL; // Memory allocation failed earlier
      dict->candidate = NULL;
    }
  }

  DEBUGF("Key '%s' belongs at position %zu (current key '%s')\n", key, index,
         index < dict->nitems ? array[index].key : "MAX");

  assert(index <= dict->nitems);
  return index;
}

size_t strdict_bisect_right(StrDict *const dict, char const *const key)
{
  if (!dict->array) {
    return 0;
  }
  StrDictItem const *const array = &*dict->array;

  size_t index = strdict_bisect_left(dict, key);

  DEBUGF("Searching for lowest key > '%s' in dictionary of size %zu\n", key,
        dict->nitems);
  while (index < dict->nitems && stricmp(array[index].key, key) <= 0) {
    ++index;
  }

  return index;
}
