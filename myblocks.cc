#include <iostream>
#include <BlocksHeader.h>

#include "myblocks.h"

using namespace std;
using namespace juce;

struct Msg
{
  int n;       // block number
  int msg[3];  // program message (sendMessageToHost)
  int button;  // 0 = message, 1 = button pressed, 2 = button released
  int num;     // button number
  int type;    // button type
  String name; // button name
  LinkedListPointer<Msg> nextListItem;
  Msg(int k, int x, int y, int z) {
    n = k; button = num = type = 0;
    msg[0] = x; msg[1] = y; msg[2] = z;
  }
  Msg(int k, int _button, int _num, int _type, String _name) {
    n = k; button = _button; num = _num; type = _type; name = _name;
    msg[0] = msg[1] = msg[2] = 0;
  }
};

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

class BlockFinder : private TopologySource::Listener,
		    private Block::ProgramEventListener,
		    private ControlButton::Listener
{
public:
  BlockFinder();

  // Our own stuff goes here.

  // Flag indicating whether the topology has changed since the last time.
  bool Changed();
  // Get a message received from a block.
  bool GetMsg(int &blocknum, int msg[3],
	      myblocks_button_info_t &button_info);
  // Count the number of connected blocks.
  int CountBlocks();
  // Get a block, given by its block number (it looks like block #0 always
  // denotes the master block). The returned pointer is for immediate use and
  // only remains valid until the toplogy is updated, as indicated by the
  // Changed() flag.
  Block *GetBlock(int blocknum);
  // Get a button of a (control) block.
  ControlButton *GetButton(Block *block, int num);
  // Set the program to be run on the given block. The Littlefoot code is in
  // the code parameter. Returns true iff the program was loaded successfully
  // (in which case msg will be "Ok"), or returns the error message from the
  // compiler in msg.
  bool SetProgram(int blocknum, String code, String &msg);
  // Commit the currently loaded program to permanent storage so that it
  // remains loaded even after the connection is broken or when the block is
  // restarted, until another program is loaded or a factory reset is
  // performed (see below).
  void SaveProgram(int blocknum);
  // Resets the given block. Not sure what this actually does, maybe it resets
  // the variable memory and restarts the program running on the block?
  void Reset(int blocknum);
  // Performs a factory reset of the block, which on a Lightpad block reloads
  // the default note grid app.
  void FactoryReset(int blocknum);

private:
  // Called by blocks when receiving a program event.
  void handleProgramEvent(Block &source, const Block::ProgramEventMessage&);

  // Called by blocks when a button is pressed/released.
  void buttonPressed(ControlButton& button, Block::Timestamp ts) override;
  void buttonReleased(ControlButton& button, Block::Timestamp ts) override;

  // Called by the PhysicalTopologySource when the BLOCKS topology changes.
  void topologyChanged() override;

  // The PhysicalTopologySource member variable which reports BLOCKS changes.
  PhysicalTopologySource pts;

  Block::Array blocks;
  bool changed;
  // Our message queue is a linked list for now, might want to do something
  // more sophisticated here in the future.
  LinkedListPointer<Msg> msgs;
  int find_block(Block &block);
  int find_button(Block &block, ControlButton &button);
};

BlockFinder::BlockFinder()
{
  // Register to receive topologyChanged() callbacks from pts.
  pts.addListener(this);
  changed = false;
}

static const char *block_type(Block::Type t)
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

int BlockFinder::find_block(Block &block)
{
  int blocknum = 0;
  for (auto& b : blocks) {
    if (*b == block) break;
    blocknum++;
  }
  return blocknum;
}

int BlockFinder::find_button(Block &block, ControlButton &button)
{
  int num = 0;
  for (auto b : block.getButtons()) {
    if (b == &button) break;
    num++;
  }
  return num;
}

void BlockFinder::handleProgramEvent(Block &block,
				     const Block::ProgramEventMessage &ev)
{
  int blocknum = find_block(block);
#if 0
  printf("%d: %d %d %d\n", blocknum, ev.values[0], ev.values[1], ev.values[2]);
#endif
  msgs.append(new Msg(blocknum, ev.values[0], ev.values[1], ev.values[2]));
}

void BlockFinder::buttonPressed(ControlButton& button, Block::Timestamp ts)
{
  int blocknum = find_block(button.block);
  int num = find_button(button.block, button);
#if 0
  cout << blocknum << ": button pressed: " << button.getType() << " (" << button.getName() << ") x = " << button.getPositionX() <<  " y = " << button.getPositionY() <<  " light? " << button.hasLight() << "\n";
#endif
  msgs.append(new Msg(blocknum, 1, num, button.getType(), button.getName()));
}

