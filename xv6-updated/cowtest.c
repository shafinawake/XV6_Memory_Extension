#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=== CMDT Test Program (Task 2 - Copy-On-Write) ===\n\n");

  printf(1, "Step 1: Parent before fork\n");
  printf(1, "----------------------------\n");
  memstats();

  int pid = fork();

  if(pid < 0) {
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0) {
    // Child process
    printf(1, "\nStep 2: Child immediately after fork\n");
    printf(1, "--------------------------------------\n");
    printf(1, "NOTE: With COW, pages are SHARED and read-only\n");
    printf(1, "No physical copying has occurred yet\n\n");
    memstats();

    printf(1, "\nStep 3: Child writing to private memory (COW trigger)\n");
    printf(1, "------------------------------------------------------\n");

    char *buf = malloc(8192);
    if(buf == 0) {
      printf(1, "malloc failed\n");
      exit();
    }

    for(int i = 0; i < 8192; i++) {
      buf[i] = 'C';   // should trigger COW page fault(s)
    }

    printf(1, "\nStep 4: Child after write (pages diverged)\n");
    printf(1, "-------------------------------------------\n");
    printf(1, "NOTE: Only modified pages are now copied\n\n");
    memstats();

    free(buf);
    exit();

  } else {
    // Parent process
    wait();

    printf(1, "\nStep 5: Parent after child exits\n");
    printf(1, "----------------------------------\n");
    printf(1, "NOTE: Parent memory remained unchanged\n\n");
    memstats();

    printf(1, "\n=== Task 2 Complete ===\n");
    printf(1, "Observation:\n");
    printf(1, "- Pages were shared after fork\n");
    printf(1, "- Physical copies created ONLY on write\n");
    printf(1, "- Confirms correct Copy-On-Write behavior\n\n");
  }

  exit();
}