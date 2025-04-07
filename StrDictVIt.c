/*
 * CBUtilLib: String dictionary value iterator
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
#include "StrDict.h"
#include "Internal/CBUtilMisc.h"

_Optional void *strdictviter_init(StrDictVIter *const iter, StrDict *const dict,
  char const *const min_key, char const *const max_key)
{
  assert(iter);
  assert(dict);
  assert(min_key);
  assert(max_key);
  DEBUGF("Start iterating over values in dictionary %p of L%zu "
         "within range (%s,%s)\n",
         (void *)dict, dict->nitems, min_key, max_key);

  *iter = (StrDictVIter){
    .dict = dict,
    .next_index = strdict_bisect_left(dict, min_key),
    .end = strdict_bisect_right(dict, max_key),
  };

  return strdictviter_advance(iter);
}

_Optional void *strdictviter_all_init(StrDictVIter *const iter,
                                      StrDict *const dict)
{
  assert(iter);
  assert(dict);
  DEBUGF("Start iterating over values in dictionary %p of %zu\n",
          (void *)dict, strdict_count(dict));

  *iter = (StrDictVIter){
    .dict = dict,
    .next_index = 0,
    .end = strdict_count(dict),
  };

  return strdictviter_advance(iter);
}

_Optional void *strdictviter_advance(StrDictVIter *const iter)
{
  assert(iter);
  assert(iter->dict);

  if (iter->next_index < iter->end) {
    DEBUGF("Advanced to index %zu\n", iter->next_index);
    return strdict_get_value_at(iter->dict, iter->next_index++);
  }

  DEBUGF("Cannot advance\n");
  return NULL; /* finished */
}

size_t strdictviter_remove(StrDictVIter *const iter)
{
  assert(iter);
  assert(iter->dict);
  assert(iter->next_index > 0);
  assert(iter->end > 0);
  --iter->end;
  size_t const index = --iter->next_index;
  strdict_remove_at(iter->dict, index);
  return index;
}
