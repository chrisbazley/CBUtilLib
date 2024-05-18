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

/*
Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 07-Aug-18: Copied this source file from SF3KtoObj.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
*/

#ifndef ArgUtils_h
#define ArgUtils_h

#include <stddef.h>
#include <stdbool.h>

bool get_long_arg(const char *name, long int *value,
                  long int min, long int max,
                  int argc, const char *const argv[], int n);

bool get_double_arg(const char *name, double *value,
                    double min, double max,
                    int argc, const char *const argv[], int n);

bool is_switch(const char *arg, const char *name, size_t min);

#endif /* ArgUtils_h */