void BlockFinder::buttonReleased(ControlButton& button, Block::Timestamp ts)
{
  int blocknum = find_block(button.block);
  int num = find_button(button.block, button);
#if 0
  cout << blocknum << ": button released: " << button.getType() << " (" << button.getName() << ") x = " << button.getPositionX() <<  " y = " << button.getPositionY() <<  " light? " << button.hasLight() << "\n";
#endif
  msgs.append(new Msg(blocknum, 2, num, button.getType(), button.getName()));
}

void BlockFinder::topologyChanged()
{
  // We have a new topology, so find out what it is and store it in a local
  // variable.
  auto currentTopology = pts.getCurrentTopology();

  blocks = currentTopology.blocks;
  changed = true;
  msgs.deleteAll();

  // Add program event and button listeners.
  for (auto& block : blocks) {
    block->addProgramEventListener(this);
    for (auto button : block->getButtons())
      button->addListener(this);
  }

#if 0
  // The blocks member of a BlockTopology contains an array of blocks. Here we
  // loop over them and print some information.
  cout << "\nDetected " << blocks.size() << " blocks.\n";

  for (auto& block : blocks) {
    cout << "\n";
    cout << "    Type:          " << block_type(block->getType())
	 << " (" << block->getType() << ")\n";
    cout << "    Description:   " << block->getDeviceDescription()
	 << (block->isMasterBlock()?" ** MASTER BLOCK **":"") << "\n";
    cout << "    Battery level: " << block->getBatteryLevel()
	 << (block->isBatteryCharging()?" (charging)":" (charged)") << "\n";
    cout << "    UID:           " << String::toHexString(block->uid) << "\n";
    cout << "    Serial number: " << block->serialNumber << "\n";
#if 0
    cout << "    Config items:  " << block->getMaxConfigIndex() << "\n";
    for (int i = 0; i < block->getMaxConfigIndex(); i++) {
      auto data = block->getLocalConfigMetaData(i);
      if (data.name == "") continue;
      cout << "item #" << i << ": " << data.name << " = " << data.value << String(block->isValidUserConfigIndex(i)?" (user)":"") << "\n";
    }
#endif
#if 0
    if (Block::Program *prog = block->getProgram()) {
      cout << "    code:\n" << prog->getLittleFootProgram();
    }
#endif
  }
#endif
}

bool BlockFinder::Changed()
{
  bool res = changed;
  changed = false;
  return res;
}

int BlockFinder::CountBlocks()
{
  return blocks.size();
}

bool BlockFinder::GetMsg(int &blocknum, int msg[3],
			 myblocks_button_info_t &button_info)
{
  Msg *front = msgs.removeNext();
  if (front) {
    blocknum = front->n;
    if (front->button) {
      static std::string name;
      name = front->name.toStdString();
      button_info.name = name.c_str();
      button_info.num = front->num;
      button_info.type = front->type;
      button_info.pressed = front->button == 1;
      memset(msg, 0, 3*sizeof(int));
    } else {
      button_info = { .name = 0 };
      memcpy(msg, front->msg, 3*sizeof(int));
    }
    delete front;
    return true;
  } else
    return false;
}

Block *BlockFinder::GetBlock(int blocknum)
{
  if (blocknum >= 0 && blocknum < blocks.size())
    return blocks.getObjectPointerUnchecked(blocknum);
  else
    return 0;
}

ControlButton *BlockFinder::GetButton(Block *block, int num)
{
  if (num >= 0 && num < block->getButtons().size())
    return block->getButtons().getUnchecked(num);
  else
    return 0;
}

struct BlockProgram : Block::Program
{
  BlockProgram(Block &b, String _code) : Block::Program(b), code(_code) {}
  String getLittleFootProgram() override { return code; }
  String code;
};

// XXXFIXME: Due to limitations in the BLOCKS SDK, SetProgram() isn't quite
// the same as what the Roli Dashboard or the IDE provides, which apparently
// have some hidden magic built into them to pick up the XML configuration
// data in the code. I couldn't find this in the SDK anywhere, so all
// SetProgram() currently does is compile and load the program on the block.
// This will generally work with simple, self-contained programs which don't
// rely on any configuration data (some programs which only need the factory
// configuration will work as well). Otherwise you'll get an error message
// indicating that some variable was not defined.

bool BlockFinder::SetProgram(int blocknum, String code, String& msg)
{
  if (blocknum >= 0 && blocknum < blocks.size()) {
    Block *block = blocks.getObjectPointerUnchecked(blocknum);
    BlockProgram *prog = new BlockProgram(*block, code);
    Result res = block->setProgram(prog);
    if (res.failed()) {
      msg = res.getErrorMessage();
      return false;
    } else {
      msg = "Ok";
      return true;
    }
  } else
    return false;
}

void BlockFinder::SaveProgram(int blocknum)
{
  if (blocknum >= 0 && blocknum < blocks.size()) {
    Block *block = blocks.getObjectPointerUnchecked(blocknum);
    block->saveProgramAsDefault();
  }
}

