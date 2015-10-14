Lua interface for then open62541 OPC UA library

Compile as
 gcc -shared -fPIC -std=c99 *.c -llua5.2 -lpthread -o open62541.so

Then run the example as "lua test.lua"
