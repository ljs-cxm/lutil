#include "lsplit.h"

/* macro to `unsign' a character */
#define uchar(c) ((unsigned char)(c))

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/

#define CAP_UNFINISHED (-1)
#define CAP_POSITION (-2)

typedef struct MatchState {
  const char *src_init; /* init of source string */
  const char *src_end;  /* end (`\0') of source string */
  lua_State *L;
  int level; /* total number of captures (finished or unfinished) */
  struct {
    const char *init;
    ptrdiff_t len;
  } capture[LUA_MAXCAPTURES];
} MatchState;

#define L_ESC '%'
#define SPECIALS "^$*+?.([%-"

static int check_capture(MatchState *ms, int l) {
  l -= '1';
  if (l < 0 || l >= ms->level || ms->capture[l].len == CAP_UNFINISHED)
    return luaL_error(ms->L, "invalid capture index");
  return l;
}

static int capture_to_close(MatchState *ms) {
  int level = ms->level;
  for (level--; level >= 0; level--)
    if (ms->capture[level].len == CAP_UNFINISHED)
      return level;
  return luaL_error(ms->L, "invalid pattern capture");
}

static const char *classend(MatchState *ms, const char *p) {
  switch (*p++) {
  case L_ESC: {
    if (*p == '\0')
      luaL_error(ms->L, "malformed pattern (ends with " LUA_QL("%%") ")");
    return p + 1;
  }
  case '[': {
    if (*p == '^')
      p++;
    do { /* look for a `]' */
      if (*p == '\0')
        luaL_error(ms->L, "malformed pattern (missing " LUA_QL("]") ")");
      if (*(p++) == L_ESC && *p != '\0')
        p++; /* skip escapes (e.g. `%]') */
    } while (*p != ']');
    return p + 1;
  }
  default: { return p; }
  }
}

static int match_class(int c, int cl) {
  int res;
  switch (tolower(cl)) {
  case 'a':
    res = isalpha(c);
    break;
  case 'c':
    res = iscntrl(c);
    break;
  case 'd':
    res = isdigit(c);
    break;
  case 'l':
    res = islower(c);
    break;
  case 'p':
    res = ispunct(c);
    break;
  case 's':
    res = isspace(c);
    break;
  case 'u':
    res = isupper(c);
    break;
  case 'w':
    res = isalnum(c);
    break;
  case 'x':
    res = isxdigit(c);
    break;
  case 'z':
    res = (c == 0);
    break;
  default:
    return (cl == c);
  }
  return (islower(cl) ? res : !res);
}

static int matchbracketclass(int c, const char *p, const char *ec) {
  int sig = 1;
  if (*(p + 1) == '^') {
    sig = 0;
    p++; /* skip the `^' */
  }
  while (++p < ec) {
    if (*p == L_ESC) {
      p++;
      if (match_class(c, uchar(*p)))
        return sig;
    } else if ((*(p + 1) == '-') && (p + 2 < ec)) {
      p += 2;
      if (uchar(*(p - 2)) <= c && c <= uchar(*p))
        return sig;
    } else if (uchar(*p) == c)
      return sig;
  }
  return !sig;
}

static int singlematch(int c, const char *p, const char *ep) {
  switch (*p) {
  case '.':
    return 1; /* matches any char */
  case L_ESC:
    return match_class(c, uchar(*(p + 1)));
  case '[':
    return matchbracketclass(c, p, ep - 1);
  default:
    return (uchar(*p) == c);
  }
}

static const char *match(MatchState *ms, const char *s, const char *p);

static const char *matchbalance(MatchState *ms, const char *s, const char *p) {
  if (*p == 0 || *(p + 1) == 0)
    luaL_error(ms->L, "unbalanced pattern");
  if (*s != *p)
    return NULL;
  else {
    int b = *p;
    int e = *(p + 1);
    int cont = 1;
    while (++s < ms->src_end) {
      if (*s == e) {
        if (--cont == 0)
          return s + 1;
      } else if (*s == b)
        cont++;
    }
  }
  return NULL; /* string ends out of balance */
}

