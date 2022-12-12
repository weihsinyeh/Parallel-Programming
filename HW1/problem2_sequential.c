#include <stdio.h>     
#include <limits.h>    
#include <math.h>
#include <time.h>
#include<stdlib.h>

int checkCircuit (int);
int main (int argc, char *argv[]) {
    long long int tmp = 0;
    long long int number_of_tosses = 1<< 30;
    
    clock_t startTime = clock();
 
    int solutions = 0;
    startTime = clock();
    for (int i = 0; i <= number_of_tosses; i++ ){                        //每個處理器要處理的範圍
        double x = 2.0 * ((double)rand() )/RAND_MAX- 1;
        double y = 2.0 * ((double)rand() )/RAND_MAX - 1;
        double distance_squared = x*x + y*y;
        if (distance_squared <= 1) tmp++;
    }
    clock_t totalTime = clock() - startTime;
    double elapsedTime = (double) (totalTime)/CLOCKS_PER_SEC;
    printf ("Process 0 finished in time %f secs.\n", elapsedTime);
    fflush (stdout);
    double pi_estimate = 4 * tmp/((double)number_of_tosses);
    printf("\nestimate pi is %f.\n\n", pi_estimate);
        
    return 0;
}