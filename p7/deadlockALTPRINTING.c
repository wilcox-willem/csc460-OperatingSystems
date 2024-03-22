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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

main(int argc, char *argv[]) {
  int i, sem_id;
  int N = 5;                  // Holds the number of procs/sems to generate
  int myID = 0;               // used to identify procs in sync
  int semArray[N];          // used to hold sem's for chopsticks
  char tabsNeeded[5] = "-";  // used to hold tabs for each proc
  char tabSpare[1] = "-";    // spare tab for my sake


  /******************** PREP BEFORE STARTING *********************/

  /*****  Make a note of "who" is the first Process *****/
  int firstID = getpid();

  /***** Seed rand w/ time *****/
  srand(time(0));	
  
  /***** Get Semaphores *****/
  for (i = 0; i < N; i++) {
    // ask os for sems
    sem_id = semget(IPC_PRIVATE, 1, 0777);

    // check if sems made
    if (sem_id == -1) {
      printf("%s","SemGet Failed.\n");
      //return (-1);
    }

    semArray[i] = sem_id;
  }
  

  


  // initialize sems (initial val 1 = ready)
  semArray[0] = sem_id;
  for (i = 0; i < N; i++) {
    semctl(sem_id, i, SETVAL, 1);
    semArray[i+1] = i;
  }
 

  /*****  Spawn all the Processes *****/
  for (i = 1; i < N; i++) {
    if (fork() > 0) break; // send parent on to Body
    myID++;
    strcat(tabsNeeded, tabSpare); // adds another tab
  }

  // stagger start times
  think_super_hard();

  /******************** STARTING MAIN BODY *********************/

  while (1) {
  // as all gkreat philosophers must do, THINK!
    think_super_hard(); // waste time
    printf("%s%d THINKING\n", tabsNeeded, myID);

  // as even greater philosophers must do, HUNGER!
    printf("%s%d HUNGRY\n", tabsNeeded, myID);
    // try to pick up Left chopstick
    int stickLeft = pick_up_chopstick(0, myID, semArray);
    think_super_hard(); // waste time
    
    // try to pick up Right chopstick
    int stickRight = pick_up_chopstick(1, myID, semArray);

  // as the greatest philosophers must do, EAT!
    printf("%s%d EATING\n", tabsNeeded, myID);
    // put down Left
    think_super_hard(); //waste time
    put_down_chopstick(0, myID, semArray);
    // put down Right
    think_super_hard(); //waste time
    put_down_chopstick(1, myID, semArray);
     
  }


  /******************** ENDING MAIN BODY *********************/  

  // clean up, ONLY need original process to do this
  if (firstID == getpid()) {
    sleep(2);
    if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1) {
      printf("%s", "Parent: ERROR in removing sem\n");
    }
  }

  return(0);
}


/***** Semaphore Functions *****/
p(int s,int sem_id) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = -1;
  sops.sem_flg = 0;
  semop(sem_id, &sops, 1);
}

v(int s, int sem_id) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  semop(sem_id, &sops, 1);
}

/***** Funcs for Bob's Killer Strats *****/
think_super_hard(){
  // to waste time, I have opted for the following strat
  // use rand int to generate massive num, then triple for loop
  // of adding a num for no reason

  int howLong = rand() % 90000000 + 1000;
  int i,j,k, dummy;

  for (i = 0; i < howLong; i++) {
    for (j = 0; i < howLong; i++) {
      for (k = 0; i < howLong; i++) {
	      dummy = dummy + 1;
      }
    }
  }
	
}




pick_up_chopstick(int side, int myID, int semArray[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // check if safe
  p(0, semArray[spotToCheck]);
  // ENTERING CRITICAL ZONE
  return (1);
}

put_down_chopstick(int side, int myID, int semArray[]){
  // side: (0 = L, 1 = R)
  // myID: (0-4)
  // semArray[5]: holds each chopstick sem

  int spotToCheck = (myID + side) % 5;

  // EXITING CRITICAL ZONE
  // signal and leave
  v(0, semArray[spotToCheck]);
  return (1);
}
