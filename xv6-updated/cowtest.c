#include "types.h"
#include "stat.h"
#include "user.h"

void
test_simple_cow()
{
  printf(1, "\n================================================\n");
  printf(1, "  TEST 1: Simple COW Behavior\n");
  printf(1, "================================================\n\n");
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] Immediately after fork:\n");
    memstats();
    
    printf(1, "\n[CHILD] Allocating and writing to trigger COW...\n");
    char *ptr = malloc(100);
    if(ptr) {
      ptr[0] = 'X';
      free(ptr);
    }
    
    printf(1, "\n[CHILD] After write:\n");
    memstats();
    
    exit();
  } else {
    wait();
    printf(1, "\n[PARENT] After child completes:\n");
    memstats();
  }
}