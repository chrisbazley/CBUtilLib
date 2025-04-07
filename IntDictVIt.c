/*
 * CBUtilLib: Integer dictionary value iterator
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
  CJB: 11-Aug-22: Created this source file.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
  CJB: 07-Apr-25: Dogfooding the _Optional qualifier.
 */

/* Local headers */
#include "IntDict.h"
#include "Internal/CBUtilMisc.h"

_Optional void *intdictviter_init(IntDictVIter *const iter, IntDict *const dict,
  IntDictKey const min_key, IntDictKey const max_key)
{
  assert(iter);
  assert(dict);
  DEBUGF("Start iterating over values in dictionary %p of %zu "
         "within range (%ld,%ld)\n",
         (void *)dict, intdict_count(dict), min_key, max_key);

  *iter = (IntDictVIter){
    .dict = dict,
    .next_index = intdict_bisect_left(dict, min_key),
    .end = intdict_bisect_right(dict, max_key),
  };

  return intdictviter_advance(iter);
}

_Optional void *intdictviter_all_init(IntDictVIter *const iter,
                                      IntDict *const dict)
{
  assert(iter);
  assert(dict);
  DEBUGF("Start iterating over values in dictionary %p of %zu\n",
          (void *)dict, intdict_count(dict));

  *iter = (IntDictVIter){
    .dict = dict,
    .next_index = 0,
    .end = intdict_count(dict),
  };

  return intdictviter_advance(iter);
}

_Optional void *intdictviter_advance(IntDictVIter *const iter)
{
  assert(iter);
  assert(iter->dict);

  if (iter->next_index < iter->end) {
    DEBUGF("Advanced to index %zu\n", iter->next_index);
    return intdict_get_value_at(iter->dict, iter->next_index++);
  }

  DEBUGF("Cannot advance\n");
  return NULL; /* finished */
}

size_t intdictviter_remove(IntDictVIter *const iter)
{
  assert(iter);
  assert(iter->dict);
  assert(iter->next_index > 0);
  assert(iter->end > 0);
  --iter->end;
  size_t const index = --iter->next_index;
  intdict_remove_at(iter->dict, index);
  return index;
}
