package.cpath = package.cpath .. ";?.dylib"

local cjson = require "cjson.safe"
local lutil = require "lutil"

local format = string.format


local function t(text, pattern, max_split)
    print(format("text: [%s], pattern: [%s], max_split: [%d]", text, pattern, max_split or 0))

    local t = lutil.split(text, pattern, max_split)
    print(cjson.encode(t))
end

local text = ",,abc,def, ,,,ghi,*,+*,,"

t(text, "")
t(text, ".")
t(text, ".", 1)
t(text, "^,%s*")
t(text, ",+")
t(text, ",%+")

t(" a b c ", " ")
t(lutil.trim(" a b c "), " ")
