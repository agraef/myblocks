
// A little test program for the BLOCKS SDK, building on the stuff in
// myblocks.cc.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    struct stat st;
    if (!stat(argv[1], &st)) {
      char *code = calloc(st.st_size, 1);
      FILE *fp = fopen(argv[1], "r");
      if (fp && fread(code, 1, st.st_size, fp) == (size_t)st.st_size) {
	if (myblocks_set_program(0, false, code))
	  printf("Successfully loaded %s.\n", argv[1]);
	else
	  fprintf(stderr, "Error: %s\n", myblocks_msg());
      } else
	perror(argv[1]);
      if (fp) fclose(fp);
    } else {
      perror(argv[1]);
    }
  }
  // Keep processing events until we're interrupted or the devices go offline.
  while (juce_process_events() && myblocks_count_blocks()) {
    usleep(1000);
  }
  printf("No more connected blocks, exiting.\n");
  juce_fini();
}
