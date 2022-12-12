#include <stdio.h>     // printf()
#include <limits.h>    
#include <mpi.h>
#include<stdlib.h>
int checkCircuit (int, int);

int main (int argc, char *argv[]) {
   int myid, numprocs;       /* loop variable (32 bits) */
   
   int namelen;
   char processor_name[MPI_MAX_PROCESSOR_NAME];
   long long int number_of_tosses = 1<< 30,tmp =0;

   double startTime = 0.0, totalTime = 0.0;
   //startTime = MPI_Wtime();

   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD,&myid);
   MPI_Get_processor_name(processor_name,&namelen);
   if(myid == 0){
       startTime = MPI_Wtime();
   }
   for (int i = 0; i <= number_of_tosses; i+=numprocs ){                        //每個處理器要處理的範圍
        double x = 2.0 * ((double)rand() )/RAND_MAX- 1;
        double y = 2.0 * ((double)rand() )/RAND_MAX - 1;
        double distance_squared = x*x + y*y;
        if (distance_squared <= 1) tmp++;
   }
   if(myid == 0){
       int temp = 0;
       for(int i = 1; i < numprocs;i++){
            
           MPI_Recv(&temp     ,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           tmp += temp;
       }
   }
   else{
       MPI_Send(&tmp, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);  
   }
   double pi_estimate = 0;
   
   if(myid == 0){
       pi_estimate = 4 * tmp/((double)number_of_tosses);
       for(int i = 1; i < numprocs;i++){  
           MPI_Send(&pi_estimate, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD); 
       }
   }
   else{
       
       MPI_Recv(&pi_estimate     ,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       //printf("%d %f\n",myid,pi_estimate);
   }
   
   if(myid == 0){
       totalTime = MPI_Wtime() - startTime;
       printf ("Process %d finished in time %f secs.\n", myid, totalTime);
       fflush (stdout);
       printf("\nestimate pi is %f.\n\n", pi_estimate);
   }
   MPI_Finalize();
   return 0;
}