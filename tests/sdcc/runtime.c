/* Copyright 2026 Christopher Bazley
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>

/* A 4 KiB heap, separated from static data and the stack. */
unsigned char __at (0xe000) __sdcc_heap[0x1000];
unsigned char __at (0xf000) __sdcc_heap_end;

/* uCsim interface configured by the CTest runner. */
volatile unsigned char __at (0xdf00) simif;

int putchar(int c)
{
  simif = 'w';
  simif = (unsigned char)c;
  return (unsigned char)c;
}

static void stop(void)
{
  simif = 's';
  for (;;) {}
}

void abort(void)
{
  puts("CBUTIL TEST ABORT");
  stop();
}

/* Stop immediately on failure; a simulator exit alone is not a test pass. */
void cbutil_sdcc_assert(unsigned int line)
{
  printf("ASSERT FAIL at line %u\n", line);
  stop();
}

void TEST_FUNCTION(void);
void __sdcc_heap_init(void);

int main(void)
{
  /* Our fixed-address heap replaces the library heap and its startup hook. */
  __sdcc_heap_init();
  TEST_FUNCTION();
  puts("CBUTIL TEST PASS");
  stop();
  return 0;
}
