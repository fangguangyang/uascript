OPC UA Data Types
=================

Lua Data Types
--------------

Lua provides basic data types, such as numbers, strings and booleans. All
datatypes can be transformed to a string with the ``tostring`` function. Strings
can be concatenated with the ``..`` operator. The ``tonumber`` function tries to
convert a string back into a number.

.. code-block:: lua

   -- this is a one-line comment

   --[[
       this is a comment
       on several lines
   ]]

   n = 133.7
   print(s)
   s = "a string"
   print(s)
   b = true
   print(b)
   h = nil
   print(h)
   print(hh) -- note that hh has not beed defined

   --[[ output:
       133.7
       a string
       true
       nil
       nil
   ]]

Lua provides a single data type for complex structures: tables. Using numerical
indices, tables can act as an array. Note that arrays are 1-indexed.

.. code-block:: lua

   t = {}
   t[1] = "test"
   t[2] = 123
   t[3] = false
   t[100] = "what is happening here?"

   print(#t) -- # returns the number of consecutive entries starting at 1
   print(t[100]) -- the entry at 100 is still there. But it does not count towards the array length

   -- ipairs iterates over the (consecutive) array entries starting at 1
   for i,v in ipairs(t) do
       print(tostring(i) .. " = " .. tostring(v))
   end

   --[[ output:
       3
       what is happening here?
       1 = test
       2 = 123
       3 = false
   ]]

Lua tables equally act as hash-maps.

.. code-block:: lua

   t2 = {}
   t2[1] = 123
   t2["index"] = false
   
   print(#t2) -- the t2 "array" has only 1 entry
   print(t2["index"])

   -- pairs iterates over all entries of the table
   for i,v in pairs(t) do
       print(tostring(i) .. " = " .. tostring(v))
   end

   --[[ output:
       1
       false
       1 = 123
       index = false

All OPC UA datatypes are entries of the ``ua.types`` table.

.. code-block:: lua

   i32 = ua.types.Int32(5)
   print(i32)
   
   

Builtin Data Types
------------------

Structures
----------

Arrays
------
