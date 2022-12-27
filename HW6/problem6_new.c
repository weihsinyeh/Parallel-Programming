#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include<time.h>
#include<limits.h>
#include <mpi.h>
#define INTER 5
int alpha = 1;
int beta = 1;
int rho = 0.7;
int Q = 40;
int tao = 1;
int nants =0;
int cities = 0;
int **dist = NULL;
int findBestCity(double **matrixpheromone,int **dist,int src){
    double randdouble = ( (double)(rand()%100000)) /(double)100000 ;
    //printf("randdouble = %f\n",randdouble);
    double cumprob = 0;
    double total = 0;
    double * prob = (double *) malloc(cities * sizeof(double));
    for(int i=0;i<cities;i++){
        if(i == src) continue;
        double distR = 1/(double)dist[src][i];
        double child = pow(matrixpheromone[src][i],alpha)*pow(distR,beta);
        if(!isnormal(child)) child = 0;
        prob[i] = child;
        total += prob[i];
    }
    //printf("total = %f\n",total);
    for(int i=0;i<cities;i++){
        double distR = 1/(double)dist[src][i];
        double a = prob[i]/(total);
        //printf("%f\n",a);
        cumprob += a;
        //printf("cumprob = %f\n",cumprob);
        if(cumprob >= randdouble) {
          if(i == src && i != 0) return  i-1;
          if(i == src && i == 0) return  i+1;
          return i;
        }
    }
  
}
int main(int argc, char *argv[]){
    int global_Max_interClonies = INT_MAX;
    int global_data;
    /**MPI*******************************************/
    int n, myid, numprocs;       /* loop variable (32 bits) */
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    /**MPI*******************************************/
    
    /****rank = 0 ******************************/
    srand(time(NULL));
    char * filename = argv[1];
    int num_threads = 5;
    int length = sizeof(filename) / sizeof(filename[0]);
    char ten;
    char onee;
    for(int i=0;i<length;i++){
        if(i==length-5) ten = filename[i];
        if(i==length-4) onee = filename[i];
    }
    cities = (ten-48) *10 + onee-48;
    printf("%d\n",cities);
    
    //allocate distance matrix
    dist  = (int **)malloc(cities*sizeof(int *));
    for(int i=0;i<cities;i++){
        dist[i] = (int *)malloc(cities*sizeof(int));
    }
    for(int i=0;i<cities;i++){
        for(int j=0;j<cities;j++){
            dist[i][j] = 0;
        }
    }
    //if(myid == 0 ){  
        FILE *fp = fopen(filename,"r");
        //read distance matrix
        for(int i=0;i<cities;i++){
            for(int j=0;j<cities;j++){
                fscanf(fp,"%d ",&dist[i][j]);
            }
        }
        fclose(fp);
    //}
    /****rank = 0 ******************************/
    MPI_Barrier(MPI_COMM_WORLD);  //Wair for the finish of root processor read file
     
    nants = 1;

# pragma omp parallel num_threads(num_threads) shared(global_Max_interClonies)
    {
        bool ** antXvisitY = (bool **)malloc(nants*sizeof(bool *));
        for(int i=0;i<nants;i++){
            antXvisitY[i] = (bool *)malloc(cities*sizeof(bool ));
        }
        double ** matrixpheromone    = (double **)malloc(cities*sizeof(double *));
        double ** newmatrixpheromone = (double **)malloc(cities*sizeof(double *));
        double * prob = (double *) malloc(cities * sizeof(double));
        int ** path                  = (int **)malloc(cities*sizeof(int *));
        for(int i=0;i<cities;i++){
            matrixpheromone[i]        = (double *)malloc(cities*sizeof(double));
            newmatrixpheromone[i]     = (double *)malloc(cities*sizeof(double));
            path[i]                   = (int *)malloc(cities*sizeof(int));
        }
        
        int local_Max_interClonies = INT_MAX;
        for(int i=0;i<nants;i++){
            for(int j=0;j<cities;j++){
                antXvisitY[i][j] = false;
            }
        }
        printf("myrank = %d\n",myid);
        
        for(int i=0;i<INTER;i++){      //預設循環次數上限
            //initialize the visit
            for(int k=0;k<nants;k++){
                for(int j=0;j<cities;j++){
                    antXvisitY[k][j] = false;
                    path[k][j] = -1; //record the path of ant k by matrix
                }
            }
            //initialize the pheromone
            for(int k=0;k<cities;k++){
                for(int j=0;j<cities;j++){
                    if(i!=0)
                        matrixpheromone[k][j] = tao * newmatrixpheromone[k][j];
                    else
                        matrixpheromone[k][j] = ((double)(rand()%100000)) /(double)100000 ;
                }
            }
            
            for(int k=0;k<nants;k++){
               int nextCity = 0,city = 0,Lk = 0;
               double deltatao =0;
               for(int j=0;j<cities-1;j++){ 
                    antXvisitY[0][city] = true;
                    
                    /******calculate nextCity ***************************************/
                    
                    double randdouble = ( (double)(rand()%100000)) /(double)100000 ;
                    //printf("randdouble = %f\n",randdouble);
                    double cumprob = 0;
                    double total = 0;
                    for(int kk=0;kk<cities;kk++){
                        if(kk == city) continue;
                        double distR = 1/(double)dist[city][kk];
                        double child = pow(matrixpheromone[city][kk],alpha)*pow(distR,beta);
                        if(!isnormal(child)) child = 0;
                        prob[kk] = child;
                        total += prob[kk];
                    }
                   //printf("total = %f\n",total);
                    for(int kk=0;kk<cities;kk++){
                        double distR = 1/(double)dist[city][kk];
                        double a = prob[kk]/(total);
                        //printf("%f\n",a);
                        cumprob += a;
                        //printf("cumprob = %f\n",cumprob);
                        if(cumprob >= randdouble) {
                            if(kk == city && kk != 0) nextCity = kk-1;
                            if(kk == city && kk == 0) nextCity = kk+1;
                            nextCity =  i;
                        }
                    }          
                              
                    /******calculate nextCity ***************************************/
                    //nextCity = findBestCity(matrixpheromone,dist,city);
                    nextCity = random()%cities;
                    while(1){
                        if(antXvisitY[k][nextCity] == false){ // visit the city
                            antXvisitY[k][nextCity] = true;
                            Lk += dist[city][nextCity];
                            path[k][city] = nextCity;
                            break;
                        }
                        else{
                          nextCity = random()%cities;
                        }
                    }
                    city = nextCity;
                }
                deltatao = (double)Q/(double)Lk;
                //update pheromone
                city = 0;
                for(int j=0;j<cities-1;j++){
                    newmatrixpheromone[city][path[k][city]] += deltatao;
                    newmatrixpheromone[path[k][city]][city] += deltatao;
                    city = path[k][city];
                }
                if(Lk < local_Max_interClonies) local_Max_interClonies = Lk;
            }
            if(i%5 == 0){
                #pragma omp critical
                {
                    if(local_Max_interClonies < global_Max_interClonies){
                        global_Max_interClonies = local_Max_interClonies;
                    }
                }
                #pragma omp barrier
            }
        } 
        //free(newmatrixpheromone);   
        //free(matrixpheromone); 
        //free(path); 
        //free(antXvisitY); 
        //free(prob); 
    }
    printf("The shortest path is %d %d\n",myid,global_Max_interClonies);

    MPI_Barrier(MPI_COMM_WORLD);
    if(myid == 0){
       int temp = 0;
       for(int i = 1; i < numprocs;i++){
            
           MPI_Recv(&temp     ,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
           if(temp < global_Max_interClonies) global_Max_interClonies = temp;
       }
     }
     else{
       MPI_Send(&global_Max_interClonies, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  
     }

    if (myid == 0){
        printf("The shortest path is %d\n",global_Max_interClonies);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(myid == 0)
        free(dist);
    MPI_Finalize();
    return 0; 
}
