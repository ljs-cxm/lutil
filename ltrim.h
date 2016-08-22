#ifndef LUTIL_TRIM_H
#define LUTIL_TRIM_H

#include <ctype.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int lutil_trim(lua_State *L);
int lutil_ltrim(lua_State *L);
int lutil_rtrim(lua_State *L);

#endif

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
