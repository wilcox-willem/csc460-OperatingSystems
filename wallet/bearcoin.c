/:/ Willem Wilcox
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

main(int argc, char *argv[]) {

  int i, semID, shmemID, firstID, *shmem;
  int simCount;          // Holds the number of sims to run
  int myID = 0;          // used to identify procs & buyer or seller
                         // (0-15 Buy, 16-31 Sell)

  // Make a note of "who" is the first Process
  firstID = getpid();

  // Check if shared memory exists already
  shmemID = getShmemID();

  /*****  Get the command line argument ****/
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
      if (shmemID == -1){
      // if not setup, init memory for crypto club
        shmemID =  shmget(IPC_PRIVATE, sizeof(int), 0770);
        if (shmemID != -1) {
          shmem = (int *) shmat(shmid, NULL, SHM_RND);
        }
        else {
          printf("Unable to get shared memory\n")
          return(1);
        }

        semID = initWalletSema();
        storeCryptoData(shmemID, semID);
      }

      // if setup, print current wallet
      else {
        printWallet(shmemID);
      }
  }

////////
  // get ids

  // run sims

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
int getShmemID() {
    FILE *fopen(), *fp; 

    int shmemID = -1;
    int semID = -1;
    
    if ((fp = fopen("./cryptodata","r")) != NULL) {
      fscanf(fp,"%d\n%d", &shmemID, $semID);
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
      fscanf(fp,"%d\n%d", &shmemID, $semID);
      fclose(fp);
    }

    return semID;
}

storeCryptoData(int shmemID, int semID) {
  FILE *fopen(), *fp;

  if ((fp = fopen("./cryptodata","w")) != NULL) {
    fprintf(fp, "%d\n%d\n", shmemID, semID);
    fclose(fp);
  } 
}

printWallet(int shmemID) {
  printf("Coins currently in Wallet: %d",&shmemID);
}


// initialize wallet gate keeper, return sem address
int initWalletSema() {
  /*****  Ask OS for Sems *****/
  int semID = semget (IPC_PRIVATE, N, 0777);

  /*****  See if you got the request *****/
  if (semID == -1) {
    printf("%s","SemGet Failed.\n");
    return (-1);
  }

  /*****  Initialize Bank Sem *****/
  semctl(semID, 0, SETVAL, 1);

  return semID;
} 
