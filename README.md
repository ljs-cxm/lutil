lutil
====

A grab bag of helpful utilities for programming with Lua.

# Utilities

`lutil.trim(string)`
----
Remove white spaces at the beginning and the end of the string.

`lutil.ltrim(string)`
----
Remove white spaces at the beginning of the string.

`lutil.rtrim(string)`
----
Remove white spaces at the end of the string.

`lutil.split(string, pattern, max_split?)`
----
Split string by the occurrences of pattern. If max_split is nonzero, at most
maxsplit splits occur, and the remainder of the string is returned as the final
element of the list.
