uascript Internals
==================

uascript is based on Lua 5.2.4 with the following extensions.

- The ua module contains the binding to the open62541 library
- The readline extension provides an improvement shell with history and autocompletion
- The interpreter makes use of a global lock in order to allow callbacks from
  the OPC UA server and clients running in separate threads
- The ``tostring`` command was extended to return the formatted content for tables
