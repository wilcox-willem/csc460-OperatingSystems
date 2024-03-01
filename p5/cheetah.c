// Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/1/24
// p5: cheetah sync!

// Funny story, at Geek Squad most of coworkers have quit
// so I am now 1/3 of the entirety of Warner Robins Geek Squad
// ...
// I cant wait to get a better job

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

main(int argc, char *argv[]) {
int i, sem_id;
int N;                  // Holds the number of procs to generate
int myID = 0;           // used to identify procs in sync
int LoopCount = 3;     // Times each proc will "speak"

/*****  Get the command line argument for how many procs. Default of 5 ****/
if (argc < 2) {
  N = 5;
} else if (argc == 2) {
  N = atoi(argv[1]); 

  // check N is in bounds
  if (N > 26 || N < 1) {
    printf(":( that's no good, try 1-26.\n");
    return(0);
  }
}

/*****  Make a note of "who" is the first Process *****/
int firstID = getpid();

/*****  Ask OS for Sems *****/
sem_id = semget (IPC_PRIVATE, N, 0777);


/*****  See if you got the request *****/
if (sem_id == -1) {
  printf("%s","SemGet Failed.\n");
  return (-1);
}

/*****  Initialize your sems *****/
semctl(sem_id, 0, SETVAL, 1);
for (i = 1; i < N; i++) {
  semctl(sem_id, i, SETVAL, 0);
}


/*************  Spawn all the Processes *********/
for (i = 1; i < N; i++) {
  if (fork() > 0) break; // send parent on to Body
  myID++;
}

sleep(1);


/*************  BODY  OF  PROGRAM     ***********/
for (i = 0; i < LoopCount; i++) {
  p(myID, sem_id); // wait for turn

  // set char for printing
  char myChar = 'A' + myID;

  printf("%c:%d\n",myChar,getpid());
  v((myID + 1) % N, sem_id); // notify next turn
}

sleep(1);

if (firstID == getpid()) { // ONLY need original process to do this
  // clean up
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
  if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}

v(int s, int sem_id) {
  struct sembuf sops;

  sops.sem_num = s;
  sops.sem_op = 1;
  sops.sem_flg = 0;
  if((semop(sem_id, &sops, 1)) == -1) printf("%s","'V' error\n");
}
