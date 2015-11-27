Lua interface for then open62541 OPC UA library

Compile as
gcc -std=c99 src/*.c src/lua-5.2.4/src/!(luac).c -Isrc -Isrc/lua-5.2.4/src -D_BSD_SOURCE -DLUA_USE_LINUX -lm -pthread -lreadline -ldl -o uascript

Changes compared to vanilla Lua:
- ua module is loaded by default in the interpreter
- ua_lock uses a mutex to enable callbacks from a ua server running in a separate thread
- included the advanced readline support patch from http://luajit.org/patches/lua-5.2.0-advanced_readline.patch
