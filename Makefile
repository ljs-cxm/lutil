# Include path where lua.h, luaconf.h and lauxlib.h reside:
INCLUDES= -I$(PWD)

# Lua executable name. Used to find the install path and for testing.
LUA= lua

CC= gcc
CCOPT= -O2 -fomit-frame-pointer
CCWARN= -Wall
SOCC= $(CC) -shared
SOCFLAGS= -fPIC $(CCOPT) $(CCWARN) $(DEFINES) $(INCLUDES) $(CFLAGS)
SOLDFLAGS= -fPIC $(LDFLAGS)
RM= rm -f

MODNAME= lutil
MODSO= $(MODNAME).so

all: $(MODSO)

# Alternative target for compiling on Mac OS X:
macosx:
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.4 $(CC) -dynamiclib -single_module -undefined dynamic_lookup"

$(MODNAME).o: $(MODNAME).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(MODSO): $(MODNAME).o
	$(SOCC) $(SOLDFLAGS) -o $(MODSO) $^

test:
	lua test.lua

clean:
	$(RM) *.o *.so

.PHONY: all macosx test clean
