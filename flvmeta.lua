local ffi = require "ffi"

local C = ffi.C

local type = type
local tonumber = tonumber
local tostring = tostring
local string_sub = string.sub


ffi.cdef[[
int64_t flv_metadata_offset(const uint8_t *buf, const size_t buflen);
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


local flvmeta = ffi.load(find_shared_obj(package.cpath, "libflvmeta.so"))


local function get_flv_metadata(buf)
    if type(buf) ~= "string" then
        return nil, "string expected"
    end

    local len = #buf
    local offset = flvmeta.flv_metadata_offset(ffi.cast("uint8_t *", buf), len)
    if offset < 0 then
        return nil, "offset: " .. tostring(offset)
    end

    return string_sub(buf, 1, tonumber(offset))
end


return {
    _VERSION = 0.01,

    get_flv_metadata = get_flv_metadata,
}
