# Project:   CBUtilLibTests

# Tools
CC = gcc
Link = gcc

# Toolflags:
CCFlags = -c -I.. -Wall -Wextra -Wsign-conversion -pedantic -std=c99 -g -DDEBUG_OUTPUT -DDEBUG_DUMP -MMD -MP -o $@
LinkFlags = -L.. -lCBUtildbg -o $@

include MakeCommon

Objects = $(addsuffix .o,$(ObjectList))

# Final targets:
Tests: $(Objects)
	$(Link) $(Objects) $(LinkFlags)

# User-editable dependencies:
.SUFFIXES: .o .c
.c.o:
	${CC} $(CCFlags) -MF $*.d $<

# These files are generated during compilation to track C header #includes.
# It's not an error if they don't exist.
-include $(addsuffix .d,$(ObjectList))
