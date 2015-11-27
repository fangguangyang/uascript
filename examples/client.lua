r1 = ua.types.ReadValueId()
r1.nodeid = ua.types.NodeId(1,96)
r1.attributeid = 13
r = ua.types.ReadRequest()
r.nodestoread[1] = r1
c = ua.Client()
c:connect("opc.tcp://127.0.0.1:16664")
res = c:read(r)
print(res)
c:disconnect()