void BlockFinder::Reset(int blocknum)
{
  if (blocknum >= 0 && blocknum < blocks.size()) {
    Block *block = blocks.getObjectPointerUnchecked(blocknum);
    block->blockReset();
  }
}

void BlockFinder::FactoryReset(int blocknum)
{
  if (blocknum >= 0 && blocknum < blocks.size()) {
    Block *block = blocks.getObjectPointerUnchecked(blocknum);
    block->factoryReset();
    // Once is not enough?
    block->factoryReset();
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


// C API ---------------------------------------------------------------------

extern "C" bool myblocks_changed()
{
  if (app)
    return app->finder.Changed();
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

extern "C" bool myblocks_set_program(int blocknum, const char *code)
{
  if (app) {
    String msg;
    bool res = app->finder.SetProgram(blocknum, code, msg);
    myblocks_msg_string = msg.toStdString();
    return res;
  } else {
    myblocks_msg_string = "JUCE not running";
    return false;
  }
}

#include <sys/types.h>
#include <sys/stat.h>

extern "C" bool myblocks_load_program(int blocknum, const char *filename)
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
    bool res = myblocks_set_program(blocknum, code);
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

extern "C" void myblocks_save_program(int blocknum)
{
  if (app) app->finder.SaveProgram(blocknum);
}

extern "C" void myblocks_reset(int blocknum)
{
  if (app) app->finder.Reset(blocknum);
}

extern "C" void myblocks_factory_reset(int blocknum)
{
  if (app) app->finder.FactoryReset(blocknum);
}

static bool isControlBlock(Block *block)
{
  auto type = block->getType();
  return type == Block::Type::liveBlock
    || type == Block::Type::loopBlock
    || type == Block::Type::touchBlock
    || type == Block::Type::developerControlBlock;
}

extern "C" bool myblocks_info(int blocknum, myblocks_info_t *info)
{
  if (app) {
    static std::string descr, serial, version, code;
    Block *block = app->finder.GetBlock(blocknum);
    if (!block || !info) return false;
    descr = block->getDeviceDescription().toStdString();
    serial = block->serialNumber.toStdString();
    version = block->versionNumber.toStdString();
    info->uid = block->uid;
    info->type = block->getType();
    info->descr = descr.c_str();
    info->serial = serial.c_str();
    info->version = version.c_str();
    info->type_descr = block_type((Block::Type)info->type);
    info->battery_level = block->getBatteryLevel();
    info->is_charging = block->isBatteryCharging();
    info->is_master = block->isMasterBlock();
    if (isControlBlock(block)) {
      info->nbuttons = block->getButtons().size();
      if (auto ledRow = block->getLEDRow())
	info->nleds = ledRow->getNumLEDs();
      else
	info->nleds = 0;
    } else {
      info->nbuttons = info->nleds = 0;
    }
    if (Block::Program *prog = block->getProgram()) {
      code = prog->getLittleFootProgram().toStdString();
      info->code = code.c_str();
    } else
      info->code = 0;
    return true;
  } else
    return false;
}

extern "C" void myblocks_set(int blocknum, size_t offset, uint8_t data)
{
  if (app) {
    Block *block = app->finder.GetBlock(blocknum);
    if (!block) return;
    block->setDataByte(offset, data);
  }
}

extern "C" uint8_t myblocks_get(int blocknum, size_t offset)
{
  if (app) {
    Block *block = app->finder.GetBlock(blocknum);
    if (!block) return 0;
    return block->getDataByte(offset);
  }
  return 0;
}

extern "C" void myblocks_send(int blocknum, int msg[3])
{
  if (app) {
    Block *block = app->finder.GetBlock(blocknum);
    if (!block) return;
    Block::ProgramEventMessage ev = { msg[0], msg[1], msg[2] };
    block->sendProgramEvent(ev);
  }
}

extern "C" bool myblocks_receive(int *blocknum, int msg[3],
				 myblocks_button_info_t *button_info)
{
  if (app)
    return app->finder.GetMsg(*blocknum, msg, *button_info);
  else
    return false;
}

extern "C" void myblocks_set_button(int blocknum, int num, unsigned color)
{
  if (app) {
    Block *block = app->finder.GetBlock(blocknum);
    if (!block || !isControlBlock(block)) return;
    auto button = app->finder.GetButton(block, num);
    if (!button) return;
    if (button->hasLight())
      button->setLightColour(color);
  }
}

extern "C" void myblocks_set_leds(int blocknum, int num, unsigned color)
{
  if (app) {
    Block *block = app->finder.GetBlock(blocknum);
    if (!block || !isControlBlock(block)) return;
    auto ledRow = block->getLEDRow();
    if (!ledRow) return;
    for (int i = 0; i < ledRow->getNumLEDs(); ++i)
      ledRow->setLEDColour(i, (i < num) ? color : 0);
  }
}
