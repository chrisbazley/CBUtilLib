/*
 * CBUtilLib test: main program
 * Copyright (C) 2012 Christopher Bazley
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

/* ISO library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Local headers */
#include "Tests.h"

int main(int argc, char *argv[])
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  }
  test_groups[] =
  {
    { "LinkedList", LinkedList_tests },
    { "StringBuffer", StringBuffer_tests },
    { "FileRWInt", FileRWInt_tests },
    { "IntDict", intdict_tests },
    { "StrDict", strdict_tests },
  };

  NOT_USED(argc);
  NOT_USED(argv);

  DEBUG_SET_OUTPUT(DebugOutput_StdOut, "");

  for (size_t count = 0; count < ARRAY_SIZE(test_groups); count ++)
  {
    /* Print title of this group of tests, then underline it */
    const size_t len = strlen(test_groups[count].test_name);
    puts(test_groups[count].test_name);
    for (size_t i = 0; i < len; i++)
        putchar('-');
    putchar('\n');

    /* Call a function to perform the group of tests */
    Fortify_EnterScope();
    test_groups[count].test_func();
    Fortify_LeaveScope();

    putchar('\n');
  }

  Fortify_OutputStatistics();

  return EXIT_SUCCESS;
}
