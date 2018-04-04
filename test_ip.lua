local clock = require "os".clock
local lutil = require "lutil"

local iters = 2000000

local function test(func)
    local start = clock()
    local num
    local n = 0
    while n < iters do
        num = func("127.0.0.1")
        n = n + 1
    end
    local elapsed = clock() - start
    print("result:", num)
    print(string.format("Elapsed Time: %.6f s, %.6f us", elapsed, 1000000 * elapsed / iters))
end


local invalid = {
    '',
    '.',
    '..',
    '...',
    '....',
    '.....',
    '0',
    '1',
    '127.',
    '256.0',
    '127.0.0',
    '127.0.0.0.',
    '127.01.0.0',
    '127.1.0.256',
}


for _, ip in ipairs(invalid) do
    assert(lutil.ip_to_number(ip) == nil, ip)
end


local ips = {
    { '0.0.0.0', 0 },
    { '127.0.0.1', 2130706433 },
    { '127.0.0.11', 2130706443 },
    { '255.255.255.255', 4294967295 },
}


for _, ip in ipairs(ips) do
    assert(lutil.ip_to_number(ip[1]) == ip[2])
end


test(lutil.ip_to_number)
