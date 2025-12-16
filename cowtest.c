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
    // Child
    printf(1, "[CHILD] Immediately after fork:\n");
    memstats();
    
    printf(1, "\n[CHILD] Writing one byte to trigger COW...\n");
    volatile char *ptr = (char*)0x3000;
    *ptr = 'X';
    
    printf(1, "\n[CHILD] After write:\n");
    memstats();
    
    exit();
  } else {
    // Parent
    wait();
    printf(1, "\n[PARENT] After child completes:\n");
    memstats();
  }
}

void
test_multiple_pages()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 2: Multiple Page Writes\n");
  printf(1, "================================================\n\n");
  
  static char buffer[8192];  // 2 pages
  int i;
  for(i = 0; i < 8192; i++)
    buffer[i] = 'A';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] Initial state:\n");
    memstats();
    
    printf(1, "\n[CHILD] Writing to first page...\n");
    buffer[0] = 'X';
    
    printf(1, "\n[CHILD] After page 1 write:\n");
    memstats();
    
    printf(1, "\n[CHILD] Writing to second page...\n");
    buffer[4096] = 'Y';
    
    printf(1, "\n[CHILD] After page 2 write:\n");
    memstats();
    
    exit();
  } else {
    wait();
  }
}

void
test_read_only()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 3: Read-Only (No COW Trigger)\n");
  printf(1, "================================================\n\n");
  
  static char buffer[4096];
  int i;
  for(i = 0; i < 4096; i++)
    buffer[i] = 'R';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] Before reading:\n");
    memstats();
    
    // Only read, no writes
    int sum = 0;
    for(i = 0; i < 4096; i++)
      sum += buffer[i];
    
    printf(1, "\n[CHILD] After reading (sum=%d):\n", sum);
    printf(1, "Expected: Shared pages should NOT decrease\n\n");
    memstats();
    
    exit();
  } else {
    wait();
  }
}

void
test_fork_exec()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 4: Fork then Exec (COW Benefit)\n");
  printf(1, "================================================\n\n");
  
  char *buf = malloc(16384);  // 4 pages
  int i;
  
  if(buf == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  for(i = 0; i < 16384; i++)
    buf[i] = 'B';
  
  printf(1, "[PARENT] Before fork (large allocation):\n");
  memstats();
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(buf);
    exit();
  }
  
  if(pid == 0) {
    printf(1, "\n[CHILD] After fork:\n");
    printf(1, "All pages shared! Exec will discard without copying.\n\n");
    memstats();
    
    printf(1, "\n[CHILD] Calling exec (replaces memory)...\n");
    char *args[] = { "echo", "COW", "avoided", "copying!", 0 };
    exec("echo", args);
    
    printf(1, "ERROR: exec failed\n");
    exit();
  } else {
    wait();
    free(buf);
    
    printf(1, "\n[PARENT] After child exec:\n");
    memstats();
    printf(1, "\nBenefit: Child never copied those 4 pages!\n");
  }
}

int
main(int argc, char *argv[])
{
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "                                                \n");
  printf(1, "     XV6 Copy-On-Write Test Suite              \n");
  printf(1, "              Task 2                            \n");
  printf(1, "                                                \n");
  printf(1, "================================================\n");
  
  test_simple_cow();
  test_multiple_pages();
  test_read_only();
  test_fork_exec();
  
  printf(1, "\n\n");
  printf(1, "================================================\n");
  printf(1, "  All COW Tests Completed Successfully\n");
  printf(1, "================================================\n\n");
  
  exit();
}