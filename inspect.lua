function inspect(tb, level)
  level = level or 0
  local spaces = string.rep(' ', level*2)
  local mt = getmetatable(tb)
  if type(tb) ~= "table" and (not mt or not mt.__pairs) then
      print(tb)
      return
  end
  for k,v in pairs(tb) do
      mt = getmetatable(v)
      if type(v) ~= "table" and (not mt or not mt.__pairs) then
         print(spaces .. k .. ': ' .. tostring(v))
      else
         print(spaces .. k .. ': {')
         inspect(v, level + 1)
         print(spaces .. '}')
      end
  end  
end
