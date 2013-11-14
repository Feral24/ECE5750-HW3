MAIN_ENV

//start include "Board.h"
typedef struct
{
	int n;
	int **squares;
} Board;

/* Allocates 2-D array squares of Board b on memory.
   2-D array is allocated on the local memory, using PMAL.
   */
void initialize(Board *b, int n);

// creates board on the global memory
void initialize_g(Board *b, int n);

// Free up all the memory of the board created using initialize()
void destroy(Board *b);

// Same as initialize(), but allocates shared memory using GMAL.
void init_shared(Board *b, int n);

// Put a queen on squares[col][row] and invalidate conflicting squares
void putqueen(Board *b, int col, int row);

// Perform a deep copy of source Board
void copy_board(Board *dest, Board *src);

// Another version of copy_board that does not use memcpy()
void copy_board_s(Board *dest, Board *src);

// This traverses the board matrix and returns the total profit of the board
int calcprofit(Board *b);

// prints the board to standard output
void print_board(Board *b);
//end include "Board.h"


typedef struct {
	int n;
	int p;
	int nexttree;
	int maxprofit;
	int numsols;
	Board bestsol;
	LOCKDEC(treelock) // locks nexttree
	LOCKDEC(proflock) // locks maxprofit, numsols, and bestsol
} GM;

GM *gm;

void search(Board *b, int col); 

// This function will initialize and start the recursive search for solutions per processor
void process_tree(void) {
	int n = gm->n;
	int thistree;

	Board *b;

	while(1) {
		// Need to lock before read to prevent multiple processes from working on the same tree
		LOCK(gm->treelock)
		int nexttree = gm->nexttree;
		if(nexttree >= n) { // If no more trees left...
			UNLOCK(gm->treelock)
			break;
		}
		thistree = gm->nexttree;
		gm->nexttree++; // increment next tree to work on
		UNLOCK(gm->treelock)
		// Create new blank board 
		b = (Board *) P_MALLOC(sizeof(Board));
		initialize(b, n);
		// place an initial queen in the appropriate row that corresponds to the tree
		putqueen(b,0,thistree);

		// Start recursive function...
		search(b,1);
		// Deallocate finished tree
		destroy(b);
		free(b);
	}
}

// Increment the numsols counter, and also checks for the best solution. 
void addsol(Board *b) {
    // Calculate the profit of the board
	int newprofit = calcprofit(b);
	// First, increment the solution counter
	LOCK(gm->proflock);
	gm->numsols++;

	// Then check if this is a better solution
	int maxprofit = gm->maxprofit;
	if(newprofit > maxprofit) {
		gm->maxprofit = newprofit;
		//Update the global Best solution Board
		copy_board_s(&gm->bestsol, b);
	}
	UNLOCK(gm->proflock)
}

// This will recursively search for valid solutions by placing queens in valid positions in col c and invalidating affected cells correspondingly
void search(Board *b, int col) {

	int n = gm->n;
	register int foundvalid = 0;
	register int row;

	// Base case 1: if outside the last column, store board as solution and return
	if (col == n) {
		//store board as solution
		addsol(b);
		return;
	}

	// Recursive case: check all valid cells in col c, 
	// and place a queen in a new copy of the board, then go one level deeper
	for (row = 0; row < n; row++) {
		if (b->squares[col][row] == 0) {
			foundvalid = 1;
			Board *newboard;
			newboard = (Board *) P_MALLOC(sizeof(Board)); // mallocs a new board struct
			
			// will malloc the array inside the Board struct and also set the value of n inside
			initialize(newboard, n); 
			copy_board(newboard, b); // copy the board over to a newboard

			putqueen(newboard,col,row); // put a queen in the valid spot, and invalidate cells accordingly
			search(newboard, col+1); // go one level deeper in recursive search
			destroy(newboard); // deallocating dead node
			free(newboard);
		}
	}

	// Base case 2: If all cells in current col are invalid, then no solution exists
	if (foundvalid != 1) {
		return;
	}
}


