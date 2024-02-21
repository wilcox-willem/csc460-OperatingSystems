#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define TURN shmem[0]

main(int argc, char *argv[]) {
int i, shmid, *shmem;
int N;                  // Holds the number of procs to generate
int myID = 0;           // used to identify procs in sync
int LoopCount = 3;     // Times each proc will "speak"

/*****  Get the command line argument for how many procs. Default of 5 ****/
if (argc < 2)
  N = 5;
else if (argc == 2) {
  N = atoi(argv[1]); 

  // check N is in bounds
  if (N > 26 || N < 1) {
    printf(":( that's no good, try 1-26.\n");
    return(0);
  }
}

/*****  Make a note of "who" is the first Process *****/
int firstID = getpid();

/*****  Get Shared Memory and ID *****/
shmid  =  shmget(IPC_PRIVATE, sizeof(int), 0770);
if (shmid == -1) {
  printf("Could not get shared memory.\n");
  return(0);
}


/****   Attach to the shared memory  ******/
shmem = (int *) shmat(shmid, NULL, SHM_RND);


/****  Initialize the shared memory ****/
TURN = 0;


/*************  Spawn all the Processes *********/
for (i = 1; i < N; i++) {
  if (fork() > 0) break; // send parent on to Body
  myID++;
}
printf("I'm Alive: %d - %d\n",getpid(),myID);
sleep(1);


/*************  BODY  OF  PROGRAM     ***********/
for (i = 0; i < LoopCount; i++) {
  while(TURN != myID);  /** busy wait for Child **/

  char myChar = 'A' + myID - 1;

  printf("%c:%d\n"myChar,myID);
  TURN = (TURN + 1) % N;
}


/****   Detach and clean-up the shared memory  ******/
if (shmdt(shmem) == -1 ) printf("shmgm: ERROR in detaching.\n");

sleep(1);

if (firstID == getpid()) // ONLY need one process to do this
if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
  printf("ERROR removing shmem.\n");

printf("alldone\n");
}

