#ifndef MYBLOCKS_H
#define MYBLOCKS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Only used by myblocks_set_program() right now.
extern const char *myblocks_msg();

extern void juce_init(void);
extern void juce_fini(void);
extern bool juce_process_events(void);
extern int  myblocks_count_blocks(void);
extern bool myblocks_set_program(int blocknum, bool save, const char *code);
extern void myblocks_reset(int blocknum, bool factory);

#ifdef __cplusplus
}
#endif

#endif
