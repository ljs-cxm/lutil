local lutil = require "lutil"

local left = "\t \n "
local right = " \t\n   "
local x = "123 \t\n abc hello world"


assert(lutil.trim(left .. x .. right) == x)

assert(lutil.ltrim(left .. x .. right) == x .. right)
assert(lutil.ltrim(left .. x) == x)

assert(lutil.rtrim(left .. x .. right) == left .. x)
assert(lutil.rtrim(x .. right) == x)
