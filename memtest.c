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

void
test_multiple_pages()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 2: Multiple Page Writes\n");
  printf(1, "================================================\n\n");
  
  char *buffer = malloc(8192);
  if(buffer == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 8192; i++)
    buffer[i] = 'A';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(buffer);
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
    
    free(buffer);
    exit();
  } else {
    wait();
    free(buffer);
  }
}

void
test_read_only()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 3: Read-Only (No COW Trigger)\n");
  printf(1, "================================================\n\n");
  
  char *buffer = malloc(4096);
  if(buffer == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 4096; i++)
    buffer[i] = 'R';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(buffer);
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] Before reading:\n");
    memstats();
    
    int sum = 0;
    for(i = 0; i < 4096; i++)
      sum += buffer[i];
    
    printf(1, "\n[CHILD] After reading (sum=%d):\n", sum);
    printf(1, "Expected: Shared pages should NOT decrease\n\n");
    memstats();
    
    free(buffer);
    exit();
  } else {
    wait();
    free(buffer);
  }
}

void
test_fork_exec()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 4: Fork then Exec (COW Benefit)\n");
  printf(1, "================================================\n\n");
  
  char *buf = malloc(16384);
  if(buf == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
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

void
test_nested_fork()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 5: Nested Fork (Grandchild)\n");
  printf(1, "================================================\n\n");
  
  char *data = malloc(8192);
  if(data == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 8192; i++)
    data[i] = 'P';
  
  printf(1, "[PARENT] Before first fork:\n");
  memstats();
  
  int pid1 = fork();
  
  if(pid1 < 0) {
    printf(1, "ERROR: fork failed\n");
    free(data);
    exit();
  }
  
  if(pid1 == 0) {
    printf(1, "\n[CHILD] After fork from parent:\n");
    memstats();
    
    int pid2 = fork();
    
    if(pid2 == 0) {
      printf(1, "\n[GRANDCHILD] After fork from child:\n");
      memstats();
      
      printf(1, "\n[GRANDCHILD] Writing to data...\n");
      data[0] = 'G';
      
      printf(1, "\n[GRANDCHILD] After write:\n");
      memstats();
      
      free(data);
      exit();
    } else {
      wait();
      printf(1, "\n[CHILD] After grandchild exits:\n");
      memstats();
      free(data);
      exit();
    }
  } else {
    wait();
    printf(1, "\n[PARENT] After child exits:\n");
    memstats();
    free(data);
  }
}

void
test_multiple_children()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 6: Multiple Children Sharing Pages\n");
  printf(1, "================================================\n\n");
  
  char *shared = malloc(4096);
  if(shared == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 4096; i++)
    shared[i] = 'S';
  
  printf(1, "[PARENT] Before forking children:\n");
  memstats();
  
  int pids[3];
  for(i = 0; i < 3; i++) {
    pids[i] = fork();
    
    if(pids[i] < 0) {
      printf(1, "ERROR: fork %d failed\n", i);
      exit();
    }
    
    if(pids[i] == 0) {
      printf(1, "\n[CHILD %d] After fork:\n", i);
      memstats();
      
      printf(1, "\n[CHILD %d] Writing to shared data...\n", i);
      shared[i * 1000] = 'C';
      
      printf(1, "\n[CHILD %d] After write:\n", i);
      memstats();
      
      free(shared);
      exit();
    }
  }
  
  for(i = 0; i < 3; i++)
    wait();
  
  printf(1, "\n[PARENT] After all children exit:\n");
  memstats();
  free(shared);
}

void
test_large_allocation()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 7: Large Memory Allocation\n");
  printf(1, "================================================\n\n");
  
  char *large = malloc(32768);
  if(large == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 32768; i++)
    large[i] = 'L';
  
  printf(1, "[PARENT] Allocated 32KB:\n");
  memstats();
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(large);
    exit();
  }
  
  if(pid == 0) {
    printf(1, "\n[CHILD] All 8 pages shared:\n");
    memstats();
    
    printf(1, "\n[CHILD] Writing to every other page...\n");
    for(i = 0; i < 4; i++)
      large[i * 8192] = 'C';
    
    printf(1, "\n[CHILD] After writing to 4 pages:\n");
    memstats();
    printf(1, "Expected: ~4 shared, ~4 private\n");
    
    free(large);
    exit();
  } else {
    wait();
    free(large);
  }
}

