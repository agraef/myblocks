#ifndef MYBLOCKS_H
#define MYBLOCKS_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// JUCE interface: initialize, finalize, process events
extern void juce_init(void);
extern void juce_fini(void);
extern bool juce_process_events(void);

// Wrappers for BLOCKS SDK operations.
// This flag becomes true whenever the block topology changes.
extern bool myblocks_changed(void);
// Number of connected blocks.
extern int  myblocks_count_blocks(void);
// Load a Littlefoot program on the block, given its source code.
extern bool myblocks_set_program(int blocknum, const char *code);
// Load a Littlefoot program on the block, given its source file name.
extern bool myblocks_load_program(int blocknum, const char *filename);
// Saves the program on the block so that it becomes permanent.
extern void myblocks_save_program(int blocknum);
// Reset the block.
extern void myblocks_reset(int blocknum);
// Factory reset (resets the block to its factory configuration).
extern void myblocks_factory_reset(int blocknum);

// Reports an error message from the most recent Littlefoot compilation. Set
// by myblocks_set_program() and myblocks_load_program(), "Ok" if no error.
extern const char *myblocks_msg();

// Retrieve information about blocks. This fills the following struct.
typedef struct {
  // unique identifier of the block (may change between sessions?)
  uint64_t uid;
  // type of block (0 = unknown, 1 = LightPad etc.)
  int type;
  // flags (is this the master block? is it being charged?)
  bool is_master, is_charging;
  // current battery level
  float battery_level;
  // number of buttons and size of the LED strip, if any (control blocks)
  int nbuttons, nleds;
  // Various useful textual information (block description, human-readable
  // description of the block type, serial number, version number of firmware,
  // Littlefoot code running on the device if any). NOTE: These fields all
  // point to static storage which is overridden each time myblocks_info() is
  // called. The Littlefoot code will only be available if it was set in this
  // session (i.e., if it is a control block, or if myblocks_set_program() was
  // used), otherwise this field will be NULL.
  const char *descr, *type_descr, *serial, *version;
} myblocks_info_t;

extern bool myblocks_info(int blocknum, myblocks_info_t *info);

typedef struct {
  const char *name; // non-null indicates a button
  int num, type;    // button number and type
  bool pressed;     // true => pressed, false => released
} myblocks_button_info_t;

extern void myblocks_set(int blocknum, size_t offset, uint8_t data);
extern uint8_t myblocks_get(int blocknum, size_t offset);
extern void myblocks_set_button(int blocknum, int num, unsigned color);
extern void myblocks_set_leds(int blocknum, int num, unsigned color);
extern void myblocks_send(int blocknum, int msg[3]);
extern bool myblocks_receive(int *blocknum, int msg[3],
			     myblocks_button_info_t *button_info);

// XXXTODO: Operations to report touch and button events, and set LEDS and
// button colors on the devices.

#ifdef __cplusplus
}
#endif

#endif
