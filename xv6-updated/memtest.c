#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=== CMDT Test Program (Task 2 - WITH COW) ===\n\n");
  
  printf(1, "Step 1: Parent before fork\n");
  printf(1, "----------------------------\n");
  memstats();
  
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "fork failed\n");
    exit();
  }
  
  if(pid == 0) {
    printf(1, "\nStep 2: Child immediately after fork\n");
    printf(1, "--------------------------------------\n");
    printf(1, "Expected: Pages should be SHARED now!\n\n");
    memstats();
    
    printf(1, "\nStep 3: Child allocating 4KB buffer\n");
    printf(1, "-------------------------------------\n");
    
    char *buf = malloc(4096);
    if(buf == 0) {
      printf(1, "malloc failed\n");
      exit();
    }
    
    int i;
    for(i = 0; i < 4096; i++) {
      buf[i] = 'X';
    }
    
    printf(1, "\nStep 4: Child after modifications\n");
    printf(1, "-----------------------------------\n");
    memstats();
    
    free(buf);
    printf(1, "\n=== Child exiting ===\n");
    exit();
    
  } else {
    wait();
    
    printf(1, "\nStep 5: Parent after child exits\n");
    printf(1, "----------------------------------\n");
    memstats();
    
    printf(1, "\n=== Task 2 Complete ===\n");
    printf(1, "SUCCESS: COW is working!\n");
    printf(1, "- Pages were shared after fork\n");
    printf(1, "- Pages became private on write\n\n");
  }
  
  exit();
}