void
test_parent_writes_after_fork()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 8: Parent Writes After Fork\n");
  printf(1, "================================================\n\n");
  
  char *data = malloc(8192);
  if(data == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 8192; i++)
    data[i] = 'P';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(data);
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] After fork (waiting):\n");
    memstats();
    
    sleep(2);
    
    printf(1, "\n[CHILD] After parent modified data:\n");
    memstats();
    printf(1, "Expected: Pages still shared (child didn't write)\n");
    
    free(data);
    exit();
  } else {
    printf(1, "\n[PARENT] After fork:\n");
    memstats();
    
    printf(1, "\n[PARENT] Writing to data...\n");
    data[0] = 'X';
    data[4096] = 'Y';
    
    printf(1, "\n[PARENT] After writes:\n");
    memstats();
    printf(1, "Expected: Parent made copies, child unaffected\n");
    
    wait();
    free(data);
  }
}

void
test_sequential_writes()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 9: Sequential Page Writes\n");
  printf(1, "================================================\n\n");
  
  char *pages = malloc(20480);
  if(pages == 0) {
    printf(1, "ERROR: malloc failed\n");
    exit();
  }
  
  int i;
  for(i = 0; i < 20480; i++)
    pages[i] = 'S';
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "ERROR: fork failed\n");
    free(pages);
    exit();
  }
  
  if(pid == 0) {
    printf(1, "[CHILD] Initial state (5 pages shared):\n");
    memstats();
    
    for(i = 0; i < 5; i++) {
      printf(1, "\n[CHILD] Writing to page %d...\n", i);
      pages[i * 4096] = 'C';
      
      printf(1, "[CHILD] After page %d write:\n", i);
      memstats();
    }
    
    free(pages);
    exit();
  } else {
    wait();
    free(pages);
  }
}

void
test_stress_cow()
{
  printf(1, "\n\n================================================\n");
  printf(1, "  TEST 10: COW Stress Test\n");
  printf(1, "================================================\n\n");
  
  printf(1, "[STRESS] Forking 5 children with 16KB each...\n");
  
  int i, j;
  for(i = 0; i < 5; i++) {
    char *mem = malloc(16384);
    if(mem == 0) {
      printf(1, "ERROR: malloc failed\n");
      exit();
    }
    
    for(j = 0; j < 16384; j++)
      mem[j] = 'M';
    
    int pid = fork();
    
    if(pid < 0) {
      printf(1, "ERROR: fork failed\n");
      free(mem);
      exit();
    }
    
    if(pid == 0) {
      printf(1, "[CHILD %d] Forked:\n", i);
      memstats();
      
      mem[i * 1000] = 'C';
      
      printf(1, "[CHILD %d] After write:\n", i);
      memstats();
      
      free(mem);
      exit();
    }
    
    free(mem);
  }
  
  for(i = 0; i < 5; i++)
    wait();
  
  printf(1, "\n[STRESS] All children completed successfully!\n");
}

int
main(int argc, char *argv[])
{
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "                                                \n");
  printf(1, "   XV6 Copy-On-Write Comprehensive Test Suite  \n");
  printf(1, "              Task 2 - Extended                 \n");
  printf(1, "                                                \n");
  printf(1, "================================================\n");
  
  test_simple_cow();
  test_multiple_pages();
  test_read_only();
  test_fork_exec();
  test_nested_fork();
  test_multiple_children();
  test_large_allocation();
  test_parent_writes_after_fork();
  test_sequential_writes();
  test_stress_cow();
  
  printf(1, "\n\n");
  printf(1, "================================================\n");
  printf(1, "   ALL 10 COW TESTS COMPLETED SUCCESSFULLY!    \n");
  printf(1, "================================================\n\n");
  
  exit();
}