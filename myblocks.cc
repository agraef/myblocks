#include <iostream>
#include <BlocksHeader.h>

using namespace std;
using namespace juce;

/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

// This is pilfered from the BlockFinder example distributed with the SDK, see
// the copyright notice above. It monitors a PhysicalTopologySource for
// changes to the connected BLOCKS. This also lets us access the currently
// connected blocks in a convenient way, so we simply add in our own access
// operations here.

class BlockFinder : private TopologySource::Listener
{
public:
  // Register as a listener to the PhysicalTopologySource, so that we receive
  // callbacks in topologyChanged().
  BlockFinder();

  // Our own stuff goes here.
  // Count the number of connected blocks.
  int CountBlocks();
  // Set the program to be run on the given block, given by its block number
  // (it looks like block #0 always denotes the master block). If save is
  // true, commit the program to permanent storage so that it remains loaded
  // even after the connection is broken or when the block is restarted, until
  // another program is loaded or a factory reset is performed (see below).
  // The Littlefoot code is in the code parameter. Returns true iff the
  // program was loaded successfully (in which case msg will be "Ok"), or
  // returns the error message from the compiler in msg.
  bool SetProgram(int blocknum, bool save, String code, String &msg);
  // Resets the given block. If factory is true, performs a factory reset,
  // which on a Lightpad block reloads the default note grid app.
  void Reset(int blocknum, bool factory);

private:
  // Called by the PhysicalTopologySource when the BLOCKS topology changes.
  void topologyChanged() override;

  // The PhysicalTopologySource member variable which reports BLOCKS changes.
  PhysicalTopologySource pts;

  Block::Array blocks;
};
 
BlockFinder::BlockFinder()
{
  // Register to receive topologyChanged() callbacks from pts.
  pts.addListener (this);
}

static String block_type(Block::Type t)
{
  switch (t) {
  case Block::lightPadBlock:
    return "LightPad";
  case Block::liveBlock:
    return "Live";
  case Block::loopBlock:
    return "Loop";
  case Block::developerControlBlock:
    return "Developer Control";
  case Block::touchBlock:
    return "Touch";
  case Block::seaboardBlock:
    return "Seaboard";
  default:
    return "Unknown";
  }
}

void BlockFinder::topologyChanged()
{
  // We have a new topology, so find out what it is and store it in a local
  // variable.
  auto currentTopology = pts.getCurrentTopology();

  // The blocks member of a BlockTopology contains an array of blocks. Here we
  // loop over them and print some information.
  cout << ("\nNew BLOCKS topology. Detected " + String (currentTopology.blocks.size()) + " blocks:\n");

  for (auto& block : currentTopology.blocks) {
    cout << ("\n");
    cout << ("    Type:          " + block_type(block->getType()) + "\n");
    cout << ("    Description:   " + block->getDeviceDescription() + (block->isMasterBlock()?" ** MASTER BLOCK **":"") + "\n");
    cout << ("    Battery level: " + String (block->getBatteryLevel()) + (block->isBatteryCharging()?" (charging)":" (charged)") + "\n");
    cout << ("    UID:           " + String (block->uid) + "\n");
    cout << ("    Serial number: " + block->serialNumber + "\n");
    cout << ("    Config items:  " + String(block->getMaxConfigIndex()) + "\n");
#if 0
    for (int i = 0; i < block->getMaxConfigIndex(); i++) {
      auto data = block->getLocalConfigMetaData(i);
      if (data.name == "") continue;
      cout << "item #" << i << ": " << data.name << " = " << data.value << String(block->isValidUserConfigIndex(i)?" (user)":"") << "\n";
    }
#endif
  }

  blocks = currentTopology.blocks;
}

int BlockFinder::CountBlocks()
{
  return blocks.size();
}

struct BlockProgram : Block::Program
{
  BlockProgram(Block &b, String _code) : Block::Program(b), code(_code) {}
  String getLittleFootProgram() override { return code; }
  String code;
};

