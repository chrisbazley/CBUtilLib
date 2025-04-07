# CBUtilLib
(C) 2018 Christopher Bazley

Release 12 (08 Apr 2025)

Introduction
------------
  This C library provides miscellaneous additional functionality not found
in the standard C library such as linked lists, string buffers and case-
insensitive string comparisons. It should have minimal dependencies on other
libraries and must not include any platform-specific code.

Fortified memory allocation
---------------------------
  I use Simon's P. Bullen's fortified memory allocation shell 'Fortify' to
find memory leaks in my applications, detect corruption of the heap
(e.g. caused by writing beyond the end of a heap block), and do stress
testing (by causing some memory allocations to fail). Fortify is available
separately from this web site:
http://web.archive.org/web/20020615230941/www.geocities.com/SiliconValley/Horizon/8596/fortify.html

  The debugging version of CBUtilLib must be linked with 'Fortify', for
example by adding 'C:o.Fortify' to the list of object files specified to the
linker. Otherwise, you will get build-time errors like this:
```
ARM Linker: (Error) Undefined symbol(s).
ARM Linker:     Fortify_malloc, referred to from
C:debug.CBUtilLib(ReaderGKey).
ARM Linker:     Fortify_free, referred to from C:debug.CBUtilLib(ReaderGKey).
```
Rebuilding the library
----------------------
  You should ensure that the standard C library and CBDebugLib (by the same
author as CBUtilLib) are on your header include path (C$Path if using the
supplied make files on RISC OS), otherwise the compiler won't be able to find
the required header files. The dependency on CBDebugLib isn't very strong: it
can be eliminated by modifying the make file so that the macro USE_CBDEBUG is
no longer predefined.

  Three make files are supplied:

- 'Makefile' is intended for use with GNU Make and the GNU C Compiler
  on Linux.
- 'NMakefile' is intended for use with Acorn Make Utility (AMU) and the
  Norcroft C compiler supplied with the Acorn C/C++ Development Suite.
- 'GMakefile' is intended for use with GNU Make and the GNU C Compiler
  on RISC OS.

These make files share some variable definitions (lists of objects to be
built) by including a common make file.

  The APCS variant specified for the Norcroft compiler is 32 bit for
compatibility with ARMv5 and fpe2 for compatibility with older versions of
the floating point emulator. Generation of unaligned data loads/stores is
disabled for compatibility with ARM v6.

  The suffix rules generate output files with different suffixes (or in
different subdirectories, if using the supplied make files on RISC OS),
depending on the compiler options used to compile the source code:

o: Assertions and debugging output are disabled. The code is optimised for
   execution speed.

oz: Assertions and debugging output are disabled. The code is suitable for
    inclusion in a relocatable module (multiple instantiation of static
    data and stack limit checking disabled). When the Norcroft compiler is
    used, the compiler optimises for smaller code size. (The equivalent GCC
    option seems to be broken.)

debug: Assertions and debugging output are enabled. The code includes
       symbolic debugging data (e.g. for use with DDT). The macro FORTIFY
       is pre-defined to enable Simon P. Bullen's fortified shell for memory
       allocations.

d: 'GMakefile' passes '-MMD' when invoking gcc so that dynamic dependencies
   are generated from the #include commands in each source file and output
   to a temporary file in the directory named 'd'. GNU Make cannot
   understand rules that contain RISC OS paths such as /C:Macros.h as
   prerequisites, so 'sed', a stream editor, is used to strip those rules
   when copying the temporary file to the final dependencies file.

  The above suffixes must be specified in various system variables which
control filename suffix translation on RISC OS, including at least
UnixEnv$ar$sfix, UnixEnv$gcc$sfix and UnixEnv$make$sfix.
Unfortunately GNU Make doesn't apply suffix rules to make object files in
subdirectories referenced by path even if the directory name is in
UnixEnv$make$sfix, which is why 'GMakefile' uses the built-in function
addsuffix instead of addprefix to construct lists of the objects to be
built (e.g. foo.o instead of o.foo).

  Before compiling the library for RISC OS, move the C source and header