static const char *max_expand(MatchState *ms, const char *s, const char *p,
                              const char *ep) {
  ptrdiff_t i = 0; /* counts maximum expand for item */
  while ((s + i) < ms->src_end && singlematch(uchar(*(s + i)), p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  while (i >= 0) {
    const char *res = match(ms, (s + i), ep + 1);
    if (res)
      return res;
    i--; /* else didn't match; reduce 1 repetition to try again */
  }
  return NULL;
}

static const char *min_expand(MatchState *ms, const char *s, const char *p,
                              const char *ep) {
  for (;;) {
    const char *res = match(ms, s, ep + 1);
    if (res != NULL)
      return res;
    else if (s < ms->src_end && singlematch(uchar(*s), p, ep))
      s++; /* try with one more repetition */
    else
      return NULL;
  }
}

static const char *start_capture(MatchState *ms, const char *s, const char *p,
                                 int what) {
  const char *res;
  int level = ms->level;
  if (level >= LUA_MAXCAPTURES)
    luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level + 1;
  if ((res = match(ms, s, p)) == NULL) /* match failed? */
    ms->level--;                       /* undo capture */
  return res;
}

static const char *end_capture(MatchState *ms, const char *s, const char *p) {
  int l = capture_to_close(ms);
  const char *res;
  ms->capture[l].len = s - ms->capture[l].init; /* close capture */
  if ((res = match(ms, s, p)) == NULL)          /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;        /* undo capture */
  return res;
}

static const char *match_capture(MatchState *ms, const char *s, int l) {
  size_t len;
  l = check_capture(ms, l);
  len = ms->capture[l].len;
  if ((size_t)(ms->src_end - s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    return s + len;
  else
    return NULL;
}

static const char *match(MatchState *ms, const char *s, const char *p) {
init: /* using goto's to optimize tail recursion */
  switch (*p) {
  case '(': {            /* start capture */
    if (*(p + 1) == ')') /* position capture? */
      return start_capture(ms, s, p + 2, CAP_POSITION);
    else
      return start_capture(ms, s, p + 1, CAP_UNFINISHED);
  }
  case ')': { /* end capture */
    return end_capture(ms, s, p + 1);
  }
  case L_ESC: {
    switch (*(p + 1)) {
    case 'b': { /* balanced string? */
      s = matchbalance(ms, s, p + 2);
      if (s == NULL)
        return NULL;
      p += 4;
      goto init; /* else return match(ms, s, p+4); */
    }
    case 'f': { /* frontier? */
      const char *ep;
      char previous;
      p += 2;
      if (*p != '[')
        luaL_error(ms->L, "missing " LUA_QL("[") " after " LUA_QL(
                              "%%f") " in pattern");
      ep = classend(ms, p); /* points to what is next */
      previous = (s == ms->src_init) ? '\0' : *(s - 1);
      if (matchbracketclass(uchar(previous), p, ep - 1) ||
          !matchbracketclass(uchar(*s), p, ep - 1))
        return NULL;
      p = ep;
      goto init; /* else return match(ms, s, ep); */
    }
    default: {
      if (isdigit(uchar(*(p + 1)))) { /* capture results (%0-%9)? */
        s = match_capture(ms, s, uchar(*(p + 1)));
        if (s == NULL)
          return NULL;
        p += 2;
        goto init; /* else return match(ms, s, p+2) */
      }
      goto dflt; /* case default */
    }
    }
  }
  case '\0': { /* end of pattern */
    return s;  /* match succeeded */
  }
  case '$': {
    if (*(p + 1) == '\0') /* is the `$' the last char in pattern? */
      return (s == ms->src_end) ? s : NULL; /* check end of string */
    else
      goto dflt;
  }
  default:
  dflt : {                            /* it is a pattern item */
    const char *ep = classend(ms, p); /* points to what is next */
    int m = s < ms->src_end && singlematch(uchar(*s), p, ep);
    switch (*ep) {
    case '?': { /* optional */
      const char *res;
      if (m && ((res = match(ms, s + 1, ep + 1)) != NULL))
        return res;
      p = ep + 1;
      goto init; /* else return match(ms, s, ep+1); */
    }
    case '*': { /* 0 or more repetitions */
      return max_expand(ms, s, p, ep);
    }
    case '+': { /* 1 or more repetitions */
      return (m ? max_expand(ms, s + 1, p, ep) : NULL);
    }
    case '-': { /* 0 or more repetitions (minimum) */
      return min_expand(ms, s, p, ep);
    }
    default: {
      if (!m)
        return NULL;
      s++;
      p = ep;
      goto init; /* else return match(ms, s+1, ep); */
    }
    }
  }
  }
}

int lutil_split(lua_State *L) {
  size_t srcl;
  const char *src = luaL_checklstring(L, 1, &srcl);
  const char *p = luaL_checkstring(L, 2);
  int max_split = luaL_optint(L, 3, srcl + 1);
  int anchor = (*p == '^') ? (p++, 1) : 0;
  int idx = 0;
  MatchState ms;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ms.L = L;
  ms.src_init = src;
  ms.src_end = src + srcl;

  lua_newtable(L);

  while (idx < max_split) {
    const char *e;
    ms.level = 0;
    e = match(&ms, src, p);
    if (e) {
      luaL_pushresult(&b);
      lua_rawseti(L, -2, ++idx);
      luaL_buffinit(L, &b);
    }
    if (e && e > src) { /* non empty match? */
      src = e;          /* skip it */
    } else if (src < ms.src_end)
      luaL_addchar(&b, *src++);
    else
      break;
    if (anchor)
      break;
  }

  luaL_addlstring(&b, src, ms.src_end - src);
  luaL_pushresult(&b);
  lua_rawseti(L, -2, ++idx);

  return 1;
}