bool BlockFinder::SetProgram(int blocknum, bool save, String code,
			     String& msg)
{
  for (auto& block : blocks) {
    if (blocknum-- == 0) {
      BlockProgram *prog = new BlockProgram(*block, code);
      Result res = block->setProgram(prog);
      if (res.failed()) {
	msg = res.getErrorMessage();
	return false;
      } else {
	if (save) block->saveProgramAsDefault();
	msg = "Ok";
	return true;
      }
    }
  }
  return false;
}

void BlockFinder::Reset(int blocknum, bool factory)
{
  for (auto& block : blocks) {
    if (blocknum-- == 0) {
      if (factory)
	block->factoryReset();
      else
	block->blockReset();
      return;
    }
  }
}

// We need a way to tie into JUCE's event loop, the easiest way to do this is
// to have a JUCE Application.
class MyJUCEApp  : public juce::JUCEApplicationBase
{
public:
  MyJUCEApp()  {}
  ~MyJUCEApp() {}

  void initialise (const juce::String&) override {}
  void shutdown() override                       {}

  const juce::String getApplicationName() override           { return "BlockFinder"; }
  const juce::String getApplicationVersion() override        { return "1.0.0"; }
  bool moreThanOneInstanceAllowed() override                 { return true; }
  void anotherInstanceStarted (const juce::String&) override {}

  void suspended() override           {}
  void resumed() override             {}
  void systemRequestedQuit() override {}
  void unhandledException(const std::exception*, const juce::String&,
			  int lineNumber) override {}

  // Our BLOCKS class.
  BlockFinder finder;
};

// The JUCE application factory needs this callback.
static JUCEApplicationBase* juce_CreateApplication()
{
  MyJUCEApp *app = new MyJUCEApp();
  return app;
}

// Our JUCE application.
static MyJUCEApp *app;

// Helper functions to initialize and finalize the app and run its event loop
// in a piecemeal fashion.

#include "myblocks.h"

extern "C" void juce_init(void)
{
  if (!app) {
    initialiseJuce_GUI();
    JUCEApplicationBase::createInstance = &juce_CreateApplication;
    app = new MyJUCEApp();
    if (app && !app->initialiseApp()) {
      cerr << "Error initializing JUCE app!\n";
      app->shutdownApp();
      app = 0;
      shutdownJuce_GUI();
    }
  }
}

extern "C" void juce_fini(void)
{
  if (app) {
    app->shutdownApp();
    delete app;
    app = 0;
    shutdownJuce_GUI();
  }
}

extern "C" bool juce_process_events(void)
{
  if (app)
    return MessageManager::getInstance()->runDispatchLoopUntil(0);
  else
    return false;
}

extern "C" int myblocks_count_blocks()
{
  if (app)
    return app->finder.CountBlocks();
  else
    return 0;
}

std::string myblocks_msg_string;
extern "C" const char *myblocks_msg()
{
  return myblocks_msg_string.c_str();
}

extern "C" bool myblocks_set_program(int blocknum, bool save, const char *code)
{
  if (app) {
    String msg;
    bool res = app->finder.SetProgram(blocknum, save, code, msg);
    myblocks_msg_string = msg.toStdString();
    return res;
  } else {
    myblocks_msg_string = "JUCE not running";
    return false;
  }
}

#include <sys/types.h>
#include <sys/stat.h>

extern "C" bool myblocks_load_program(int blocknum, bool save,
				      const char *filename)
{
  struct stat st;
  if (stat(filename, &st)) {
    myblocks_msg_string = std::string(filename) + ": " +
      std::string(strerror(errno));
    return false;
  }
  char *code = (char*)calloc(st.st_size+1, 1);
  FILE *fp = fopen(filename, "r");
  if (fp && fread(code, 1, st.st_size, fp) == (size_t)st.st_size) {
    fclose(fp);
    bool res = myblocks_set_program(blocknum, save, code);
    free(code);
    return res;
  } else {
    free(code);
    myblocks_msg_string = std::string(filename) + ": " +
      std::string(strerror(errno));
    if (fp) fclose(fp);
    return false;
  }
}

extern "C" void myblocks_reset(int blocknum, bool factory)
{
  if (app) app->finder.Reset(blocknum, factory);
}
