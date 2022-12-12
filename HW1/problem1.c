#include <stdio.h>     
#include <limits.h>    
#include <mpi.h>
int checkCircuit (int, int);

int tree_sum(int my_rank, int numprocs){ 
    int remain_procs = numprocs;      
    
 
    int solutions = 0;                    //the number of answer
    
    //every processor process the value 
    for (int i = my_rank; i <= USHRT_MAX; i+=numprocs ){                        
        int value = checkCircuit (my_rank, i);                          
        //if satisfy the bool operand then solutions plus 1
        if(value == 1){                                               
            solutions ++;
        }
    }
    /***************************tree structure *********************************
    
    when number of processor is odd, tree structure check by remainder.
    if the number of processor which is need to compute is odd in current level,
    then my_rank == middle processor do nothing in this level and wait to compute in next level.  
    
    In the last, 
    because ramain is 1 and break while loop and finialize the level processing int parallel programing.
    every processor will run this while loop and chech what to do by their rank.
    
    ****************************************************************************/
    while(!(remain_procs ==  1)) {
        int half = remain_procs/2;
        int rm   = remain_procs%2;
        if(my_rank < half) {
            int temp = 0;
            //receive local_sum compute by other processor, and sum the number of solutions
            MPI_Recv(&temp    , 1, MPI_INT, my_rank+half+rm, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
            solutions += temp;
        }
        else if(my_rank >= half+rm && my_rank < remain_procs) {
            //pass number of solutions to other processor, other processor will sum the number of solutions
            MPI_Send(&solutions, 1, MPI_INT, my_rank-half-rm, 0, MPI_COMM_WORLD);  
            break;
        }
        remain_procs = half+rm;
    }
    return solutions;
}
int main (int argc, char *argv[]) {
   int my_rank, numprocs,namelen;
   char processor_name[MPI_MAX_PROCESSOR_NAME];
   double startTime = 0.0, totalTime = 0.0;
   //MPI initialize
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
   MPI_Get_processor_name(processor_name,&namelen);
   
   if(my_rank == 0){                     //if my_rank==0 then parallel processing start
	      startTime = MPI_Wtime(); 
    }
    
   int solutions = tree_sum(my_rank, numprocs);
   
   if(my_rank == 0){                     //if my_rank==0 then print the result;
        totalTime = MPI_Wtime() - startTime;
        printf ("Process %d finished in time %f secs.\n", my_rank, totalTime);
        fflush (stdout);
        printf("\nA total of %d solutions were found.\n\n", solutions);
        
    }
   MPI_Finalize();
   return 0;
}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise
 */

#define EXTRACT_BIT(n,i) ( (n & (1<<i) ) ? 1 : 0)

/* checkCircuit() checks the circuit for a given input.
 * parameters: id, the id of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */

#define SIZE 16

int checkCircuit (int id, int bits) {
    int v[SIZE];
    
    for (int i = 0; i < SIZE; i++)
        v[i] = EXTRACT_BIT(bits,i);
    if(   ( v[ 0] ||  v[ 1]) && (!v[ 1] || !v[ 3]) && ( v[ 2] ||  v[ 3]) && (!v[ 3] || !v[ 4]) 
       && ( v[ 4] || !v[ 5]) && ( v[ 5] || !v[ 6]) && ( v[ 5] ||  v[ 6]) && ( v[ 6] || !v[15]) 
       && ( v[ 7] || !v[ 8]) && (!v[ 7] || !v[13]) && ( v[ 8] ||  v[ 9]) && ( v[ 8] || !v[ 9]) 
       && (!v[ 9] || !v[10]) && ( v[ 9] ||  v[11]) && ( v[10] ||  v[11]) && ( v[12] ||  v[13]) 
       && ( v[13] || !v[14]) && ( v[14] ||  v[15])  ){
        printf ("\n%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id,
        v[15],v[14],v[13],v[12],v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
        fflush (stdout);
        return 1;
    } 
    else {
        return 0;
    }
}
