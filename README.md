# myblocks

This is a simple C and Lua wrapper around the stand-alone BLOCKS SDK used to manage and control the [Roli BLOCKS][] devices, such as the Lightpad, the Seaboard and the various control blocks. It provides the most essential parts of the SDK to application programs written in C or Lua. This currently comprises:

- Discovering the blocks which are connected to the host computer and retrieve information about these.

- Loading programs written in the Littlefoot language onto Lightpad blocks, and saving them for permanent use.

- Resetting a block (which restarts the program running on a block), and performing a factory reset (which puts a block back into factory state).

- Getting and setting the heap data of a block.

- Receiving program messages from and sending messages to Lightpad blocks.

- Receiving button events, and setting the button LEDs and LED strips of control blocks.

All this functionality is also available in Pd by means of the `blocks` external. The external is written in Lua and thus requires [Pd-Lua][] to run.

C and Lua test programs are included in the sources, as well as some Pd patches and sample Littlefoot programs, please check *Usage* below for details.

[Roli BLOCKS]: https://roli.com/products/blocks/
[Pd-Lua]: https://github.com/agraef/pd-lua

## Installation

Currently the build system targets Linux, but porting to Mac and Windows should be a piece of cake. You'll need GNU make, pkg-config and gcc. Lua 5 is required to build the Lua module which is also needed for the Pd external. Lua 5.2 should probably do, Lua 5.3 has been tested. (The package can also be built without having Lua installed, but then only the C test program will be compiled.)

Please check the Makefile for details. On Linux, first run `make checkout` to check out the BLOCKS SDK submodule, then `make` to build everything, and finally `sudo make install` to install the Pd external (the latter step is optional; you can also run the included Pd patches directly from the source directory). Clean and uninstall targets are provided as well. For package maintainers, the usual `DESTDIR` and `prefix` variables are supported to change the installation prefix and do staged installations.

## Usage

After running `make`, there's a C program `testblocks` which you can run to test the basic functionality. The sources also contain a Lua script `testblocks.lua` to test the Lua interface. Both programs work the same and offer the same command line options; please see the comments at the beginning of the Lua script for usage instructions.

But myblocks is used most conveniently through the included Pd external. Make sure that Pd-Lua is installed and activated in Pd, then just open one of the provided patches in Pd and follow the instructions in it. In each of the patches you'll find a toggle labeled "click here to start/stop" which needs to be turned on to launch a little JUCE application which drives the myblocks interface and gives access to all the functionality described above. You can turn the toggle off and on again to reset the interface as often as needed.

## Littlefoot

The Lightpad block can be programmed in the Littlefoot language to customize the way it behaves and communicates with the host application. The myblocks library and the Pd external let you load such programs on the device using the corresponding SDK operations.

Littlefoot is a tiny C-like language which gets compiled to byte code, see https://docs.juce.com/blocks/the_littlefoot_language.html for details. It offers a collection of built-in functions which let you change the display of the device, handle touch events, and communicate with the host program, see https://docs.juce.com/blocks/group__LittleFootFunctions.html.

A few sample Littlefoot programs based on Roli's documentation and examples are included in the sources, you can find these in the examples subdirectory. More examples can be found in Roli's source code repository at https://github.com/WeAreROLI/Littlefoot-Examples.

You can load these programs with the `testblocks` program, or by sending a `load` message to the `blocks` object in Pd, as shown in the blocks-help.pd patch. Note that exiting the interface will revert the Lightpad to its previous configuration, unless you also use the `save` option (`save` message in Pd) to commit the Littlefoot program to permanent storage. After doing this, you'll have to do a factory reset of the device (`reset` option of the `testblocks` program, or `factoryreset` message in Pd) to restore it to its original state. (If you have a Mac or a Windows PC, you can also use Roli's Dashboard program to do all this.)

The Littlefoot language and the SDK provide various facilities for exchanging data between device and host application. The following facilities are currently available in the myblocks library:

- Heap data. This is a special memory area shared by the Littlefoot program and the host, which allows to exchange data between the two by getting and setting individual bytes or integer values in the heap.

- Program messages. These allow to send and receive messages consisting of up to three integer values between device and host.

- Button events. Pressing any of the buttons on the control blocks connected to the master block generates a corresponding event which can be processed by the host program. Conversely, the host program can set the button LEDs and the LED strips on the control blocks to provide feedback.

- MIDI data. Littlefoot programs can also send and receive MIDI data, which also works if the host program doesn't use the SDK, but requires a MIDI connection between device and host.

MIDI communication will work with any host which can send and receive MIDI data, but requires that the program running on the Lightpad has been set up for MIDI operation. The other facilities will work with or without a MIDI connection, and are available through API functions of the myblocks library or the corresponding Pd messages. Please have a look at the myblocks.h header file and the included Pd patches for details.

Please note that myblocks currently does *not* handle configuration meta-data in XML format which can be found in many examples in the Littlefoot-Examples repository. This is used by the Roli Dashboard and the BLOCKS CODE IDE as an alternative way to initialize variables values in the running program, but it doesn't seem to be properly supported in the SDK yet. You'll notice this when you get an error message from the compiler complaining about undefined variables when trying to load such a program using myblocks. To make these programs work, you'll have to rewrite them to use global variables and an initialization routine in Littlefoot instead, as I have done for some of the examples included in the package.

## Reporting Bugs

Bug reports and pull requests are always appreciated! Please submit them through the source code repository, or send me an email.

Enjoy! :)  
Albert Graef <aggraef@gmail.com>
