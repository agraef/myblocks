#ifndef MYBLOCKS_H
#define MYBLOCKS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void juce_init(void);
extern void juce_fini(void);
extern bool juce_process_events(void);
extern bool myblocks_changed(void);
extern int  myblocks_count_blocks(void);
extern bool myblocks_set_program(int blocknum, const char *code);
extern bool myblocks_load_program(int blocknum, const char *filename);
extern void myblocks_save_program(int blocknum);
extern void myblocks_reset(int blocknum);
extern void myblocks_factory_reset(int blocknum);

typedef struct {
  uint64_t uid;
  int type;
  const char *descr, *type_descr, *serial, *version;
  float battery_level;
  bool is_master, is_charging;
} myblocks_info_t;

extern bool myblocks_info(int blocknum, myblocks_info_t *info);

// Only used by myblocks_set_program() right now.
extern const char *myblocks_msg();

#ifdef __cplusplus
}
#endif

#endif
