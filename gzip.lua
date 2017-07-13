local ffi = require "ffi"

local C = ffi.C
local ffi_new = ffi.new
local ffi_str = ffi.string

local type = type
local rshift = bit.rshift


ffi.cdef[[
enum {
  Z_BEST_SPEED          =  1,
  Z_BEST_COMPRESSION    =  9,
  Z_DEFAULT_COMPRESSION = -1,
};
int gzip_compress(uint8_t *src, size_t srclen, uint8_t *dest, size_t *destlen,
                  int level);
]]


local function find_shared_obj(cpath, so_name)
    local io, string = io, string
    for k in string.gmatch(cpath, "[^;]+") do
        local so_path = string.match(k, "(.*/)")
        so_path = so_path .. so_name

        -- Don't get me wrong, the only way to know if a file exist is trying
        -- to open it.
        local f = io.open(so_path)
        if f ~= nil then
            io.close(f)
            return so_path
        end
    end
end


local gzip = ffi.load(find_shared_obj(package.cpath, "libgzip.so"))
local buflen = ffi_new("size_t[1]", 0)


local _M = {
    _VERSION = "0.01",
    best_speed = C.Z_BEST_SPEED,
    best_compression = C.Z_BEST_COMPRESSION,
    default_compression = C.Z_DEFAULT_COMPRESSION,
}


function _M.compress(data, level)
    if type(data) ~= "string" then
        return nil, "string expected"
    end

    local len = #data

    -- NGINX
    local n = len + rshift(len + 7, 3) + rshift(len + 63, 6) + 5 + 18;
    local buf = ffi.new("uint8_t[?]", n)

    buflen[0] = n
    local rc = gzip.gzip_compress(ffi.cast("uint8_t *", data), len,
                                  buf, buflen, level or C.Z_BEST_SPEED)
    if rc ~= 0 then
        return nil, "rc: " .. rc
    end

    return ffi_str(buf, buflen[0])
end


return _M
