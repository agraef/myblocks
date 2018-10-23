
// A little test program for the BLOCKS SDK, building on the stuff in
// myblocks.cc.

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "myblocks.h"

int main(int argc, char *argv[])
{
  juce_init();
  // Wait until the devices are online.
  printf("Waiting for blocks to be connected.\n");
  while (juce_process_events() && !myblocks_count_blocks()) ;
  // Presumably we're ready now, do some initialization of the master block if
  // requested.
  if (myblocks_count_blocks() && argc > 1) {
    printf("\nInitializing the master block:\n");
    if (!strcmp(argv[1], "reset")) {
      // Do a factory reset.
      printf("Factory reset.\n");
      myblocks_factory_reset(0);
    } else {
      // Load Littlefoot code.
      printf("Loading %s.\n", argv[1]);
      if (myblocks_load_program(0, argv[1])) {
	printf("Successfully loaded %s.\n", argv[1]);
	// Save the program on the device if requested.
	if (argv[2] && !strcmp(argv[2], "save")) {
	  printf("Saving program.\n");
	  myblocks_save_program(0);
	}
      } else
	fprintf(stderr, "Error: %s\n", myblocks_msg());
    }
  }
  // Keep processing events until we're interrupted or the devices go offline.
  while (juce_process_events() && myblocks_count_blocks()) {
    if (myblocks_changed()) {
      printf("\nTopology change:\n");
      for (int i = 0, n = myblocks_count_blocks(); i < n; i++) {
	myblocks_info_t info;
	if (myblocks_info(i, &info))
	  printf("%d: %0lx %d %-8s %2d %2d %s%s\n", i,
		 info.uid, info.type, info.type_descr,
		 info.nbuttons, info.nleds,
		 info.descr, info.is_master?" ** MASTER **":"");
      }
    }
    int blocknum, msg[3];
    myblocks_button_info_t button_info;
    while (myblocks_receive(&blocknum, msg, &button_info))
      if (button_info.name)
	printf("%d: button %d %d (%s) %d\n", blocknum, button_info.num,
	       button_info.type, button_info.name, button_info.pressed);
      else
	printf("%d: msg %d %d %d\n", blocknum, msg[0], msg[1], msg[2]);
    usleep(1000);
  }
  printf("No more connected blocks, exiting.\n");
  juce_fini();
}