files with .c and .h suffixes into subdirectories named 'c' and 'h' and
remove those suffixes from their names. You probably also need to create
'o', 'oz', 'd' and 'debug' subdirectories for compiler output.

Licence and disclaimer
----------------------
  This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

  This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
for more details.

  You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Credits
-------
  This library was derived from CBLibrary. Both libraries are free software,
available under the same license.

  The implementations of functions strdup(), stricmp() and strnicmp() were
copied from UnixLib, which is copyright (c) 1995-1999 Simon Callan, Nick
Burrett, Nicholas Clark and Peter Burwood. Portions are (c) The Regents of
the University of California, portions are (c) Sun Microsystems, Inc. and
portions are derived from the GNU C Library and fall under the GNU Library
General Public License.

History
-------
Release 1 (04 Nov 2018)
- Extracted the relevant components from CBLib to make a standalone library.

Release 2 (02 Nov 2019)
- Modified fread_int32le not to rely on undefined behaviour caused by
  unrepresentable results of left-shifting a signed integer type.
- Added tests for the fread_int32le and fwrite_int32le functions.
- Recategorised more debugging output as verbose.
- Added a simpler function to append whole strings to a string buffer.

Release 3 (30 Sep 2020)
- Debugging output is less verbose by default.
- Minor refactoring of trigonometric look-up code.

Release 4 (27 Jul 2022)
- Removed redundant uses of the 'extern' keyword.
- Added 'const' qualifiers to arguments in function definitions.
- Validate whole linked list after insertion or removal of an item.
- Assertions to detect null pointer arguments to string comparison functions.
- New macro definitions to help iterate over linked lists.
- Replaced function-like macros for linked lists with inline functions.

Release 5 (10 Aug 2022)
- Converted the trivial string buffer functions into inline functions.
- Added string and integer dictionaries, with unit tests.

Release 6 (14 Aug 2022)
- Added iterator interfaces for the string and integer dictionaries.
- Dictionary functions now use size_t instead of int for array indices
  and return Boolean success/failure instead of -1.
- Consequently the 'contains' functions of dictionaries were subsumed into
  the 'find' functions.
- Exposed the ability to find a dictionary item with a specific value as
  a new function.
- The STRDICT_FOR_EACH and INTDICT_FOR_EACH macros now require an extra
  parameter to make it clearer when the limit variable shadows another
  variable declaration.
- The LINKEDLIST_FOR_EACH_SAFE macro now requires an extra parameter to make
  it clear when the next item pointer shadows another variable declaration.
- Updated several copyright headers which originally belonged to SFeditor.
- Added a missing include for assertions in inline functions for the
  integer dictionary.
- Documented the existing behaviour of strdup when passed a null pointer.
- Changed the return type of strinflate from int to size_t.
- Made conversion of pointer difference from signed to unsigned explicit.
- Minimized variable scope and mutability in strinflate and strdup.
- Assert valid parameter values and make sign conversion explicit in
  TrigTable_make.

Release 7 (17 Jun 2023)
- Fix text of debug output from the intdict_find_specific and
  strdict_find_specific functions.
- Ensure header "CBUtilMisc.h" is included last in source files in case
  any of the other included header files redefine macros such as assert().

Release 8 (17 Nov 2023)
- Added functions to append a formatted string to the existing content of a
  string buffer.
- Modified csv_parse_string to use strtol and strtod instead of atoi, atof
  and atod to avoid undefined behaviour if the value is unrepresentable.
- Added type-safe veneers which call the csv_parse_string function, e.g.
  csv_parse_as_int.

Release 9 (03 Dec 2023)
- Added missing source files for string buffers.

Release 10 (18 May 2024)
- Corrected description of the return value of strdict_remove and a
  test for the same.
- Added #ifdefs to make the tests pass when built without Fortify.
- Added new makefiles for use on Linux.

Release 11 (19 May 2024)
- Improved the README.md file for Linux users.

Release 12 (08 Apr 2025)
- Dogfooding the _Optional qualifier.

Contact details
---------------
Christopher Bazley

Email: mailto:cs99cjb@gmail.com

WWW:   http://starfighter.acornarcade.com/mysite/
