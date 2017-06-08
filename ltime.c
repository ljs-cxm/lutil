#include <time.h>
#include "ltime.h"

static int getboolfield(lua_State *L, const char *key) {
  int res;
  lua_getfield(L, -1, key);
  res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}

static int getfield(lua_State *L, const char *key, int d) {
  int res;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    res = (int)lua_tointeger(L, -1);
  else {
    if (d < 0)
      return luaL_error(L, "field " LUA_QS " missing in date table", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}

// https://github.com/mnp/libfast-mktime/blob/master/fast-mktime.c
time_t cached_mktime(struct tm *tm) {
  static struct tm cache;
  static time_t time_cache = 0;
  time_t result;
  time_t carry;

  /* the epoch time portion of the request */
  carry = 3600 * tm->tm_hour + 60 * tm->tm_min + tm->tm_sec;

  if (cache.tm_mday == tm->tm_mday && cache.tm_mon == tm->tm_mon &&
      cache.tm_year == tm->tm_year) {
    result = time_cache + carry;
  } else {
    cache.tm_mday  = tm->tm_mday;
    cache.tm_mon   = tm->tm_mon;
    cache.tm_year  = tm->tm_year;
    time_cache     = mktime(&cache);
    cache.tm_isdst = 0;

    result = (-1 == time_cache) ? -1 : time_cache + carry;
  }

  return result;
}

int lutil_os_time(lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1)) /* called without args? */
    t = time(NULL);          /* get current time */
  else {
    struct tm ts;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1); /* make sure table is at the top */
    ts.tm_sec   = getfield(L, "sec", 0);
    ts.tm_min   = getfield(L, "min", 0);
    ts.tm_hour  = getfield(L, "hour", 12);
    ts.tm_mday  = getfield(L, "day", -1);
    ts.tm_mon   = getfield(L, "month", -1) - 1;
    ts.tm_year  = getfield(L, "year", -1) - 1900;
    ts.tm_isdst = getboolfield(L, "isdst");
    t           = cached_mktime(&ts);
  }
  if (t == (time_t)(-1))
    lua_pushnil(L);
  else
    lua_pushnumber(L, (lua_Number)t);
  return 1;
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
