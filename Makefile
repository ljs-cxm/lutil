# Include path where lua.h, luaconf.h and lauxlib.h reside:
INCLUDES= -I$(PWD) -I$(PWD)/lua5.1

# Lua executable name. Used to find the install path and for testing.
LUA= lua

CC= gcc
CCOPT= -O3 -Wall -pedantic -fomit-frame-pointer -Wall -DNDEBUG
CCWARN= -Wall
SOCC= $(CC) -shared
SOCFLAGS= -fPIC $(CCOPT) $(CCWARN) $(DEFINES) $(INCLUDES) $(CFLAGS)
SOLDFLAGS= -fPIC $(LDFLAGS)
RM= rm -f

DEP_TRIM= ltrim
DEP_SPLIT= lsplit
MODNAME= lutil
MODSO= $(MODNAME).so

all: $(MODSO)

# Alternative target for compiling on Mac OS X:
macosx:
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.4 $(CC) -dynamiclib -single_module -undefined dynamic_lookup"

$(DEP_TRIM).o: $(DEP_TRIM).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(DEP_SPLIT).o: $(DEP_SPLIT).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(MODNAME).o: $(MODNAME).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(MODSO): $(MODNAME).o $(DEP_TRIM).o $(DEP_SPLIT).o
	$(SOCC) $(SOLDFLAGS) -o $(MODSO) $^

test:
	lua test.lua

clean:
	$(RM) *.o *.so

.PHONY: all macosx test clean
