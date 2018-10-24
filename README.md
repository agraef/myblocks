# myblocks

This is a simple C and Lua wrapper around the stand-alone BLOCKS SDK used to manage and control the [Roli BLOCKS][] devices, such as the Lightpad, the Seaboard and the various control blocks. It provides the most essential parts of the SDK to application programs written in C or Lua. This currently comprises:

- Discovering the blocks which are connected to the host computer and retrieve information about these.

- Loading programs written in the Littlefoot language onto Lightpad blocks, saving programs on a block for permanent use, resetting blocks, and performing a factory reset of blocks to put them back into factory state.

- Receiving program messages from and sending messages to Lightpad blocks.

- Receiving button events, and setting the button LEDs and LED strips of control blocks.

All this functionality is also available in Pd by means of the `blocks` external (which is written in Lua and thus requires Pd-Lua to run).

C and Lua test programs are included in the sources, as well as some Pd patches and sample Littlefoot programs, please check *Usage* below for details.

[Roli BLOCKS]: https://roli.com/products/blocks/

## Installation

Currently the build system targets Linux, but porting to Mac and Windows should be a piece of cake. You'll need GNU make, pkg-config and gcc. Lua 5 is required to build the Lua module which is also needed for the Pd external. Lua 5.2 should probably do, Lua 5.3 has been tested. (The package can also be built without having Lua installed, but then only the C test program will be compiled.)

Please check the Makefile for details. On Linux, first run `make checkout` to check out the BLOCKS SDK submodule, then `make` to build everything, and finally `sudo make install` to install the Pd external (the latter step is optional; you can also run the included Pd patches directly from the source directory). Clean and uninstall targets are provided as well. For package maintainers, the usual `DESTDIR` and `prefix` variables are supported to change the installation prefix and do staged installations.

## Usage

If running `make` completed with success, there's a C program `testblocks` and a Lua script `testblocks.lua` which you can run to check that the basic functionality works. Both programs work the same and offer the same command line options; please see the comments at the beginning of the Lua script for usage instructions. E.g., make sure that your BLOCKS devices are connected to the host computer, then run:

    ./testblocks examples/XYZPad.littlefoot

Or:

    lua testblocks.lua examples/XYZPad.littlefoot

To run one of the included Pd patches, make sure that Pd-Lua is installed and activated in Pd. Then just open the patch in Pd and follow the instructions in it. In each of the patches you'll find a toggle labeled "click here to start/stop" which needs to be turned on to launch the BLOCKS interface and use all the functionality described above. You can turn this off and on again to reset the interface as often as needed.

A few Littlefoot programs to be loaded on Lightpad blocks are included in the examples subdirectory (more can be found in Roli's source code repository at https://github.com/WeAreROLI/Littlefoot-Examples). You can load these with the `testblocks` program, as shown above, or by sending a `load` message to the `blocks` object in Pd, as shown in the blocks-help.pd patch. Note that exiting the interface will revert the Lightpad to its previous configuration, unless you also use the `save` option (`save` message in Pd) to commit the Littlefoot program to permanent storage. After doing this, you'll have to do a factory reset of the device (`reset` option of the `testblocks` program, or `factoryreset` message in Pd) to restore it to its original state. (If you have a Mac or a Windows PC, you can also use Roli's Dashboard program to do all this.)

Littlefoot is a very tiny C-like language designed especially to program the BLOCKS Lightpad, see https://docs.juce.com/blocks/the_littlefoot_language.html and https://docs.juce.com/blocks/group__LittleFootFunctions.html for details. It lets you operate the display of the device and handle touch events. It also provides the necessary facilities to communicate with the host program, allowing you to send and receive both direct "program" messages, or the usual kinds of short MIDI messages. The latter require a MIDI connection to the host program, while the former can be used directly through the corresponding API functions (or the `msg` message in the Pd interface).

## Notes

Bug reports and pull requests are appreciated. Please submit them through the source code repository, or send me an email.

Note that many examples in Roli's Littlefoot-Examples repository contain XML meta-data with configuration items which don't appear to be directly supported in the stand-alone BLOCKS SDK right now (at least I couldn't find a way to make them work). You'll notice this when you get an error message from the compiler complaining about undefined variables when trying to load such a program on a Lightpad. To make these programs work, at present you'll have to rewrite them to use global variables and an initialization routine in Littlefoot instead, as I have done for some of the examples included in the package.

Enjoy! :)  
Albert Graef <aggraef@gmail.com>
