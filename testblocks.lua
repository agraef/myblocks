-- This is basically the same program as testblocks.c, but written in Lua
-- using the Lua myblocks module.

-- Usage: lua testblocks.lua [ reset | filename [ save ] ]

-- The program can be interrupted with Ctrl+C at any time. It will first wait
-- for any blocks to be connected and then report topology changes (i.e.,
-- blocks being connected or disconnected), printing some information about
-- the connected blocks. In addition, there are some command line options for
-- initializing the master block (block #0) during startup, as soon as it's
-- connected:

-- Specifying the name of a file with Littlefoot code as the first argument
-- loads the program into the master block. The master block will normally
-- restore its previous configuration automatically when the script is exited
-- or the connection is broken, but you can also make the change permanent by
-- specifying 'save' as the second argument.

-- Specifying 'reset' as the first argument does a factory reset of the master
-- block, which restores its factory settings. (This will also revert any
-- permanent changes you did when loading a program on the block with 'save'.)

myblocks = require("myblocks")

myblocks.start()

-- We wrap most of the script in this little main function which is pcalled
-- below to take care of interrupts (Ctrl+C). This ensures that we always stop
-- JUCE before bailing out, so that it doesn't bother us with a bunch of
-- leaked object assertions.

function main()
   -- Wait until the devices are online.
   print("Waiting for blocks to be connected.")
   while myblocks.process() and myblocks.count_blocks() == 0 do
      myblocks.msleep(10)
   end
   -- Presumably we're ready now, do some initialization of the master block if
   -- requested.
   if arg[1] ~= nil then
      print("\nInitializing the master block:")
      if arg[1] == "reset" then
	 -- Do a factory reset.
	 print("Factory reset.");
	 myblocks.factory_reset(0);
      else
	 -- Load Littlefoot code.
	 print(string.format("Loading %s.", arg[1]));
	 if myblocks.load_program(0, arg[1]) then
	    print(string.format("Successfully loaded %s.", arg[1]))
	    -- Save the program on the device if requested.
	    if arg[2] == "save" then
	       print("Saving program.")
	       myblocks.save_program(0)
	    end
	 else
	    print(string.format("Error: %s", myblocks.msg()));
	 end
      end
   end
   -- Keep processing events until we're interrupted or the devices go offline.
   while myblocks.process() and myblocks.count_blocks() > 0 do
      if myblocks.changed() then
	 print("\nTopology change:")
	 local n = myblocks.count_blocks()
	 for i = 0, n-1 do
	    local info = myblocks.info(i)
	    print(string.format("%d: %0x %d %-8s %s%s", i, info.uid,
				info.type, info.type_descr, info.descr,
				info.is_master and " ** MASTER **" or ""))
	 end
      end
      myblocks.msleep(10)
   end
   print("No more connected blocks, exiting.")
end

if not pcall(main) then print("Interrupted, exiting.") end
myblocks.stop()
