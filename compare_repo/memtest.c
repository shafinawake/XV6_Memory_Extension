#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=== CMDT Test Program (Task 1 - No COW) ===\n\n");
  
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
    printf(1, "NOTE: Without COW, all pages are copied (private)\n\n");
    memstats();
    
    printf(1, "\nStep 3: Child allocating and writing 8KB buffer\n");
    printf(1, "-------------------------------------------------\n");
    
    char *buf = malloc(8192);
    if(buf == 0) {
      printf(1, "malloc failed\n");
      exit();
    }
    
    for(int i = 0; i < 8192; i++) {
      buf[i] = 'X';
    }
    
    printf(1, "\nStep 4: Child after modifications\n");
    printf(1, "-----------------------------------\n");
    memstats();
    
    free(buf);
    exit();
    
  } else {
    // Parent process
    wait();
    
    printf(1, "\nStep 5: Parent after child exits\n");
    printf(1, "----------------------------------\n");
    memstats();
    
    printf(1, "\n=== Task 1 Complete ===\n");
    printf(1, "Observation: All pages are private (copied at fork)\n");
    printf(1, "This demonstrates the PROBLEM that Task 2 (COW) will solve.\n\n");
  }
  
  exit();
}