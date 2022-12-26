#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include<time.h>
#include<limits.h>
#define INTER 100
int alpha = 1;
int beta = 5;
int rho = 0.7;
int Q = 10;
int tao = 1;
int num_ants = 10;
int num_cities;
int findBestCity(double **matrixpheromone,int **dist,int src){
    double randdouble = ((double)(rand()%100000)*0.00001);
    double cumprob = 0;
    int total = 0;
    for(int i=0;i<num_cities;i++){
        if(i == src) continue;
        total += pow(matrixpheromone[src][i],alpha)*pow(1/dist[src][i],beta);
    }
    double randdoube = randdouble * total;
    for(int i=0;i<num_cities;i++){
        if(i == src) continue;
        cumprob += pow(matrixpheromone[src][i],alpha)*pow(1/dist[src][i],beta);
        if(cumprob >= randdouble)  return i;
    }
}
int main(int argc, char *argv[]){
    srand(time(NULL));
    char * filename = argv[1];
    int num_threads = atoi(argv[2]);
    int cities;
    FILE *fp = fopen(filename,"r");
    //allocate distance matrix
    int **dist  = (int **)malloc(cities*sizeof(int *));
    for(int i=0;i<cities;i++){
        dist[i] = (int *)malloc(cities*sizeof(int));
    }
    //read distance matrix
    for(int i=0;i<cities;i++){
        for(int j=0;j<cities;j++){
            fscanf(fp,"%d",&dist[i][j]);
        }
    }
    fclose(fp);
    int nclonies = 20;
    int nants = 10;
    bool *** antXvisitY = (bool ***)malloc(nclonies*sizeof(bool **));
    int  *** path       = (int  ***)malloc(nclonies*sizeof(int **));
    for(int i=0;i<nclonies;i++){
        antXvisitY[i] = (bool **)malloc(nants*sizeof(bool *));
        path[i]       = (int **) malloc(nants*sizeof(int *));
        for(int j=0;j<nants;j++){
            antXvisitY[i][j] = (bool *)malloc(cities*sizeof(bool));
            path[i][j]       = (int *)malloc(cities*sizeof(int));
        }
    }
    double ** matrixpheromone    = (double **)malloc(cities*sizeof(double *));
    double ** newmatrixpheromone = (double **)malloc(cities*sizeof(double *));
    for(int i=0;i<cities;i++){
        matrixpheromone[i]    = (double *)malloc(cities*sizeof(double));
        newmatrixpheromone[i] = (double *)malloc(cities*sizeof(double));
    }
    int global_Max_interClonies = INT_MAX;
# pragma omp parallel num_threads(num_threads) shared(global_Max_interClonies)
    {
        #pragma omp for //分很多蟻巢(threads)
        {
            for(int numOfClonies = 0;numOfClonies < nclonies;numOfClonies++){
                //每個蟻巢有10隻螞蟻
                //declare a bool two dimension array to record the visited city
                int local_Max_interClonies = INT_MAX;
                for(int i=0;i<nants;i++){
                    for(int j=0;j<cities;j++){
                        antXvisitY[numOfClonies][i][j] = false;
                    }
                }

                for(int i=0;i<INTER;i++){      //預設循環次數上限
                    //initialize the visit
                    for(int k=0;k<nants;k++){
                        for(int j=0;j<cities;j++){
                            antXvisitY[numOfClonies][k][j] = false;
                            //record the path of ant k by matrix
                            path[numOfClonies][k][j] = -1;
                        }
                    }
                    //initialize the pheromone
                    if(i!=0){ //We do not need to initialize the pheromone in the first iteration
                        for(int k=0;k<cities;k++){
                            for(int j=0;j<cities;j++){
                                newmatrixpheromone[k][j] = tao * matrixpheromone[k][j];
                            }
                        }
                    }
                    else{ //initialize the pheromone in the first iteration
                        for(int k=0;k<cities;k++){
                            for(int j=0;j<cities;j++){
                                newmatrixpheromone[k][j] = 1;
                            }
                        }
                    }

                    for(int k=0;k<nants;k++){
                        int nextCity = 0,city = 0,Lk = 0,deltatao =0;
                        antXvisitY[numOfClonies][0][city] = true;
                        //visit all the city
                        for(int j=0;j<cities;j++){ 
                            antXvisitY[numOfClonies][0][city] = true;
                            //choose next city j according to the transition rule
                            while(1){
                                nextCity = findBestCity(matrixpheromone,dist,city);
                                if(antXvisitY[numOfClonies][k][nextCity] == false){ // visit the city
                                    antXvisitY[numOfClonies][k][nextCity] = true;
                                    Lk += dist[city][nextCity];
                                    path[numOfClonies][k][city] = nextCity;
                                    break;
                                }
                            }
                            city = nextCity;
                        }
                        deltatao = Q/Lk;
                        //update pheromone
                        for(int j=0;j<cities;j++){
                            newmatrixpheromone[city][path[numOfClonies][k][city]] += deltatao;
                            newmatrixpheromone[path[numOfClonies][k][city]][city] += deltatao;
                            city = path[numOfClonies][k][city];
                        }
                        if(Lk < local_Max_interClonies){
                            local_Max_interClonies = Lk;
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
                }
            }
        }
    }
    printf("The shortest path is %d",global_Max_interClonies);
}
