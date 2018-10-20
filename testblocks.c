
// A little test program for the BLOCKS SDK, building on the stuff in
// myblocks.cc.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "myblocks.h"

int main(int argc, char *argv[])
{
  juce_init();
  // Wait until the devices are online.
  printf("Waiting for blocks to be connected.\n");
  while (juce_process_events() && !myblocks_count_blocks()) ;
  // Presumably we're ready now, load Littlefoot code into the master block if
  // specified on the command line.
  if (myblocks_count_blocks() && argc > 1) {
    printf("\nLoading %s.\n", argv[1]);
    if (myblocks_load_program(0, false, argv[1]))
      printf("Successfully loaded %s.\n", argv[1]);
    else
      fprintf(stderr, "Error: %s\n", myblocks_msg());
  }
  // Keep processing events until we're interrupted or the devices go offline.
  while (juce_process_events() && myblocks_count_blocks()) {
    usleep(1000);
  }
  printf("No more connected blocks, exiting.\n");
  juce_fini();
}
