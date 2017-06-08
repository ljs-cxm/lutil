#include "lua.h"
#include "lauxlib.h"

#include "lip.h"
#include "ltrim.h"
#include "ltime.h"
#include "lsplit.h"

#define LUTIL_VERSION "0.0.1"

static const struct luaL_Reg lutil_lib[] = {
    {"trim", lutil_trim},
    {"ltrim", lutil_ltrim},
    {"rtrim", lutil_rtrim},
    {"split", lutil_split},
    {"os_time", lutil_os_time},
    {"ip_to_number", lutil_ip_to_number},
    {NULL, NULL}};

int luaopen_lutil(lua_State *L) {
#if LUA_VERSION_NUM < 502
  luaL_register(L, "lutil", lutil_lib);
#else
  luaL_newlib(L, lutil_lib);
#endif

  return 1;
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
