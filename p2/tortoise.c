// Will Wilcox
// CSC 460
// 2/12/24
// Tortoise Synch
// I set a new PR for Cobra/Assemble fork bombs :)
// I also made a bash script "OrphanObliterator" to cull orphans :)

#include <stdio.h>
#include <stdlib.h>

main(int argc, char *argv[]) {
FILE *fopen(), *fp;

// check command arg, store total for later
int userArgNumKids = 0;
int totalKids = userArgNumKids;
if (argc == 2) {
    userArgNumKids = atoi(argv[1]);
    if (userArgNumKids < 1 || userArgNumKids > 26) {
        printf(":( that's no good, try 1-26.\n");
        return(0);
    }
    userArgNumKids -= 1;
    totalKids = userArgNumKids;
    
} else {
    printf(":( that's no good, try arg 1-26.\n");
    return(0);
}



/*****  Open File to write a value  *****/
if((fp = fopen("syncfile","w")) == NULL) {
    printf(":( could not open syncfile to write.\n");
    return(0);
}



/*****  Write into syncfile ID of whose turn.  *****/
fprintf(fp,"%d",0);
fclose(fp);



/*****  Create Child process/es and give unique myID *****/
/*****  AKA Begin operation "Tom Forkery: Baby Boogaloo" *****/
// 1) Get og Process, set vars
int ogID = getpid();
int newID = getpid();
int position = 1;
char posChar = 'A';

// 2) Start the forkery
int keepMakingKids = 1;
while (keepMakingKids == 1 && fork() == 0) {
    userArgNumKids = userArgNumKids - 1;
    position = position + 1;

    if (userArgNumKids < 1){
        keepMakingKids = 0;
        position = 0;
    }
}

// 3) Adjust vars
if (ogID - newID == 0) {
    newID = getpid();
    posChar = 'A' + position;
}

/*****  ogID writes to file, kids wait *****/
int  i = 0;
int  found;
for (i=0; i<3; i++) {    
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
     
    // my turn
    printf("%c:%d\n",posChar,newID);
 
    // update file to allow otherID to go 
    if((fp = fopen("syncfile","w")) == NULL) {
        printf(":( could not open syncfile to write.\n");
        return(0);
    }
    
    int nextProc = position + 1;
    if (position >= totalKids) {
       nextProc = 0;
    }

    fprintf(fp,"%d",(nextProc));
    fclose(fp);
}


    return(0);
}

