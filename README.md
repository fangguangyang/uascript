uascript is a script-language frontend for the open62541 OPC UA library. It is
based on the Lua 5.2 language and licensed under MIT in order to enable a tight
integration with Lua.

Note that the open62541 library is included under a different license that
allows static linking.

Changes compared to vanilla Lua:
- ua module is loaded by default in the interpreter
- ua_lock uses a mutex to enable callbacks from a ua server running in a separate thread
- includes the advanced readline support patch from http://luajit.org/patches/lua-5.2.0-advanced_readline.patch

To use uascript, compile the code as 
`gcc -std=c99 src/*.c src/lua-5.2.4/src/*.c -Isrc -Isrc/lua-5.2.4/src -D_BSD_SOURCE -DLUA_USE_LINUX -lm -pthread -lreadline -ldl -o uascript`
or use the precompiled binaries in the /bin folder.