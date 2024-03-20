/:/ Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/19/24
// p6: bearcoin!

// Funny story, at Geek Squad more of my coworkers have quit
// Us remaining few are slowly but surely stepping out the door too
// ...
// I still cant wait to get a better job

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

main(int argc, char *argv[]) {
int i, semID, shmemID, firstID;
int simCount;          // Holds the number of sims to run
int myID = 0;          // used to identify procs & buyer or seller
bool myRole = 0;        // (0-15 Buy, 16-31 Sell)
int LoopCount = 3;     // Times each proc will "speak"
bool isActive = false; // Check if currently running or not

/*****  Make a note of "who" is the first Process *****/
firstID = getpid();

/***** Check if shared memory exists already ******/
shmemID = getShmemID();


/*****  Get the command line argument, do setup if needed ****/

// int arg call
if (argc ==  2 && argv[1] != "cleanup") {
  simCount = atoi(argv[1]); 

  // check simCount is in bounds 1-100
  if (simCount > 100 || simCount < 1) {
    printf(":( that's no good, try 1-100.\n");
    return(0);
  }

// cleanup arg call
} else if (argc == 2 && argv[1] == "cleanup" && shmemID != -1) {
    run_cleanup(); // implement function for cleanup

// no args call
} else if (argc == 1){
    // check if existing memory already
    if (shmemID != -1){
    // if not, setup memory for crypto club
    // and create "cryptodata" file to store data
        initWallet();
    }
    // if setup, print current wallet
    else {
	printWallet(shmemID);
    }

}



/*****  Ask OS for Sems *****/
semID = semget (IPC_PRIVATE, N, 0777);

/*****  See if you got the request *****/
if (semID == -1) {
  printf("%s","SemGet Failed.\n");
  return (-1);
}

/*****  Initialize Bank Sem *****/
semctl(semID, 0, SETVAL, 1);



/*************  Spawn all the Processes *********/
for (i = 1; i < 32; i++) {
  if (fork() > 0) break; // send parent on to Body
  myID++;
  myRole++;
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
k
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


/***** Crypto Functions *****/

// returns -1 if unable to open file
getShmemID() {
    FILE *fopen(), *fp; 

    int shmemID = -1;
    
    if ((fp = fopen("./cryptodata","r")) != NULL) {
	fscanf(fp,"%d",&shmemID);
	fclose(fp);
    }

    return shmemID;
}


// initialize wallet w/ shared mem
initWallet(shmemID) {

    /*****  Ask OS for Sems *****/
    int shmemID = semget (IPC_PRIVATE, N, 0777);

    /*****  See if you got the request *****/
    if (shmemID == -1) {
      printf("%s","SemGet Failed.\n");
      return (-1);
    }

    /*****  Initialize Bank Sem *****/
    semctl(shmemID, 0, SETVAL, 1);

} 
