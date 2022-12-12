//F74109016 
#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include "bmp.h"
#include <cstddef>
#include<algorithm>

using namespace std;
      
int main(int argc,char *argv[]){
    /****** Initialize*********************/
    double startwtime = 0.0, endwtime=0;
    int size,rank,totalNumber = 0,partner;

    int *offsets           =0;
    int *localNumOfElement =0;
    int *globalArray = NULL;
    int *subArray = NULL;
    int *receiveArray = NULL;
    int *temp = NULL;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Status status;
    
    srand(time(NULL)+rank); //random sand
    
    //Read Input form user
    if(rank == 0){
        printf("Input the number that need to be sorted\n");
        scanf("%d",&totalNumber);
    }
    /****** Initialize*********************/
    
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0){                //Root Processor's timer start
        startwtime = MPI_Wtime();  
    }
    
    /****** Broadcast information**********/
    //Because only processor Root read file, the information of file need to be broadcast.
    MPI_Bcast(&totalNumber,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    
    //After every processor receive the totalNumber from processor root they allocate the array
    globalArray = new int[totalNumber];
    /****** Broadcast information**********/
    
    
    for(int i=0;i<totalNumber;i++){
      globalArray[i] = 0;
    }
    
    /*****Every processor need to calculate how many element they need to work********/
    localNumOfElement = new int[size];  //the distance from the address of BMPSaveData
    offsets = new int[size];            //the number of element in this block
    
    int remainder = totalNumber % size;
    int sum = 0;
    for(int i=0;i<size;i++){
        localNumOfElement[i] = totalNumber / size;
        if(remainder > 0){
            localNumOfElement[i] += 1;
            remainder -= 1;
        }
        offsets[i] = sum;
        sum += localNumOfElement[i];
    }
    /*****Every processor need to calculate how many element they need to work********/ 
     
     
    /*****allocate buffer******************/
    receiveArray = new int[localNumOfElement[rank]+2];  //the buffer store elements receive from partner
    subArray = new int[localNumOfElement[rank]+2];      //the buffer store each process's own data
    /*****allocate buffer******************/
    
    
    /*****random generator*****************/
    for(int i=0;i<localNumOfElement[rank];i++){
        subArray[i] = (rand()+1) % 65536;
    }
    /*****random generator*****************/
    
    
    /*****sort the part of data (the process responsible) before odd-even sort********/
    MPI_Barrier(MPI_COMM_WORLD);
    sort(subArray,subArray + localNumOfElement[rank]);
    MPI_Barrier(MPI_COMM_WORLD);
    /*****sort the part of data (the process responsible) before odd-even sort********/
    
    
    /*****Odd Even Sort**********************/
    for(int phase = 0;phase < size;phase++){
        bool work = true;
        
        /*****Find Partner**********************/
        if(phase % 2 == 0){
            if(rank % 2 != 0)       //odd rank
                partner = rank - 1;
            else                    //even rank
                partner = rank + 1;
        }
        else{
            if(rank % 2 != 0)
                partner = rank + 1;
            else
                partner = rank - 1;
        }
        if(partner == - 1 || partner == size){
            partner = MPI_PROC_NULL;
            work = false;
        }
        /*****Find Partner**********************/
         
        /*****Information change by processor***/
        MPI_Sendrecv(&subArray[0],                 //const void *sendbuf
                     localNumOfElement[rank],      //int sendcount
                     MPI_INT,                      //MPI_Datatype sendtype
                     partner,                      //int dest
                     1,                            //int sendtag
                     &receiveArray[0],             //void *recvbuf
                     localNumOfElement[partner],   //int recvcount,  
                     MPI_INT,                      //MPI_Datatype recvtype
                     partner,                      //int source, 
                     1,                            //int recvtag
                     MPI_COMM_WORLD,               //MPI_COMM_WORLD MPI_Comm comm
                     &status);                      //MPI_Status *status) 
        /*****Information change by processor***/
         
        /*****allocate memeory to store immediate result in merge sort**/  
        temp = new int[localNumOfElement[rank] + 1];
        for(int i = 0;i < localNumOfElement[rank];i++){
            temp[i] = subArray[i];

        }
        /*****merge ************************/
        if(work){
            if(rank < partner){ //if rank < partner then this rank store the smaller part 
                int i,j,k;
                int localN = localNumOfElement[rank];
                int partnerN = localNumOfElement[partner];
                // i point to the right subarray's element
                // j point to the left subarray's element
                // k point to the new subarray's element
                for(i=j=k=0;k<localN;k++){   
                    if(j == partnerN || (i < localN && temp[i] < receiveArray[j])){
                        subArray[k] = temp[i++];
                    }
                    else{
                        subArray[k] = receiveArray[j++];
                    }
                }
            }
            else{               //if rank > partner then this rank store the bigger part
                int i,j,k;
                int localN = localNumOfElement[rank];
                int partnerN = localNumOfElement[partner];
                // i point to the right subarray's element
                // j point to the left subarray's element
                // k point to the new subarray's element
                for(i=k=localN-1,j=partnerN-1;k>=0;k--){
                    if(j == -1 || (i >=0 && temp[i] >= receiveArray[j])){
                        subArray[k] = temp[i--];
                    }
                    else{
                        subArray[k] = receiveArray[j--];
                    }
                }
            }
        }
        /*****merge ************************/
    }
    /*****Odd Even Sort**********************/
    
    
    MPI_Barrier(MPI_COMM_WORLD);
 
   //gather every part the processor compare
    MPI_Gatherv(subArray,  
                localNumOfElement[rank], 
                MPI_INT,
                globalArray,
                localNumOfElement, 
                offsets, 
                MPI_INT, 
                0, 
                MPI_COMM_WORLD);
           
    if(rank == 0){
        //terminate the timer and print
        endwtime = MPI_Wtime();  
        cout << "The execution time = "<< endwtime-startwtime <<endl ;
        
        printf("final sorted data:\n");
        for(int i = 0;i < totalNumber;i++){
            printf("%d ",globalArray[i]);
        }
    }
    MPI_Finalize();
    return 0;
}