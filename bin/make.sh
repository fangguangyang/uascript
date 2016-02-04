#!/bin/bash
# 64bit (native)
gcc -g -std=c99 src/*.c src/lua-5.2.4/src/*.c -Isrc -Isrc/lua-5.2.4/src -D_BSD_SOURCE -DLUA_USE_LINUX -lm -pthread -lreadline -ldl -o uascript_x64 -ffunction-sections -fdata-sections -fmerge-all-constants -fno-ident -Wl,--gc-sections
# 32bit
#gcc -static -m32 -Os -std=c99 src/*.c src/lua-5.2.4/src/*.c -Isrc -Isrc/lua-5.2.4/src -D_BSD_SOURCE -DLUA_USE_LINUX -lm -pthread -lreadline -ldl -o uascript_x86 -ffunction-sections -fdata-sections -fmerge-all-constants -fno-ident -Wl,--gc-sections
# win
#i586-mingw32msvc-gcc -static -Os -std=c99 src/*.c src/lua-5.2.4/src/*.c -Isrc -D_BSD_SOURCE -Isrc/lua-5.2.4/src -DLUA_WIN -lws2_32 -o uascript.exe -ffunction-sections -fdata-sections -fmerge-all-constants -fno-ident -Wl,--gc-sections
