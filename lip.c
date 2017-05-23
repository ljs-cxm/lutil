#include <ctype.h>
#include <stdint.h>
#include "lip.h"

#define LUA_INVALID_IP \
  lua_pushnil(L);      \
  lua_pushstring(L, "invalid ip")

int lutil_ip_to_number(lua_State *L) {
  const char *ip;
  size_t ip_len = 0;

  ip = luaL_checklstring(L, 1, &ip_len);

  uint32_t bytes[4];
  size_t i = 0, j = 0, idx = 0;

  for (; i < ip_len; i = j + 1) {
    if (idx > 3) {
      LUA_INVALID_IP;
      return 2;
    }

    uint32_t byte = 0;
    for (j = i; j < ip_len && '0' <= ip[j] && ip[j] <= '9'; ++j) {
      byte = byte * 10 + ip[j] - '0';
    }

    if (j == i || j - i > 3 || (j - i > 1 && ip[i] == '0') ||
        (j < ip_len && ip[j] != '.')) {
      LUA_INVALID_IP;
      return 2;
    }

    if (byte > 255) {
      LUA_INVALID_IP;
      return 2;
    }

    bytes[idx++] = byte;
  }

  if (idx != 4 || !(ip_len && ip[ip_len - 1] != '.')) {
    LUA_INVALID_IP;
    return 2;
  }

  uint32_t num = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];

  lua_pushnumber(L, num);

  return 1;
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