int main(int argc, char **argv) {
	int n,p,i;
	unsigned int t1,t2,t3,t4;
	MAIN_INITENV
	if (argc!=3) {
		printf("Usage: nqueens N P\nAborting...\n");
		exit(0);
	}

	CLOCK(t1)
	// Initialize global memory
	gm = (GM *)G_MALLOC(sizeof(GM));
	gm->n = atoi(argv[1]);
	p = gm->p = atoi(argv[2]);
	assert(p > 0);
	assert(p <= 8);
	n = gm->n;
	gm->nexttree = 0;
	LOCKINIT(gm->treelock)
	LOCKINIT(gm->proflock)
	gm->maxprofit = 0;
	gm->numsols = 0;
	// Initialize the best solution board here
	initialize_g(&(gm->bestsol), n);

	// Initialize processes per processor
	for(i = 0; i < p-1; i++) {
		CREATE(process_tree)
	}

	printf("N= %u, P = %u\n",n,p);
	// Run and time the processes
	CLOCK(t2)
	printf("Initialization time: %u us\n",t2-t1);
	process_tree();
	WAIT_FOR_END(p-1)
	CLOCK(t3)

	printf("Calculation time: %u us\n",t3-t2);
	//Print results
	printf("Number of solutions found: %u\n",gm->numsols);
	printf("Maximum profit found: %u\n", gm->maxprofit);
	print_board(&gm->bestsol);	// Print best solution's board
	CLOCK(t4)
	printf("Finalization time: %u us\n",t4-t3); 
	printf("Total time: %u us\n",t4-t1); 
	MAIN_END
	return 0;

}

/************* INCLUDE Board.c **************/

// Allocates 2-D array squares of Board b on local memory
void initialize(Board *b, int n) {
	b->n = n;
	b->squares = (int**) P_MALLOC(n*sizeof(int*));
	int i;
	for (i = 0; i < n; i++) {
		b->squares[i] = (int*) P_MALLOC(n*sizeof(int));
		memset(b->squares[i],0,n*sizeof(int));
	}
}

// Same as above but in global memory
void initialize_g(Board *b, int n) {
	b->n = n;
	b->squares = (int**) G_MALLOC(n*sizeof(int*));
	int i, j;
	for (i = 0; i < n; i++) {
		b->squares[i] = (int*) G_MALLOC(n*sizeof(int));
		for (j = 0; j < n; j++) {
			b->squares[i][j] = 0;
		}
	}
}

// Deallocates memory for a board
void destroy(Board *b) {
	int i;
	for (i = 0; i < b->n; i++) {
		free(b->squares[i]);
	}
	free(b->squares);
}


// Put a queen on squares[col][row] and invalidate conflicting squares
// A cell is 1 if there is a queen there, -1 if it is invalid, and 0 if it is valid
void putqueen(Board *b, int col, int row) {
	int n = b->n;
	int i;
	for(i=0; i < n; i++) {
		b->squares[col][i] = -1;
	}
	for(i=col+1; i < n; i++) {
		b->squares[i][row] = -1;
	}
	int j;
	for(i=col+1, j=row+1; i<n && j<n; i++, j++) {
		b->squares[i][j] = -1;
	}
	for(i=col+1, j=row-1; i<n && j>=0; i++, j--) {
		b->squares[i][j] = -1;
	}
	b->squares[col][row] = 1;
}

// Perform a deep copy of source Board
void copy_board(Board *dest, Board *src) {
	dest->n = src->n;
	int i;
	for(i = 0; i < src->n; i++) {
		// assuming PMAL is malloc
		memcpy(dest->squares[i], src->squares[i], (src->n)*sizeof(int));
	}
}

// Performs the above without memcpy
void copy_board_s(Board *dest, Board *src) {
	dest->n = src->n;
	int i, j;
	for(i = 0; i < src->n; i++) {
		for(j = 0; j < src->n; j++) {
			dest->squares[i][j] = src->squares[i][j];
		}
	}
}

// This traverses the board matrix and returns the total profit of the board
int calcprofit(Board *b) {
	int totalprofit = 0;
	register int i,j;
	int n = gm->n;

	for (i = 0; i<n; i++) {
		for (j = 0; j<n; j++) {
			if (b->squares[i][j] == 1)
				totalprofit += abs(i-j);
		}
	}
	return totalprofit;
}

// Prints out the board of a solution
void print_board(Board *b) {
	printf("Best solution's board:\n");
	printf("--------\n");
	int i, j;
	for(i=0; i<b->n; i++) {
		for(j=0; j<b->n; j++) {
		    int val = (b->squares[i][j] + abs(b->squares[i][j]))/2; // Removes -1's
			printf("%d ", val);
		}
		printf("\n");
	}
	printf("--------\n");
}
/************* END Board.c **************/

