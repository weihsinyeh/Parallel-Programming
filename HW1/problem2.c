#include<stdio.h>
#include<limits.h>
#include<mpi.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
double tree_sum(int my_rank,int numprocs,long long int number_of_tosses){
    long long int local_sum = 0,temp=0;
    long long int m = numprocs;
    long long int remain = numprocs, half, rm; 
  
    //the sand of random is time
    srand(time(NULL) + my_rank);  
  
    int tmp = 0;
    //compute the number of counts in circle
    for (long long int toss = 0; toss < number_of_tosses; toss+=numprocs){
        double x = 2.0 * ((double)rand() )/RAND_MAX- 1;
        double y = 2.0 * ((double)rand() )/RAND_MAX - 1;
        double distance_squared = x*x + y*y;
        if (distance_squared <= 1) tmp++;
    }
    //local_sum sum  the counts computed 
    local_sum += tmp;
    
    /***************************tree structure *********************************
    
    when number of processor is odd, tree structure check by remainder.
    if the number of processor which is need to compute is odd in current level,
    then my_rank == middle processor do nothing in this level and wait to compute in next level.  
    
    In the last, 
    because ramain is 1 and break while loop and finialize the level processing int parallel programing.
    every processor will run this while loop and chech what to do by their rank.
    
    ****************************************************************************/
    while(!(remain ==  1)){
        half = remain/2;
        rm   = remain%2;
        if(my_rank < half){
            
            //receive local_sum compute by other processor, ans sum the counts in circles
            MPI_Recv(&temp, 1, MPI_LONG_LONG, my_rank+half+rm, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);      
            local_sum += temp; 
        }
        else if(half+rm <= my_rank && my_rank < remain){ 
            //pass local_sum to other processor, other processor will sum the counts in circles
            MPI_Send(&local_sum, 1, MPI_LONG_LONG, my_rank-half-rm, 0, MPI_COMM_WORLD);  
            break;
        }
        remain = half+rm;
    }
    double pi_estimate = 0;
    //if rank == 0, compute the value of pi
    if(my_rank == 0){ 
        pi_estimate = 4 * local_sum/((double)number_of_tosses);
        return pi_estimate;
    }
    //if rank != 0 return 0, but it don't affect answer,because only rank == 0 print answer
    return pi_estimate; 
}

/***************************bitmask tree_structure pass ******************
bitmask tree_structure pass will be explained detail in my pdf
**************************************************************************/

int broadcast(long long int number_of_tosses, int my_rank, int numprocs, MPI_Comm comm) {
    int        partner;
    unsigned   bitmask = 1;
    int        participate = bitmask << 1;
    
    //chech if there is still the need of broadcast,if the number of processor < bitmask then finish
    while (bitmask < numprocs) { 
        if (my_rank < participate) {   
            partner = my_rank ^ bitmask;
            if (my_rank < partner) {  //when my_rank < parther send number_of_tosses to other node
                if (partner < numprocs ){ 
                    MPI_Send(&number_of_tosses, 1, MPI_LONG_LONG, partner, 0, comm);
                }
            } 
            else {                    //when my_rank >= parther receive number_of_tosses from other node
                MPI_Recv(&number_of_tosses, 1, MPI_LONG_LONG, partner, 0, comm, MPI_STATUS_IGNORE);
            }
        }
        bitmask     = bitmask << 1;   //bitmask can process mort bits.
        participate = bitmask<< 1;    //processor in every turn multiply 2
    }
    return number_of_tosses;

}  
int main (int argc, char *argv[]) {
  
    int my_rank,numprocs,namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    double startTime = 0.0, totalTime = 0.0;
    //MPI initialize
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
    MPI_Get_processor_name(processor_name,&namelen);
    long long int number_of_tosses = 1;
    while(1){
      if(my_rank == 0){
          //input
          printf("Enter the number of intervals: (0 quits) ");
          fflush( stdout );
          scanf("%lld",&number_of_tosses);
          //if my_rank=0 the time of parallel processing start
	        startTime = MPI_Wtime();
          if (number_of_tosses == 0){
             MPI_Finalize();
             break;
          }
      }
      
      if (number_of_tosses == 0){
          MPI_Finalize();
          break;   
      }
      //broadcast number_of_tosses to every processor
      number_of_tosses = broadcast(number_of_tosses,my_rank,numprocs,MPI_COMM_WORLD);
      //get pi_estimate in tree. it is the real pi_estimate when my_rank == 0
      double pi_estimate = tree_sum(my_rank,numprocs,number_of_tosses);
    
      
      //my_rank == 0 print all data
      if(my_rank == 0){
          printf("\nestimate pi is %f.\n\n", pi_estimate);
          fflush(stdout);
          totalTime = MPI_Wtime() - startTime;
          printf ("Process %d finished in time %f secs.\n", my_rank, totalTime);
          fflush (stdout);
      }
      MPI_Finalize();
      break;
    }
    return 0;
}
