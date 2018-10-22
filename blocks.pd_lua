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

function MB:in_1_info(atoms)
   function mkinfo(i)
      local info = myblocks.info(i)
      -- We rearrange the order a bit (uid comes last, code is omitted). Also,
      -- we convert flags (booleans) to numbers, and uid, which is a 64 bit
      -- number, is converted to a symbol (hex representation).
      return {i, info.type,
	      info.is_master and 1 or 0, info.is_charging and 1 or 0,
	      info.battery_level,
	      info.descr, info.type_descr, info.serial, info.version,
	      string.format("%0x", info.uid)}
   end
   if type(atoms[1]) == "number" then
      self:outlet(1, "info", mkinfo(math.floor(atoms[1])))
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
	 self:error("myblocks: " .. myblocks.msg())
      end
   else
      self:error("myblocks: save expects the block number as argument")
   end
end

function MB:in_1_set(atoms)
   if type(atoms[1]) == "number" and type(atoms[2]) == "string" then
      if not myblocks.set_program(atoms[1], atoms[2]) then
	 self:error("myblocks: " .. myblocks.msg())
      end
   else
      self:error("myblocks: save expects the block number as argument")
   end
end

function MB:in_1_save(atoms)
   if type(atoms[1]) == "number" then
      myblocks.save_program(atoms[1])
   else
      self:error("myblocks: save expects the block number as argument")
   end
end

function MB:tick()
   if myblocks.process() and myblocks.changed() then
      self:outlet(2, "float", {myblocks.count_blocks()})
   end
   self.clock:delay(self.period)
end
