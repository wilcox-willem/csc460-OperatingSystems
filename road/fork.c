// WILLEM WILCOX
// CSC 460
// 1/29/24
// Hey, you got to witness the birth of this file!

#include <stdio.h>
#include <stdlib.h>

// displays current gen, pid, and ppid
void getProcessInfo(int gen) {
	int thisGen,thisPID,thisPPID;

	thisGen = gen;
	thisPID = getpid();
	thisPPID = getppid();

	printf("%-6d %-6d %-6d\n",thisGen,thisPID,thisPPID); 	
}

// powerhouse of this program, recursively does forking
void getForked(int N, int current) {
	// N is the given total of generations
	// current is the current generation of processes
    	
	// base case, current > N
	if (current <= N) {
		int forkStatus = 0;
		int i = current;
		
		// loop with if statement to only allow
		// parent to make kids
		for (i; i <= N; i++) {
			if (forkStatus == 0) forkStatus = fork();
		}	
			
		// if not parent, wait and recurse
		if (forkStatus != 0) {
			getProcessInfo(current);
			sleep(2);
			current += 1;
			getForked(N, current);
		}
	}
}


int main(int argc, char * argv[]) {
	int N,myGen,myPID,myPPID;

	// get arguments from user
	// if args arent 0-5, exit 
	if (argc == 2) {		
		N = atoi(argv[1]);
		if (!(N >= 0) || !(N <= 5)) {
			printf("Please try with an arg 0-5\n");
			exit(1);
		}
	} else {
		printf("Please call with an argument that is 0-5\n");
		exit(1);
	}
	
	// print headings and og/parent process
	printf("Gen    PID    PPID  \n"); 
	getProcessInfo(0);

	// begin forking
	getForked(N, 1);

	return 0;
}

