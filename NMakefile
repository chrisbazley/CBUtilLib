# Project:   CBUtilLib
include MakeCommon

# Tools
CC = cc
LibFile = libfile

# Toolflags:
CCCommonFlags =  -c -depend !Depend -IC: -throwback -fahi -apcs 3/32/fpe2/swst/fp/nofpr -memaccess -L22-S22-L41 -o $@
CCFlags = $(CCCommonFlags) -DNDEBUG -Otime
CCDebugFlags = $(CCCommonFlags) -g -DUSE_CBDEBUG -DDEBUG_OUTPUT -DFORTIFY 
CCModuleFlags = $(CCCommonFlags) -DNDEBUG -Ospace -zM -zps1 -ff
LibFileFlags = -c -o $@

# Acorn Make doesn't find object files in subdirectories if referenced by
# non-standard file name suffixes so use addprefix not addsuffix here
ReleaseObjects = $(addprefix o.,$(ObjectList))
DebugObjects = $(addprefix debug.,$(ObjectList))
ModuleObjects = $(addprefix oz.,$(ObjectList))

# Final targets:
all: @.debug.$(LibName)Lib \
     @.o.$(LibName)Lib \
     @.oz.$(LibName)Lib

@.o.$(LibName)Lib: $(ReleaseObjects)
	$(LibFile) $(LibFileFlags) $(ReleaseObjects)

@.debug.$(LibName)Lib: $(DebugObjects)
	$(LibFile) $(LibFileFlags) $(DebugObjects)

@.oz.$(LibName)Lib: $(ModuleObjects)
	$(LibFile) $(LibFileFlags) $(ModuleObjects)

# User-editable dependencies:
.SUFFIXES: .o .c .debug .oz
.c.o:; ${CC} $(CCFlags) $<
.c.oz:; ${CC} $(CCModuleFlags) $<
.c.debug:; ${CC} $(CCDebugFlags) $<

# Dynamic dependencies:
