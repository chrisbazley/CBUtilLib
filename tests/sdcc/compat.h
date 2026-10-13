/* Compact assertion reporting for the SDCC Z80 tests. */
#ifndef CBUTIL_SDCC_COMPAT_H
#define CBUTIL_SDCC_COMPAT_H
#include <assert.h>
void cbutil_sdcc_assert(unsigned int line);
/* Keep assertions active without storing every expression/function string
 * in the 16-bit address space. The test's name is printed before it runs. */
#undef assert
#define assert(...) ((__VA_ARGS__) ? (void)0 : cbutil_sdcc_assert(__LINE__))
#endif
