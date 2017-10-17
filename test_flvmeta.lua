local io = io
local flvmeta = require "flvmeta"

local args = { ... }

local content = io.open(args[1], "r"):read(1024 * 1024)

io.write(flvmeta.get_flv_metadata(content))
