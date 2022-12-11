json = require "json"

local frandom = io.open("/dev/urandom", "rb")
local d = frandom:read(4)
math.randomseed(d:byte(1) + (d:byte(2) * 256) + (d:byte(3) * 65536) + (d:byte(4) * 4294967296))

local open = io.open

local function read_file(path)
    local file = open(path, "rb") -- r read mode and b binary mode
    if not file then return nil end
    local content = file:read "*a" -- *a or *all reads the whole file
    file:close()
    return content
end

local data_str = read_file("data.json");
local data = json.decode(data_str) 

request = function()
    number = math.random(1,500)
    headers = {}
    headers["Content-Type"] = "application/json"
    body = ''
    return wrk.format("GET", "/user?no_cache&email=".. tostring(data[number]['email']), headers, body)
end