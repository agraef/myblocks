local MB = pd.Class:new():register("blocks")

function MB:initialize(name, atoms)
    self.inlets = 1
    self.outlets = 2
    self.period = #atoms>0 and atoms[1] or nil
    if type(self.period) ~= "number" then
       self.period = 10 -- hard-coded default
    end
    return true
end

function MB:postinitialize()
   self.clock = pd.Clock:new():register(self, "tick")
end

function MB:finalize()
   self.clock:destruct()
   myblocks.stop()
end

myblocks = require("myblocks")

function MB:in_1_float(f)
   if f ~= 0 then
      myblocks.start()
      self:tick()
   else
      myblocks.stop()
      self.clock:unset()
   end
end

function MB:in_1_count()
   self:outlet(1, "float", {myblocks.count_blocks()})
end

function MB:in_1_blocknum(atoms)
   local uid = tonumber(atoms[1], 16)
   if uid ~= nil then
      self:outlet(1, "float", {myblocks.blocknum(uid)})
   else
      self:error("blocks: blocknum expects a string of hex digits")
   end
end

function MB:in_1_info(atoms)
   function mkinfo(i)
      local info = myblocks.info(i)
      if info ~= nil then
	 -- We rearrange the order a bit (uid comes last, code is
	 -- omitted). Also, we convert flags (booleans) to numbers, and uid,
	 -- which is a 64 bit number, is converted to a symbol (hex
	 -- representation).
	 return {i, info.type,
		 info.is_master and 1 or 0, info.is_charging and 1 or 0,
		 info.battery_level, info.nbuttons, info.nleds,
		 info.descr, info.type_descr, info.serial, info.version,
		 string.format("%0x", info.uid)}
      else
	 return nil
      end
   end
   if type(atoms[1]) == "number" then
      local info = mkinfo(math.floor(atoms[1]))
      if info ~= nil then
	 self:outlet(1, "info", info)
      else
	 self:error("blocks: info expects a valid block number")
      end
   else
      local n = myblocks.count_blocks()
      for i = 0, n-1 do
	 self:outlet(1, "info", mkinfo(i))
      end
   end
end

function MB:in_1_reset(atoms)
   if type(atoms[1]) == "number" then
      myblocks.reset(atoms[1])
   else
      local n = myblocks.count_blocks()
      for i = 0, n-1 do
	 myblocks.reset(i)
      end
   end
end

function MB:in_1_factoryreset(atoms)
   if type(atoms[1]) == "number" then
      myblocks.factory_reset(atoms[1])
   else
      local n = myblocks.count_blocks()
      for i = 0, n-1 do
	 myblocks.factory_reset(i)
      end
   end
end

function MB:in_1_load(atoms)
   if type(atoms[1]) == "number" and type(atoms[2]) == "string" then
      if not myblocks.load_program(atoms[1], atoms[2]) then
	 self:error("blocks: " .. myblocks.msg())
      end
   else
      self:error("blocks: load expects a block number and a program file name")
   end
end

function MB:in_1_save(atoms)
   if type(atoms[1]) == "number" then
      myblocks.save_program(atoms[1])
   else
      self:error("blocks: save expects a block number")
   end
end

function MB:in_1_msg(atoms)
   local n = #atoms
   local ok = n > 1
   if ok then
      for i = 1, n do
	 ok = type(atoms[i]) == "number"
	 if not ok then break end
      end
   end
   if ok then
      local t = table.pack(table.unpack(atoms, 2))
      myblocks.send(atoms[1], t)
   else
      self:error("blocks: msg expects a block number followed by a list of numbers")
   end
end

function MB:in_1_button(atoms)
   local blocknum = atoms[1]
   local num = atoms[2]
   local color = atoms[3]
   if type(blocknum) == "number" and type(num) == "number" and
      type(color) == "number" then
      myblocks.set_button(blocknum, num, color)
   else
      self:error("blocks: button expects a block number followed by a button number and a color")
   end
end

function MB:in_1_leds(atoms)
   local blocknum = atoms[1]
   local num = atoms[2]
   local color = atoms[3]
   if type(blocknum) == "number" and type(num) == "number" and
      type(color) == "number" then
      myblocks.set_leds(blocknum, num, color)
   else
      self:error("blocks: leds expects a block number followed by an led number and a color")
   end
end

function MB:in_1_getdata(atoms)
   local blocknum = atoms[1]
   local offset = atoms[2]
   if type(blocknum) == "number" and type(offset) == "number" then
      self:outlet(1, "data", {myblocks.get(blocknum, offset)})
   else
      self:error("blocks: getdata expects a block number followed by an offset")
   end
end

function MB:in_1_setdata(atoms)
   local blocknum = atoms[1]
   local offset = atoms[2]
   local data = atoms[3]
   if type(blocknum) == "number" and type(offset) == "number" and
      type(data) == "number" then
      myblocks.set(blocknum, offset, data)
   else
      self:error("blocks: setdata expects a block number followed by an offset and a byte value")
   end
end

function MB:tick()
   if myblocks.process() and myblocks.changed() then
      self:outlet(2, "float", {myblocks.count_blocks()})
   end
   local i, msg = myblocks.receive()
   while i ~= nil do
      if msg.name ~= nil then
	 -- button
	 print(string.format("%d: button %d %d (%s) %d", i, msg.num,
			     msg.type, msg.name, msg.pressed and 1 or 0))
	 self:outlet(1, "button", { i, msg.num, msg.name, msg.type,
				    msg.pressed and 1 or 0 })
      else
	 -- program message
	 self:outlet(1, "msg", { i, table.unpack(msg) })
      end
      i, msg = myblocks.receive()
   end
   self.clock:delay(self.period)
end
