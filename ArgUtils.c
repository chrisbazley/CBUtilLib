/*
 * CBUtilLib: Command-line argument parser utilities
 * Copyright (C) 2018 Christopher Bazley
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
  CJB: 07-Aug-18: Copied this source file from SF3KtoObj.
  CJB: 17-Jun-23: Include "CBUtilMisc.h" last in case any of the other
                  included header files redefine macros such as assert().
*/

/* ISO library header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* My library files */
#include "ArgUtils.h"
#include "Internal/CBUtilMisc.h"

bool get_long_arg(const char *const name, long int *const value,
                  const long int min, const long int max,
                  int const argc, const char *const argv[],
                  int const n)
{
  assert(name != NULL);
  assert(*name != '\0');
  assert(value != NULL);
  assert(min <= max);
  assert(argc > 0);
  assert(argv != NULL);
  assert(n >= 0);

  if (n >= argc || argv[n][0] == '-') {
    fprintf(stderr, "Missing value for %s\n", name);
    return false;
  }

  char *endptr;
  *value = strtol(argv[n], &endptr, 0 /* integer constant */);
  if (*endptr != '\0') {
    fprintf(stderr, "Bad value for %s\n", name);
    return false;
  }

  if (*value < min || *value > max) {
    fprintf(stderr, "Value for %s is out of range %ld..%ld\n",
            name, min, max);
    return false;
  }

  return true;
}

bool get_double_arg(const char *const name, double *const value,
                    const double min, const double max,
                    int const argc, const char *const argv[],
                    int const n)
{
  assert(name != NULL);
  assert(*name != '\0');
  assert(value != NULL);
  assert(min <= max);
  assert(argc > 0);
  assert(argv != NULL);
  assert(n >= 0);

  if (n >= argc || argv[n][0] == '-') {
    fprintf(stderr, "Missing value for %s\n", name);
    return false;
  }

  char *endptr;
  *value = strtod(argv[n], &endptr);
  if (*endptr != '\0') {
    fprintf(stderr, "Bad value for %s\n", name);
    return false;
  }

  if (*value < min || *value > max) {
    fprintf(stderr, "Value for %s is out of range %f..%f\n",
            name, min, max);
    return false;
  }

  return true;
}

bool is_switch(const char * const arg, const char * const name,
               const size_t min)
{
  /* Allow any abbreviation of the canonical switch name
     no shorter than min characters */
  assert(arg != NULL);
  assert(name != NULL);
  assert(min > 0);

  const size_t arg_len = strlen(arg);

  return (arg_len >= min) &&
         (arg_len <= strlen(name)) &&
         (strncmp(arg, name, arg_len) == 0);
}
