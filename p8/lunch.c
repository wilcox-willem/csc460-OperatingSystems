// Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/30/24
// p8: Cristina's Lively Lunch

//  MY UNDERSTANDING OF HOW TO USE SEMAPHORES 
/*

### To Create ###
  // this asks the OS for a sem array, 
  // size: HowManySems, 
  // access: 0777 (read and write)

  sem_id = semget (IPC_PRIVATE, HowManySems, 0777);

  // sem_id = int ID of sem array

### To Set ###
  semctl(sem_id, posInSemID, SETVAL, 1);

  // 1 (or > 0) = ready
  // 0 = not ready

### To down/wait/p ###
  p(int s,int sem_id);

  // s = posInSemID, decrements sem if > 0 or blocks

### To up/signal/v ###
  v(int s,int sem_id);

  // s = posInSemID, increments sem

### To Clean up ###
  // tells OS to remove sem
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

// states
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define DIE -1

#define LEFT 0
#define RIGHT 1
#define MUTEX 5



main(int argc, char *argv[]) {
  int i, semID, shmemID_sticks, shmemID_states;
  int *shmemArray_sticks, *shmemArray_states;
  int N = 5;                  // Holds the number of procs/sems to generate
  int myID = 0;               // used to identify procs in sync
  int mySticks[2] = {0,0};    // used to track sticks held (0 = L, 1 = R) Q
  int state[N];               // represents each philosphers state



  /******************** PREP BEFORE STARTING *********************/

  /*****  Make a note of "who" is the first Process *****/
  int firstID = getpid();

  // initialize shmem
  shmemID_sticks =  shmget(IPC_PRIVATE, N*sizeof(int), 0770); 

  shmemID_states =  shmget(IPC_PRIVATE, (N+1)*sizeof(int), 0770);
  if (shmemID_sticks != -1 && shmemID_states != -1) {
    shmemArray_sticks = (int *) shmat(shmemID_sticks, NULL, SHM_RND);
    shmemArray_states = (int *) shmat(shmemID_states, NULL, SHM_RND);
  }
  else {
    printf("shmem error\n");
    return (1);
  }
  
  /***** Get Semaphores *****/
  semID = semget (IPC_PRIVATE, N+1, 0777); // +1 for mutex sem

  // check if sem array made
  if (semID == -1) {
    printf("%s","SemGet Failed.\n");
    return (-1);
  }


  // initialize sems/arrays (initial val 1 = ready)
  for (i = 0; i < N; i++) {
    semctl(semID, i, SETVAL, 1);
    shmemArray_sticks[i] = 1;
  }

  // set mutex
  semctl(semID, MUTEX, SETVAL, 1);
 

  /*****  Spawn all the Processes *****/
  int firstFork = fork();

  if (fork() == 0) { // child starts forking
    for (i = 0; i < N; i++) {
      if (fork() > 0) break; // send parent on to Body
      myID++;
    }
  } else {
    myID = N;
  }

  /***** Seed rand w/ ID *****/
  srand(myID);
	
  /******************** STARTING MAIN BODY *********************/

  if (getpid() != firstID) {
    int theTime = shmemArray_states[N];

    while (theTime < 60) {
    // as all great philosophers must do, THINK!
      if (mySticks[0] == 0 && mySticks[1] == 0) {
        shmemArray_states[myID] = THINKING;
      }

    // as even greater philosophers must do, HUNGER!
      else if (mySticks[0] == 1 || mySticks[1] == 1) {
        shmemArray_states[myID] = HUNGRY;
          
        if (mySticks[LEFT] == 0) { 
          // try to pick up Left chopstick
          pick_up_chopstick(LEFT, myID, semID, shmemArray_sticks, mySticks);
        }
        if (mySticks[RIGHT] == 0) { 
          // try to pick up Right chopstick
          pick_up_chopstick(RIGHT, myID, semID, shmemArray_sticks, mySticks);
        }
      }

    // as the greatest philosophers must do, EAT!
      if (mySticks[0] == 1 && mySticks[1] == 1) {
        shmemArray_states[myID] = EATING;
        // put down Left
        put_down_chopstick(LEFT, myID, semID, shmemArray_sticks, mySticks);

        // put down Right
        put_down_chopstick(RIGHT, myID, semID, shmemArray_sticks, mySticks);
      }
      
    // update time 
      theTime = shmemArray_states[N];
    }

    // now that main loop is over, die!
    shmemArray_states[myID] = DIE;
  } else {
    /****** firstID process keeps track of time and prints states ******/

    int loopVal = 0;
    int masterTime = 0;
    char printStr[50] = " "; 

    for (masterTime = 0; masterTime <= 60; masterTime++) {
      shmemArray_states[N] = masterTime;
      
      printf("%d. ", masterTime);
      for (loopVal = ; loopVal < N; loopVal++){
        if (shmemArray_states[loopVal] == THINKING) printf("thinking  ");
        else if (shmemArray_states[loopVal] == HUNGRY) printf("hungry    ");
        else if (shmemArray_states[loopVal] == EATING) printf("eating    ");
        else if (shmemArray_states[loopVal] == DIE) printf("dead      ");
        else printf("error     ");
      }
      printf("\n");

      sleep(1);

    }
  }

  /******************** ENDING MAIN BODY *********************/  
  
  // start clean up! first proc only
  if (firstID == getpid()) {

    // check if ready for clean up
    int cleanUpReady = 0;
    int timeWaited = 0;

    while (cleanUpReady != 1 || timeWaited < 15) {
      cleanUpReady = 1;
      
      for (i = 0; i < N; i++) {
        if (shmemArray_states[i] != DIE) {
          cleanUpReady = 0;
        }
      }

      timeWaited++;

      sleep(1);
    }
  
    // clean up, sems
    if ((semctl(semID, 0, IPC_RMID, 0)) == -1)
      printf("%s", "Parent: ERROR in removing sem\n");
    

    // clean up, shmem
    if (shmdt(shmemID_sticks) == -1) 
      printf("shmgm: ERROR in detaching.\n");

    if ((shmctl(shmemID_sticks, IPC_RMID, NULL)) == -1)
      printf("ERROR removing shmem.\n");

    if (shmdt(shmemID_states) == -1) 
      printf("shmgm: ERROR in detaching.\n");

    if ((shmctl(shmemID_states, IPC_RMID, NULL)) == -1)
      printf("ERROR removing shmem.\n");
  }

  return(0);
}

/***** Funcs for C's Lvly *****/
pick_up_chopstick(int side, int myID, int semID, int shmemArray_sticks[], int mySticks[]){
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
  if((semop(semID, &sops, 1)) == -1) printf("%s", "'P' error\n");
}


v(int s, int semID) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  if((semop(semID, &sops, 1)) == -1) printf("%s", "'V' error\n");
}
