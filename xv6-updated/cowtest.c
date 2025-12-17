#include "types.h"
#include "stat.h"
#include "user.h"

void
test_simple_fork()
{
  printf(1, "\n================================================\n");
  printf(1, "  TEST: Simple Fork and Write\n");
  printf(1, "================================================\n\n");
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] After fork:\n");
    memstats();
    
    printf(1, "\n[CHILD] Writing to trigger COW...\n");
    int x = 100;
    x = x + 1;
    
    printf(1, "\n[CHILD] After write:\n");
    memstats();
    
    exit();
  } else {
    wait();
    printf(1, "\n[PARENT] After child exits:\n");
    memstats();
  }
}

int
main(int argc, char *argv[])
{
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "     XV6 Copy-On-Write Test\n");
  printf(1, "================================================\n");
  
  test_simple_fork();
  
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "  COW Test Complete\n");
  printf(1, "================================================\n\n");
  
  exit();
}