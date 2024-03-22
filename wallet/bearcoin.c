// Willem Wilcox
// CSC 460, Dr. Allen, MU
// 3/19/24
// p6: bearcoin!
// This one was interesting, lots of things to wrap my brain around.
// I am slowly but surely becoming a VIM warrior too.
// However, I still love using a clean-ish vscode gui

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

main(int argc, char *argv[]) {

  int i, semID, shmemID, firstID, *shmem;
  int simCount;          // Holds the number of sims to run
  int myChange = 1;      // Holds this procs change in val
  int myID = 0;          // used to identify vals of buyer or seller
                         // (0-15 Buy, 16-31 Sell)
  int runSim = 0;   // used to keep sim section out of arg checker
  int temp_balance, old_balance, new_balance;

  // Make a note of "who" is the first Process
  firstID = getpid();

  // Check if shared memory exists already
  shmemID = getShmemID();

  /*****  Get the command line argument ****/
  // int arg call
  if (argc ==  2 && strcmp(argv[1], "cleanup") != 0) {
    simCount = atoi(argv[1]); 

    // check simCount is in bounds 1-100
    if (simCount > 100 || simCount < 1) {
      printf(":( that's no good, try 1-100.\n");
      return (0);
    }

    if (getShmemID() == -1) {
      printf("error, system not initialized. Run without args first\n");
      return (1);
    }

    runSim = 1; // sim logic after arg checker
  }
  // cleanup arg call 
  else if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
      run_cleanup(); // implemented cleanup
  } 
  // no args call
  else if (argc == 1){
      // nor args: not initialized
      if (shmemID == -1) {
        shmemID =  shmget(IPC_PRIVATE, sizeof(int), 0770);
        if (shmemID != -1) {
          shmem = (int *) shmat(shmemID, NULL, SHM_RND);
          *shmem = 460;
        }
        else {
          printf("Unable to get shared memory\n");
          return (1);
        }

        /*****  Ask OS for Sems *****/
        semID = semget(IPC_PRIVATE, 1, 0777);

        /*****  See if you got the request *****/
        if (semID == -1) {
          printf("%s","SemGet Failed.\n");
          return (1);
        }

        /*****  Initialize Bank Sem *****/
        semctl(semID, 0, SETVAL, 1);

        /***** Store ID's in "cryptodata" *****/
        storeCryptoData(shmemID, semID);
      }
      // if setup, print current wallet
      else {
	      shmemID = getShmemID();
	      shmem = (int *) shmat(shmemID, NULL, SHM_RND); 
        printWallet(shmem);
      }
  }


  // Run the sim if passed in args
  if (runSim == 1) {
    // prep ID's
    shmemID = getShmemID();
    semID = getSemID();
    shmem = (int *) shmat(shmemID, NULL, SHM_RND); 

    // start forking
    /*********  Spawn all the Parents *********/
    for (i = 1; i < 16; i++) {
      if (fork() > 0) break; // send parent on to Body
      myID++;
    }

    // set parent starting money
    for (i = 0; i < myID; i++){
      myChange = myChange * 2;
    }

    /*********  Spawn all the Kids *********/
    if (myID >= 0 && myID <= 15) {
      if (fork() == 0) {
        myID += 16;
        myChange = myChange * -1; // -1 since takin money
      }
    }

    // core loop of sim
    for (i = 0; i < simCount; i++) {
      //printf("- %d - %d and $%d\n",semID,  myID, myChange);

      // wait for safe usage     
      p(0, semID);

      // WARNING: ENTERING THE CRITICAL ZONE
      temp_balance = *shmem; // get current balance
      old_balance = temp_balance;
      new_balance = temp_balance + myChange;

      *shmem = new_balance;

      if (myID < 16) {
        printf("%d: %d + %d = %d\n", myID, old_balance, myChange, new_balance);
      }
      else {
        printf("\t\t%d: %d %d = %d\n", myID, old_balance, myChange, new_balance);
      }  

	//printWallet(shmem);
	
      // WARNING: LEAVING THE CRITICAL ZONE
      // signal end of usage     
      v(0, semID);

    }
  }

  //printf("Done!\n");
  return (0);
}


/***** Semaphore Functions *****/
p(int s,int sem_id) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = -1;
  sops.sem_flg = 0;

  semop(sem_id,&sops, 1);
  //if(semop(sem_id, &sops, 1) == -1){	
  //  printf("P error\n");
  //} 
}

v(int s, int sem_id) {
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_op = 1;
  sops.sem_flg = 0;


  semop(sem_id,&sops, 1);
  //if(semop(sem_id, &sops, 1) == -1){	
  //  printf("P error\n");
  //}
}
    


/***** Crypto Functions *****/

// returns -1 if unable to open file
int getShmemID() {
    FILE *fopen(), *fp; 

    int shmemID = -1;
    int semID = -1;
    
    if ((fp = fopen("./cryptodata","r")) != NULL) {
      fscanf(fp,"%d\n%d", &shmemID, &semID);
      fclose(fp);
    }

    return shmemID;
}

// returns -1 if unable to open file
int getSemID() {
    FILE *fopen(), *fp; 

    int shmemID = -1;
    int semID = -1;
    
    if ((fp = fopen("./cryptodata","r")) != NULL) {
      fscanf(fp,"\n%d", &semID);
      fclose(fp);
    }
	

    //printf("SemID Found: %d",semID);
    return semID;
}

storeCryptoData(int shmemID, int semID) {
  FILE *fopen(), *fp;

  fp = fopen("./cryptodata","w");
  fprintf(fp, "%d\n%d\n", shmemID, semID);
  fclose(fp);
   
}

printWallet(int* shmem) {
  printf("Coins currently in Wallet: %d\n",*shmem);
}

run_cleanup() {
  FILE *fopen(), *fp; 
  int shmemID, semID;  

  // if init/file exists
  if ((fp = fopen("./cryptodata","r")) != NULL) {
    fclose(fp);
    semID = getSemID();
    shmemID = getShmemID();

    // remove shmem
    if ((shmctl(shmemID, IPC_RMID, NULL)) == -1)
    printf("ERROR removing shmem\n");

    // remove sem
    if ((semctl(semID, 0, IPC_RMID, 0)) == -1)
    printf("ERROR removing sem\n");

    
    system("rm ./cryptodata");
  }

}
