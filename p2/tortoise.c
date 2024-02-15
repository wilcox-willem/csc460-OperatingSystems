// Willem Wilcox
// CSC 480
// P2: Tortoise Sync
// Dr Allen, MU
// 2/11/24

#include <stdio.h>
#include <stdlib.h>



main(int argc, char *argv[]) {

FILE *fopen(), *fp;

// check command arg
int howManyKidsWeHavin = 0;
if (argc == 2) {
    howManyKidsWeHavin = atoi(argv[1]);
    if (howManyKidsWeHavin < 1 || howManyKidsWeHavin > 26) {
        printf(":( that's no good, try 1-26.\n");
        return(0);
    }
    howManyKidsWeHavin -= 1;
    
} else {
    printf(":( that's no good, try arg 1-26.\n");
    return(0);
}

/*****  Open File to write a value  *****/

if((fp = fopen("syncfile","w")) == NULL)
    {
    printf(":( could not open myLittleFile to write.\n");
    return(0);
    }

/*****  Write into syncfile ID of whose turn.  *****/

fprintf(fp,"%d",getpid());
printf("I am - %d\n", fp);
fclose(fp);


/*****  Create Child process/es and give unique myID *****/
/*****  AKA Begin operation "Tom Forkery: Baby Boogaloo" *****/
// 1) Get og Process, set vars
int ogID = getpid();

int newID = getpid();
int position = 0;
char posChar = 'A';
//printf("%d: %d\n",ogID,newID);

// 2) Start the forkery
int keepMakingKids = 1;
int totalKids = howManyKidsWeHavin;

int loopIter = 0;
while (keepMakingKids == 1 && fork() == 0) {
    loopIter = loopIter + 1;
    howManyKidsWeHavin = howManyKidsWeHavin - 1;

    if (howManyKidsWeHavin < 1){
        keepMakingKids = 0;
    }
}

// 3) if baby process, adjust vars
if (ogID - newID == 0) {
    newID = getpid();
    position = getpid() - ogID;
    if (position > 25) {
        position -= 25;
    }
    posChar = 'A' + position;
    //printf("%c:%d\n",posChar,newID);
}

/*****  ogID writes to file, kids wait *****/

int  i = 0;
int  found;

for (i=0; i<3; i++) {
    printf("%d %d %c\n",i,getpid(),posChar);
    
    /***** Repeatedly check file until myID is found *****/
    found = -1;
    while (found != position) {
        if((fp = fopen("syncfile","r")) == NULL) {
            printf(":( could not open syncfile to read.\n");
            return(0);
        }

        fscanf(fp,"%d",&found);
        fclose(fp);
    }
     
    // It must be my turn to do something.....
    printf("%c:%d\n",posChar,newID);
 
    // Update file to allow otherID to go 

    if((fp = fopen("syncfile","w")) == NULL) {
        printf(":( could not open myLittleFile to write.\n");
        return(0);
    }
     
    int nextProc = position + 1;
    if (position > totalKids) {
       nextProc = 0;
    }

    fprintf(fp,"%d",(nextProc));
    fclose(fp);

}

    return(0);
}

