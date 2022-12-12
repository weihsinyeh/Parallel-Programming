#include <stdio.h>     
#include <limits.h>    
#include <math.h>
#include <time.h>
int checkCircuit (int);
int main (int argc, char *argv[]) {
   
    clock_t startTime = clock();
 
    int solutions = 0;
    startTime = clock();
    for (int i = 0; i <= USHRT_MAX; i++ ){                        //每個處理器要處理的範圍
      int value = checkCircuit (i);                          //檢查範圍中的數字是否滿足
      if(value == 1){                                                 //如果滿足則將答案+1
        solutions ++;
      }
    }
    
    clock_t totalTime = clock() - startTime;
    double elapsedTime = (double) (clock()-startTime)/CLOCKS_PER_SEC;
    printf ("finished in time %f secs.\n", elapsedTime);
    fflush (stdout);
    printf("\nA total of %d solutions were found.\n\n", solutions);
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

int checkCircuit (int bits) {

    int v[SIZE];
    for (int i = 0; i < SIZE; i++)
        v[i] = EXTRACT_BIT(bits,i);
    if(   ( v[ 0] ||  v[ 1]) && (!v[ 1] || !v[ 3]) && ( v[ 2] ||  v[ 3]) && (!v[ 3] || !v[ 4]) 
       && ( v[ 4] || !v[ 5]) && ( v[ 5] || !v[ 6]) && ( v[ 5] ||  v[ 6]) && ( v[ 6] || !v[15]) 
       && ( v[ 7] || !v[ 8]) && (!v[ 7] || !v[13]) && ( v[ 8] ||  v[ 9]) && ( v[ 8] || !v[ 9]) 
       && (!v[ 9] || !v[10]) && ( v[ 9] ||  v[11]) && ( v[10] ||  v[11]) && ( v[12] ||  v[13]) 
       && ( v[13] || !v[14]) && ( v[14] ||  v[15])  ){
        printf ("\n0) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", 
        v[15],v[14],v[13],v[12],v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
        fflush (stdout);
        return 1;
    } 
    else {
        return 0;
    }
}