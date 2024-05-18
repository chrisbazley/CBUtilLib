/*
 * CBUtilLib: String buffer
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

/* StringBuff.h declares functions and types for dynamically-allocated
   extensible string buffers.

Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 07-Apr-12: Created this header file.
  CJB: 26-Nov-14: Truncate and append operations now remove any pending
                  undo even if they have no other effect.
  CJB: 30-Nov-14: Added stringbuffer_prepare_append and
                  stringbuffer_finish_append functions.
  CJB: 07-Aug-18: Added the stringbuffer_append_separated function.
  CJB: 05-Feb-19: Added the stringbuffer_append_all function.
  CJB: 10-Aug-22: Converted the most trivial functions into inline functions.
  CJB: 24-Sep-23: Added functions to append a formatted string.
 */

#ifndef StringBuff_h
#define StringBuff_h

/* ISO library headers */
#include <assert.h>
#include <stdint.h>
#include <stddef.h> /* (for size_t) */
#include <stdbool.h>
#include <stdarg.h>

typedef struct
{
  size_t  buffer_size; /* No. of bytes of memory allocated for the 'buffer'
                          array. Must be greater than string_len unless
                          string_len is 0. */
  size_t  string_len;  /* Length of the string in the 'buffer' array, not
                          including its nul terminator. */
  char   *buffer;      /* Pointer to a dynamically-allocated character array
                          containing the current string. May be NULL if
                          buffer_size and string_len are both 0. */
  size_t  undo_len; /* Length of the previous string in the 'buffer' array
                       or equal to string_len if not possible to undo. */
  char    undo_char; /* Character replaced by the nul terminator
                        (to allow truncation to be undone). */
}
StringBuffer;
   /*
    * Control structure for a string buffer (storage lifetime is under
    * client's control).
    */

void stringbuffer_init(StringBuffer * /*buffer*/);
   /*
    * Initializes a given string buffer. The caller must already have
    * allocated space for the 'buffer' control structure. The initial
    * content will be the empty string.
    */

char *stringbuffer_prepare_append(StringBuffer * /*buffer*/,
                                  size_t       * /*min_size*/);
   /*
    * Prepares to append at the end of the current string in a given buffer
    * by reserving space for at least 'min_size' bytes (which must include
    * space for a nul terminator). The buffer will be enlarged if necessary.
    * If successful then 'min_size' may be increased to reflect the actual
    * amount of space available at the returned address.
    * Returns: pointer to the place at which to append characters, or a null
    * pointer if additional space was required but could not be allocated.
    */

void stringbuffer_finish_append(StringBuffer * /*buffer*/, size_t /*n*/);
   /* Finishes appending 'n' bytes (not including nul terminator) at the
    * end of the current string in a given buffer. Even if no characters
    * are appended, calling this function makes it impossible to undo any
    * previous append or truncation operation.
    */

bool stringbuffer_append(StringBuffer * /*buffer*/,
                         const char   * /*tail*/,
                         size_t         /*n*/);
   /*
    * Appends up to 'n' characters from 'tail' at the end of the current
    * string in a given buffer. The buffer will be enlarged if necessary.
    * If 'n' is 0 then 'tail' can be a null pointer. Even if no characters
    * are appended, calling this function makes it impossible to undo any
    * previous append or truncation operation.
    * Returns: true if successful, or false if additional space was required
    *          but could not be allocated.
    */

static inline bool stringbuffer_append_all(StringBuffer *const buffer,
  const char *const tail)
{
  return stringbuffer_append(buffer, tail, SIZE_MAX);
}
   /*
    * Appends all characters from 'tail' at the end of the current
    * string in a given buffer. The buffer will be enlarged if necessary.
    * Even if no characters are appended, calling this function makes it
    * impossible to undo any previous append or truncation operation.
    * Returns: true if successful, or false if additional space was required
    *          but could not be allocated.
    */

bool stringbuffer_append_separated(StringBuffer * /*buffer*/,
                                   const char     /*sep*/,
                                   const char   * /*tail*/);
   /*
    * Appends a given separator character followed by all characters of
    * 'tail' at the end of the current string in a given buffer. The buffer
    * will be enlarged if necessary.
    * On failure, the string is unmodified. On success, the effects of this
    * function can be undone atomically (unlike multiple calls to 'append').
    * Returns: true if successful, or false if additional space was required
    *          but could not be allocated.
    */

bool stringbuffer_vprintf(StringBuffer * /*buffer*/,
                          const char * /*format*/,
                          va_list /*args*/);
   /*
    * Appends a string formatted according to 'format' with parameter
    * substitution from 'args' at the end of the current string in a given
    * buffer. The buffer will be enlarged if necessary. On failure, the
    * string is unmodified. Even if no characters are appended, calling this
    * function makes it impossible to undo any previous append or truncation
    * operation.
    * Returns: true if successful, or false if additional space was required
    *          but could not be allocated.
    */

static inline bool stringbuffer_printf(StringBuffer *const buffer,
                                       const char *const format,
                                       ...)
{
  va_list args;
  va_start(args, format);
  bool const success = stringbuffer_vprintf(buffer, format, args);
  va_end(args);
  return success;
}
   /*
    * Like stringbuffer_vprintf except that it takes a variable number of
    * arguments to be substituted into the appended string.
    * Returns: true if successful, or false if additional space was required
    *          but could not be allocated.
    */

void stringbuffer_truncate(StringBuffer * /*buffer*/, size_t /*len*/);
   /*
    * Truncates the current string in a given buffer after 'len' characters.
    * If the string is already less than or equal to 'len' characters in
    * length then it is not truncated. Even if no truncation occurs, calling
    * this function makes it impossible to undo any previous append or
    * truncation operation.
    */

static inline size_t stringbuffer_get_length(
  const StringBuffer *const buffer)
{
  assert(buffer != NULL);

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->string_len] == '\0');
  }

  return buffer->string_len;
}
   /*
    * Gets the length in characters of the current string in a given buffer,
    * not including nul terminator.
    */

static inline char *stringbuffer_get_pointer(
     const StringBuffer *const buffer)
{
  assert(buffer != NULL);

  /* If the string buffer contains the empty string "" then the length
     may be zero (no allocated memory) as a special case. */
  if (buffer->string_len > 0)
  {
    assert(buffer->buffer != NULL);
    assert(buffer->string_len < buffer->buffer_size);
    assert(buffer->buffer[buffer->string_len] == '\0');
  }

  return buffer->buffer ? buffer->buffer : "";
}
   /*
    * Returns a direct pointer to the current string in a given buffer. May
    * instead return a pointer to a string literal if the length is 0.
    * Should not be used to modify the buffer's contents unless restored
    * (e.g. as in make_path).
    */

void stringbuffer_minimize(StringBuffer * /*buffer*/);
   /*
    * Attempts to minimize the amount of space used by a given string buffer.
    * Calling this function makes it impossible to undo any previous append
    * or truncation operation.
    */

void stringbuffer_undo(StringBuffer * /*buffer*/);
   /*
    * Undoes the last append or truncation operation on a given string
    * buffer. Attempting to undo after minimizing the buffer, or after
    * extending or truncating a string by 0 characters, has no effect.
    * Undoing the same operation multiple times also has no effect.
    */

void stringbuffer_destroy(StringBuffer * /*buffer*/);
   /*
    * Frees any additional space allocated for the content of a given string
    * buffer.
    */

#endif
