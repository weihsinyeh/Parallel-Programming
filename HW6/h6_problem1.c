#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include<time.h>
#include<limits.h>
#include <mpi.h>
/*** for gr17_d.txt */
int alpha = 2;
int beta =5;
double rho = 0.8;
int Q = 2000;
/*** for gr17_d.txt */
 
/*** for fri26_d.txt
int alpha = 2;
int beta =5;
double rho = 0.8;
int Q = 2000;
 *** for fri26_d.txt */
 
/*** for dantzig42_d.txt
int alpha = 2;
int beta =4;
double rho = 0.7;
int Q = 1000;
 *** for dantzig42_d.txt */
 
/*** for att48_d.txt
int alpha = 2;
int beta =5;
double rho = 0.8;
int Q = 40000;
 *** for att48_d.txt */
int cities = 0;
int numberAnt =40;
int num_threads = 10;

int malloc2dint(int ***array, int n, int m);
int free2dint(int ***array);
double malloc2ddouble(double ***array, int n, int m);
double free2ddouble(double ***array);
int main(int argc, char *argv[]){
    int global_best_between_colonies = INT_MAX;   // the shortest path between colonies
    srand(time(NULL));
    char * filename = NULL;
    int **dist = NULL;
    
    /**MPI*******************************************/
    int myid, numprocs;       
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    /**MPI*******************************************/
    
    /****rank = 0 ******************************/
    if(myid == 0){
        filename = argv[1];
        //determine the number of cities
        if(strcmp(filename,"fri26_d.txt")==0) cities = 26;
        else if(strcmp(filename,"gr17_d.txt")==0) cities = 17;
        else if(strcmp(filename,"dantzig42_d.txt")==0)cities = 42;
        else if(strcmp(filename,"att48_d.txt")==0)cities = 48;
        printf("filename = %s\n",filename);
        printf("number of ant of colony : %d\n",numberAnt);
        printf("number of thread : %d\n",num_threads);
        printf("cities : %d\n",cities);
        printf("alpha = %d\n",alpha);
        printf("beta = %d\n",beta);
        printf("rho = %f\n",rho);
        printf("Q = %d\n",Q);
    }
    /****rank = 0 ******************************/
    
    
    //rank0 broadcast the number of cities to other core
    MPI_Bcast(&cities,1,MPI_INT,0,MPI_COMM_WORLD);    
    // every core have to allocate a two dimensional matrix
    malloc2dint(&dist, cities, cities);


    /****rank = 0 read file and record distance***/
    if(myid == 0 ){  
        FILE *fp = fopen(filename,"r");
        for(int i=0;i<cities;i++){
            for(int j=0;j<cities;j++){
                fscanf(fp,"%d ",&dist[i][j]);
            }
        }
        fclose(fp);
    }
    /****rank = 0 ******************************/
    
    //Wair for the finish of root processor read file
    MPI_Barrier(MPI_COMM_WORLD);  
    
    /****rank 0  broad cast the distance of the matrix to other core****/
    for(int i=0;i<cities;i++){    
      MPI_Bcast(&(dist[i][0]),cities,MPI_INT,0,MPI_COMM_WORLD);
    }
    /****rank 0  broad cast the distance of the matrix to other core****/
    

    /**** initialize the global matrix of pheromone ***/
    double ** globalpheromone  = NULL;
    malloc2ddouble(&globalpheromone,cities,cities);
    for(int i=0;i<cities;i++){
        for(int j=0;j<cities;j++){
            globalpheromone[i][j] = 0;
        }
    }
    /**** initialize the global matrix of pheromone ***/
    
    
    /**** Each thread runs a whole ant colony, which will affect the critical section *****/
# pragma omp parallel num_threads(num_threads) shared(global_best_between_colonies)
    {
        bool * visit = (bool *)malloc(cities*sizeof(bool ));        // record which city the ant visit
        int  * next  = (int *)malloc(cities*sizeof(int));           // record the next city ant pass 
        double * prob = (double *) malloc(cities * sizeof(double)); // record the probability of not visited city
        int local_best_in_every_colony = INT_MAX;                   // the local shortest of every colony
        
        /**** declare pheromone matrix ******************/
        double ** pheromone     = NULL;
        double ** newpheromone  = NULL;
        malloc2ddouble(&pheromone,cities,cities);
        malloc2ddouble(&newpheromone,cities,cities);
        for(int i=0;i<cities;i++){
            for(int j=0;j<cities;j++){
                pheromone[i][j] = 0;
                newpheromone[i][j] = 0;
            }
        }
        /**** declare pheromone matrix ******************/
        
        for(int i=0;i<numberAnt;i++){     
            for(int j=0;j<cities;j++){
                visit[j] = false;    
                next[j] = -1;
                for(int k=0;k<cities;k++){
                    if(i!=0){
                        // If it is not the first ant of the colony it will reference the pheromone 
                        // that ant pass previous, but the pheromone will decrease with parameter "rho" 
                        pheromone[j][k] = rho * newpheromone[j][k];
                    }
                    else{ 
                        // If it is the first ant of the colony every path is unknown 
                        // So the pheromone is random.
                        pheromone[j][k] = ((double)(rand()%100000)) /(double)100000 ;
                        newpheromone[j][k] = pheromone[j][k] ;
                    }
                }
            }
            //Place the m ants on n random cities
            int first = rand()%cities;
            int nextCity = 0,city = first,TotalDistance = 0; // record the total distance; 
            
            //visit all the city 
            for(int j=0;j<cities-1;j++){  
                visit[city] = true;      
                double total = 0;        // total probability
                //first initialize every probability.
                for(int k=0;k<cities;k++)  prob[k] = 0;
                
                /******calculate every Numerator of every city***********************/
                for(int k=0;k<cities;k++){
                    if(!visit[k]){
                        double distR = 1/(double)dist[city][k];
                        double child = pow(pheromone[city][k],alpha)*pow(distR,beta);
                        prob[k] = child;
                        total += prob[k];
                     }
                }
                /******calculate nextCity by cumulative probability****************/
                while(1){
                    double randnum = ( (double)(rand()%100000)) /(double)100000 ;
                    double cumprob = 0; // cumulative probability
                    bool find =false;
                    for(int k=0;k<cities;k++){
                       if(!visit[k]){
                           double a = prob[k]/(double)(total);
                           cumprob += a;
                           if(cumprob >= randnum) {
                              nextCity =  k;
                              TotalDistance += dist[city][nextCity];
                              next[city] = nextCity;
                              find = true;
                              break;
                           }
                       }
                    }  
                    if(find) break;        
                }
                city = nextCity;
                /******calculate nextCity by cumulative probability****************/
            }  
            //Add the distance when back to the source city
            TotalDistance += dist[nextCity][first];
            visit[nextCity] = true;
            double deltatao = (double)Q/(double)TotalDistance; // cacluate the deltatao.
            city = first;
 
            for(int j=0;j<cities-1;j++){
                // add the deltatao to every path that the ant visited.
                //every ant will bring its deltatao to the pheromone matrix
                newpheromone[city][next[city]] += deltatao; 
                newpheromone[next[city]][city] += deltatao;
                city = next[city];
            }

            if(TotalDistance < local_best_in_every_colony) local_best_in_every_colony = TotalDistance;
            if(i%5 == 0){   // share the best tour of pheromone matrix every five ant in colony;
                #pragma omp barrier
                // find which one is the best tour of pheromone matrix
                #pragma omp critical(inter)
                {
                    // update the global best shortest path between every thread 
                    if(local_best_in_every_colony < global_best_between_colonies){
                        global_best_between_colonies = local_best_in_every_colony;
                    }
                }
                #pragma omp barrier
                // If my local best shortes path is the global shortest path,
                // then my pheromone matrix is the best
                if(global_best_between_colonies == local_best_in_every_colony){// 
                    for(int j = 0;j <cities;j++){
                        for(int k = 0;k <cities;k++){
                            globalpheromone[j][k] = newpheromone[j][k];
                        }
                    }
                }
                #pragma omp barrier
                // If my local best shortes path is not the global shortest path,
                // then my pheromone matrix is the best
                if(global_best_between_colonies != local_best_in_every_colony){
                    for(int j = 0;j <cities;j++){
                        for(int k = 0;k <cities;k++){
                            newpheromone[j][k] = globalpheromone[j][k];
                        }
                    }
                }
            }
        } 
        #pragma omp critical(outer)
        {
            // Last time : update the global best shortest path between every thread 
            if(local_best_in_every_colony < global_best_between_colonies){
                global_best_between_colonies = local_best_in_every_colony;
            }
        }
    }
    //the shortest path of every core
    printf("The shortest path of core%d is %d\n",myid,global_best_between_colonies);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if(myid == 0){
       int temp = 0;
       for(int i = 1; i < numprocs;i++){ 
           MPI_Recv(&temp     ,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           if(temp < global_best_between_colonies) global_best_between_colonies = temp;
       }
    }
    else{
       MPI_Send(&global_best_between_colonies, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (myid == 0){ //Printing the best tour 
        printf("The shortest path is %d\n",global_best_between_colonies);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    free2dint(&dist);
    MPI_Finalize();
    return 0; 
    
}
int malloc2dint(int ***array, int n, int m) {

    /* allocate the n*m contiguous items */
    int *p = (int *)malloc(n*m*sizeof(int));
    if (!p) return -1;

    /* allocate the row pointers into the memory */
    (*array) = (int **)malloc(n*sizeof(int*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    /* set up the pointers into the contiguous memory */
    for (int i=0; i<n; i++) 
       (*array)[i] = &(p[i*m]);

    return 0;
}

int free2dint(int ***array) {
    /* free the memory - the first element of the array is at the start */
    free(&((*array)[0][0]));

    /* free the pointers into the memory */
    free(*array);

    return 0;
}
double malloc2ddouble(double ***array, int n, int m) {

    /* allocate the n*m contiguous items */
    double *p = (double *)malloc(n*m*sizeof(double));
    if (!p) return -1;

    /* allocate the row pointers into the memory */
    (*array) = (double **)malloc(n*sizeof(double*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    /* set up the pointers into the contiguous memory */
    for (int i=0; i<n; i++) 
       (*array)[i] = &(p[i*m]);

    return 0;
}

double free2ddouble(double ***array) {
    /* free the memory - the first element of the array is at the start */
    free(&((*array)[0][0]));

    /* free the pointers into the memory */
    free(*array);

    return 0;
}
