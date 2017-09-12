# Include path where lua.h, luaconf.h and lauxlib.h reside:
INCLUDES= -I$(PWD) -I$(PWD)/lua5.1

# Lua executable name. Used to find the install path and for testing.
LUA= lua

CC= gcc
CCOPT= -O3 -std=c99 -Wall -pedantic -fomit-frame-pointer -Wall -DNDEBUG
CCWARN= -Wall
SOCC= $(CC) -shared
SOCFLAGS= -fPIC $(CCOPT) $(CCWARN) $(DEFINES) $(INCLUDES) $(CFLAGS)
SOLDFLAGS= -fPIC $(LDFLAGS)
RM= rm -f

DEP_IP= lip
DEP_TRIM= ltrim
DEP_TIME= ltime
DEP_SPLIT= lsplit
MODNAME= lutil
MODSO= $(MODNAME).so
TARGET= all

XXHASH= xxhash
BLOOM_FILTER_NAME= lbloom_filter
BLOOM_FILTER_MOD= $(BLOOM_FILTER_NAME).so

all: $(MODSO) $(BLOOM_FILTER_MOD)

# Alternative target for compiling on Mac OS X:
macosx:
	$(MAKE) $(MODSO) "SOCC=MACOSX_DEPLOYMENT_TARGET=10.4 $(CC) -dynamiclib -single_module -undefined dynamic_lookup"
	$(MAKE) $(BLOOM_FILTER_MOD) "SOCC=MACOSX_DEPLOYMENT_TARGET=10.4 $(CC) -dynamiclib -single_module -undefined dynamic_lookup"

$(DEP_IP).o: $(DEP_IP).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(DEP_TRIM).o: $(DEP_TRIM).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(DEP_TIME).o: $(DEP_TIME).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(DEP_SPLIT).o: $(DEP_SPLIT).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(MODNAME).o: $(MODNAME).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(MODSO): $(MODNAME).o $(DEP_IP).o $(DEP_TRIM).o $(DEP_TIME).o $(DEP_SPLIT).o
	$(SOCC) $(SOLDFLAGS) -o $@ $^

$(BLOOM_FILTER_NAME).o: $(BLOOM_FILTER_NAME).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(XXHASH).o: $(XXHASH).c
	$(CC) $(SOCFLAGS) -c -o $@ $<

$(BLOOM_FILTER_MOD): $(BLOOM_FILTER_NAME).o $(XXHASH).o
	$(SOCC) $(SOLDFLAGS) -o $@ $^

test:
	lua test.lua

clean:
	$(RM) *.o *.so

.PHONY: all macosx test clean
