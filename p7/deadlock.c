// Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/1/24
// p7: Bob's Deadly Diner

// I'm not a huge anime watcher, BUT recently I have been rewatching
// 'Ghost in the Shell' and 'Akira'. I love the art style, and I think
// the concepts are both a little cliche but still relevant and
// important to ponder and analyze. Have you seen either, if not
// would you watch either movie?

// ALSO, anytime I watch them, I much prefer the originals over
// any remakes or remasters. Especially with Ghost in the Shell
// There's 2 remakes, one just worsens visuals and effects, the other
// is the scarlet johansen version, which isn't bad but it glosses
// over a lot of the little details that make the original so special.

// Anywho, I really enjoy watching them over n over every now and then.
// I am curious on your thoughts if you have seen them!

// ANYWHO, enjoy my assignment! I had a lot of fun with this one
// [I mean cmon, trying to break stuff is always fun :) ]

// Have a wonderful morning/day/evening! I hope you find a new reason to smile today!

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


main(int argc, char *argv[]) {
  int i, semID, shmemID;
  int *shmemArray;
  int N = 5;                  // Holds the number of procs/sems to generate
  int myID = 0;               // used to identify procs in sync
  int mySticks[2] = {0,0};    // used to track sticks held (0 = L, 1 = R)
  int semArray[N];            // used to hold sem's for chopstick access
  char tabsNeeded[5] = "\t";  // used to hold tabs for each proc
  char tabSpare[1] = "\t";    // spare tab for my sake
  //int state[N];               // represents each philosphers state



  /******************** PREP BEFORE STARTING *********************/

  /*****  Make a note of "who" is the first Process *****/
  int firstID = getpid();

  // initialize shmem
  shmemID =  shmget(IPC_PRIVATE, 5*sizeof(int), 0770);
  if (shmemID != -1) {
    shmemArray = (int *) shmat(shmemID, NULL, SHM_RND);
  }
  else {
    printf("shmem error\n");
    return (1);
  }


  
  /***** Get Semaphores *****/
  for (i = 0; i < N; i++) {
    // ask os for sems
    //if (i == 0) {
	semID = semget (IPC_PRIVATE, 1, 0777);
    //}
    // check if sems made
    if (semID == -1) {
      printf("%s","SemGet Failed.\n");
      return (-1);
    }

    semArray[i] = semID;
    shmemArray[i] = 1;
  }
  
  // initialize shmem
  shmemID =  shmget(IPC_PRIVATE, 5*sizeof(int), 0770);
  if (shmemID != -1) {
    shmemArray = (int *) shmat(shmemID, NULL, SHM_RND);
  }
  else {
    printf("shmem error\n");
    return (1);
  }



  // initialize sems/arrays (initial val 1 = ready)
  for (i = 0; i < N; i++) {
    semctl(semID, i, SETVAL, 1);
    semArray[i] = i;
    shmemArray[i] = 1;
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
    think_super_hard(); // waste time
    if (mySticks[0] == 0 && mySticks[1] == 0) {
      printf("%s%d THINKING\n", tabsNeeded, myID);
    }
  // as even greater philosophers must do, HUNGER!
    if (mySticks[0] == 1 || mySticks[1] == 1) {
      printf("%s%d HUNGRY\n", tabsNeeded, myID);
    }    

    // try to pick up Left chopstick
    pick_up_chopstick(0, myID, semArray, shmemArray, mySticks);
    think_super_hard(); // waste time
    
    // try to pick up Right chopstick
    pick_up_chopstick(1, myID, semArray, shmemArray, mySticks);

  // as the greatest philosophers must do, EAT!
    if (mySticks[0] == 1 && mySticks[1] == 1) {
      printf("%s%d EATING\n", tabsNeeded, myID);
    }

    // put down Left
    think_super_hard(); //waste time
    put_down_chopstick(0, myID, semArray, shmemArray, mySticks);

    // put down Right
    think_super_hard(); //waste time
    put_down_chopstick(1, myID, semArray, shmemArray, mySticks);
     
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


/***** Semaphore Functions *****/
p(int s,int semID) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  semop(semID, &sops, 1);
}

v(int s, int semID) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  semop(semID, &sops, 1);
}

/***** Funcs for Bob's Killer Strats *****/
think_super_hard(){
  // to waste time, I have opted for the following strat
  // use rand int to generate massive num, then triple for loop
  // of adding a num for no reason


  int howLong1 = rand() % 90000000 + 1000;
  int howLong2 = rand() % 90000000 + 1000;
  int howLong3 = rand() % 90000000 + 1000;
  int i, j, k, dummy;

  for (i = 0; i < howLong1; i++) {
    for (j = 0; i < howLong2; i++) {
      for (k = 0; i < howLong3; i++) {
        dummy = dummy + 1;
      }
    }
  }

}

pick_up_chopstick(int side, int myID, int semArray[], int shmemArray[], int mySticks[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // check if safe
  p(0, semArray[spotToCheck]);
  // ENTERING CRITICAL ZONE

  if (mySticks[side] == 0 && shmemArray[spotToCheck] == 1) {
    shmemArray[spotToCheck] -= 1;
    mySticks[side] += 1;
  }

  // EXITING CRITICAL ZONE
  // signal and leave
  v(0, semArray[spotToCheck]);
}

put_down_chopstick(int side, int myID, int semArray[], int shmemArray[], int mySticks[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // check if safe
  p(0, semArray[spotToCheck]);
  // ENTERING CRITICAL ZONE

  if (mySticks[side] == 1 && shmemArray[spotToCheck] == 0) {
    shmemArray[spotToCheck] += 1;
    mySticks[side] -= 1;
  }

  // EXITING CRITICAL ZONE
  // signal and leave
  v(0, semArray[spotToCheck]);
}
