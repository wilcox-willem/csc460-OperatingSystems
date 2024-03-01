// Willem Wilcox
// CSC 460, Dr. Allen
// 2/24/24
// grocery: addFood.c
//  - started with a copy of "viewList.c" from this folder

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <string.h>

// master struct 
struct bbStruct {
        int id;
        char name[20];
        int favNum;
        char favFood[30];
};

main(int argc, char *argv[]) {
    int shmid,i;
    struct bbStruct *shmem;
    FILE *fopen(), *fp;

    int listPosition = 21; // WW: my pos in class list - 1
    char itemToUpdate[] = "Food Item";
    char newItem[30];

    /***** Update newItem with cl arg ******/
    if (argc != 2) {
	printf("\nError: use one string arg\n");
	return(1);
    }

    strcpy(newItem,argv[1]);

    /***** Check if valid user arg *****/
    if (strlen(newItem) > 30) {	
	printf("\nError: max string length is 30\n");
        return(1);
    }


    /*****  Open File to hold Shared Memory ID  *****/
    if((fp = fopen("./BBID.txt","r")) == NULL) {
        printf("viewList:  could not open file.\n");
        return(0);
    }


    /*****  Get Shared Memory ID and close file  *****/
    fscanf(fp,"%d",&shmid);
    fclose(fp);


    /****   Attach to the shared memory  ******/
    shmem = (struct bbStruct *) shmat(shmid, NULL, SHM_RND);


    /***** Adjust position of shmem to student position  *****/
    shmem = shmem + listPosition;

    /***** Update current item with newItem ******/
    // in this case: favFood
    strcpy(shmem->favFood,newItem);
   
    /***** Show updates  *****/ 
    printf("\n\nUpdating current \"%s\" with: \"%s\"\n\n",itemToUpdate,newItem);
    printf("%2d: %20s| %8d| %30s|\n",shmem->id,shmem->name,shmem->favNum,shmem->favFood);

    return(0);
}
