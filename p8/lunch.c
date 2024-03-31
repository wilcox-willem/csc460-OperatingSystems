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
  int *shmem_states;
  int N = 5;                  // Holds the number of procs/sems to generate
  int myID = 0;               // used to identify procs in sync
  int state[N];               // represents each philosphers state



  /******************** PREP BEFORE STARTING *********************/

  /*****  Make a note of "who" is the first Process *****/
  int firstID = getpid();

  // initialize shmem
  shmemID_states =  shmget(IPC_PRIVATE, (N+1)*sizeof(int), 0770);

  if (shmemID_states != -1) {
    shmem_states = (int *) shmat(shmemID_states, NULL, SHM_RND);
  } else {
    printf("shmem error\n");
    return (1);
  }
  
  /***** Get Semaphores *****/
  semID = semget (IPC_PRIVATE, N+1, 0777); // +1 for mutex sem

  // check if sem array made
  if (semID == -1) {
    printf("%s","SemGet Failed.\n");
    return (1);
  }


  // initialize sems/arrays (initial val 1 = ready)
  for (i = 0; i < N; i++) {
    semctl(semID, i, SETVAL, 1);
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
    myID = -1;
  }

  /***** Seed rand w/ ID *****/
  srand(myID);
	
  /******************** STARTING MAIN BODY *********************/

  if (getpid() != firstID) {
    /***** The Philosophers Loop *****/
    
    int theTime = shmem_states[N];

    while (theTime <= 60) {
    // as all great philosophers must do, ~~ THINK! ~~
      think();

    // as even greater philosophers must do, ~~ HUNGER! ~~
      pick_up_chopsticks(myID, semID, shmem_states);

    // as the greatest philosophers must do, ~~ EAT! ~~
      eat();
      put_down_chopsticks(myID, semID, shmem_states);
      
    // update time 
      theTime = shmem_states[N];
    }

    // now that main loop is over, die!
    shmem_states[myID] = DIE;
    return (0);

  } else {
    /****** the management loop ******/

    int loopVal = 0;
    int timePassed = 0;
    char printStr[50] = " "; 

    for (timePassed = 0; timePassed <= 60; timePassed++) {
      shmem_states[N] = timePassed;
      
      printf("%d. ", timePassed);
      for (loopVal = 0; loopVal < N; loopVal++){
        if (shmem_states[loopVal] == THINKING) printf("thinking  ");
        else if (shmem_states[loopVal] == HUNGRY) printf("hungry    ");
        else if (shmem_states[loopVal] == EATING) printf("eating    ");
        else if (shmem_states[loopVal] == DIE) printf("dead      ");
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

    while (cleanUpReady == 0 && timeWaited < 20) {
      cleanUpReady = 1;
      
      for (i = 0; i < N; i++) {
        if (shmem_states[i] != DIE) {
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

    if (shmdt(shmem_states) == -1) 
      printf("shmgm: ERROR in detaching.\n");

    if ((shmctl(shmemID_states, IPC_RMID, NULL)) == -1)
      printf("ERROR removing shmem.\n");

    printf("Done!");
  }

  return(0);
}

/***** Funcs for C's Lvly *****/

think() {
  // sleep 4-10 sec

  int sleepTimer = (rand() % 7) + 4;

  while (sleepTimer > 0) {
    sleep(1);
    sleepTimer -= 1;
  }
}

eat() {
  // sleep 1-3 sec
  
  int sleepTimer = (rand() % 3) + 1;

  while (sleepTimer > 0) {
    sleep(1);
    sleepTimer -= 1;
  }
}

test(int myID, int semID, int shmem_states[]) {
  int spotLeft = (myID + 4) % 5;
  int spotRight = (myID + 1) % 5;

  if (shmem_states[myID] == HUNGRY && 
      shmem_states[spotLeft] != EATING && 
      shmem_states[spotRight] != EATING) {
    shmem_states[myID] = EATING;
    v(myID, semID);
  }

}

pick_up_chopsticks(int myID, int semID, int shmem_states[]) {
  p(MUTEX, semID);                  // enter crit sect
  shmem_states[myID] = HUNGRY;      // update state
  test(myID, semID, shmem_states);  // try to get chops
  v(MUTEX, semID);                  // exit crit sect
  p(myID, semID);                   // block if chops not acquired
}



put_down_chopsticks(int myID, int semID, int shmem_states[]) {
  int spotLeft = (myID + 4) % 5;
  int spotRight = (myID + 1) % 5;

  p(MUTEX, semID);                    // enter crit sect
  shmem_states[myID] = THINKING;      // update state
  test(spotLeft, semID, shmem_states);  // check if neigbor can eat
  test(spotRight, semID, shmem_states); // check if other neigbor can eat
  v(MUTEX, semID);                    // exit crit sect
  p(myID, semID);                     // block if chops not acquired
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
