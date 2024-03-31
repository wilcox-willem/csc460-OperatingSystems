// Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/30/24
// p8: Cristina's Lively Lunch

//  MY UNDERSTANDING OF HOW TO USE SEMAPHORES 
/*

### To Create ###
  sem_id = semget (IPC_PRIVATE, HowManySems, 0777);

### To Set ###
  semctl(sem_id, posInSemArray, SETVAL, 1);

### To down/wait/p ###
  p(int s,int sem_id);

### To up/signal/v ###
  v(int s,int sem_id);

### To Clean up ###
  if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
    printf("ERROR in removing sem\n");

*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT 0
#define RIGHT 1



main(int argc, char *argv[]) {
  int i, semID, shmemID_sticks, shmemID_states;
  int *shmemArray_sticks, *shmemArray_states;
  int N = 5;                  // Holds the number of procs/sems to generate
  int myID = 0;               // used to identify procs in sync
  int mySticks[2] = {0,0};    // used to track sticks held (0 = L, 1 = R)
  int semArray[N];            // used to hold sem's for chopstick access
  char tabsNeeded[5] = "\t";  // used to hold tabs for each proc
  char tabSpare[1] = "\t";    // spare tab for my sake
  int state[N];               // represents each philosphers state



  /******************** PREP BEFORE STARTING *********************/

  /*****  Make a note of "who" is the first Process *****/
  int firstID = getpid();

  // initialize shmem
  shmemID_sticks =  shmget(IPC_PRIVATE, N*sizeof(int), 0770);
  if (shmemID_sticks != -1) {
    shmemArray_sticks = (int *) shmat(shmemID_sticks, NULL, SHM_RND);
  }
  else {
    printf("shmem error\n");
    return (1);
  }
  
  /***** Get Semaphores *****/
  semID = semget (IPC_PRIVATE, N+1, 0777);

  // check if sem array made
  if (semID == -1) {
    printf("%s","SemGet Failed.\n");
    return (-1);
  }


  // initialize sems/arrays (initial val 1 = ready)
  for (i = 0; i < N; i++) {
    semctl(semID, i, SETVAL, 1);
    semArray[i] = i;
    shmemArray_sticks[i] = 1;
  }
 

  /*****  Spawn all the Processes *****/
  for (i = 1; i < N; i++) {
    if (fork() > 0) break; // send parent on to Body
    myID++;
    strcat(tabsNeeded, tabSpare); // adds another tab
  }

  /***** Seed rand w/ time *****/
  srand(myID);
	
  /******************** STARTING MAIN BODY *********************/

  while (1) {
  // as all great philosophers must do, THINK!
    if (mySticks[0] == 0 && mySticks[1] == 0) {
      printf("%s%d THINKING\n", tabsNeeded, myID);
    }
  // as even greater philosophers must do, HUNGER!
    if (mySticks[0] == 1 || mySticks[1] == 1) {
      printf("%s%d HUNGRY\n", tabsNeeded, myID);
    }    

    // try to pick up Left chopstick
    pick_up_chopstick(0, myID, semArray, shmemArray_sticks, mySticks);
    
    // try to pick up Right chopstick
    pick_up_chopstick(1, myID, semArray, shmemArray_sticks, mySticks);

  // as the greatest philosophers must do, EAT!
    if (mySticks[0] == 1 && mySticks[1] == 1) {
      printf("%s%d EATING\n", tabsNeeded, myID);
    }

    // put down Left
    put_down_chopstick(0, myID, semArray, shmemArray_sticks, mySticks);

    // put down Right
    put_down_chopstick(1, myID, semArray, shmemArray_sticks, mySticks);
     
  }


  /******************** ENDING MAIN BODY *********************/  

  // clean up, ONLY need original process to do this
  if (firstID == getpid()) {
    sleep(2);
    if ((semctl(semID, 0, IPC_RMID, 0)) == -1) {
      printf("%s", "Parent: ERROR in removing sem\n");
    }
  }

  return(0);
}

/***** Funcs for C's Lvly *****/
pick_up_chopstick(int side, int myID, int semID, int shmemArray_sticks[], int mySticks[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // check if safe
  p(spotToCheck, semID);
  // ENTERING CRITICAL ZONE

  if (mySticks[side] == 0 && shmemArray_sticks[spotToCheck] == 1) {
    shmemArray_sticks[spotToCheck] -= 1;
    mySticks[side] += 1;
  }

  // EXITING CRITICAL ZONE
  // signal and leave
  v(spotToCheck, semID);
}

put_down_chopstick(int side, int myID, int semID, int shmemArray_sticks[], int mySticks[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // check if safe
  p(spotToCheck, semID);
  // ENTERING CRITICAL ZONE

  if (mySticks[side] == 1 && shmemArray_sticks[spotToCheck] == 0) {
    shmemArray_sticks[spotToCheck] += 1;
    mySticks[side] -= 1;
  }

  // EXITING CRITICAL ZONE
  // signal and leave
  v(spotToCheck, semID);
}


/***** Semaphore Functions *****/
p(int s,int semID) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}


v(int s, int semID) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'V' error\n");
}
