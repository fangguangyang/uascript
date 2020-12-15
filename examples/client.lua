local root = "/icore/eg/"
package.path = root .. "mod/?.lua;" .. root .. "mod/?/init.lua;" .. root .. "expand/?.lua;" .. root .. "business/?.lua;"
package.cpath = "../build/?.so"
local ua = require "ua"
local inspect = require "inspect"
c = ua.Client()
c:connect("opc.tcp://127.0.0.1:16664")

w1 = ua.types.WriteValue()
w1.nodeId = ua.types.NodeId(1, "the.answer")
w1.attributeId = ua.attributeIds.Value
w1.value.value = ua.types.Int32(43)
c:write({w1})

r1 = ua.types.ReadValueId()
r1.nodeId = ua.types.NodeId(1, "the.answer")
r1.attributeId = ua.attributeIds.Value
res = c:read({r1})
print(res)

c:disconnect()
