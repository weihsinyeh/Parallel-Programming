#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include<time.h>
#include<limits.h>
#define INTER 5
int alpha = 1;
int beta = 1;
int rho = 0.7;
int Q = 40;
int tao = 1;
int num_ants = 10;
int cities;
double powM(double base,int up){
  double ret = 1;
  for(int i=0;i<up;i++){
    ret *= base;
  }
  return ret;
}
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
      
    srand(time(NULL));
    char * filename = argv[1];
    int num_threads = atoi(argv[2]);
    cities = atoi(argv[3]);

    FILE *fp = fopen(filename,"r");
    //allocate distance matrix
    int **dist  = (int **)malloc(cities*sizeof(int *));
    for(int i=0;i<cities;i++){
        dist[i] = (int *)malloc(cities*sizeof(int));
    }
    //read distance matrix
    for(int i=0;i<cities;i++){
        for(int j=0;j<cities;j++){
            fscanf(fp,"%d ",&dist[i][j]);
        }
    }
    fclose(fp);
    int nants = 1;
    int global_Max_interClonies = INT_MAX;
# pragma omp parallel num_threads(num_threads) shared(global_Max_interClonies)
    {
        bool ** antXvisitY = (bool **)malloc(nants*sizeof(bool *));
        for(int i=0;i<cities;i++){
            antXvisitY[i] = (bool *)malloc(nants*sizeof(bool ));
        }
        double ** matrixpheromone    = (double **)malloc(cities*sizeof(double *));
        double ** newmatrixpheromone = (double **)malloc(cities*sizeof(double *));
        int ** path                  = (int **)malloc(cities*sizeof(int *));
        for(int i=0;i<cities;i++){
            matrixpheromone[i]        = (double *)malloc(cities*sizeof(double));
            newmatrixpheromone[i]     = (double *)malloc(cities*sizeof(double));
            path[i]                   = (int *)malloc(cities*sizeof(int));
        }
        //每個蟻巢有10隻螞蟻
        
        //declare a bool two dimension array to record the visited city
        int local_Max_interClonies = INT_MAX;
        for(int i=0;i<nants;i++){
            for(int j=0;j<cities;j++){
                antXvisitY[i][j] = false;
            }
        }
        
        for(int i=0;i<INTER;i++){      //預設循環次數上限
            //initialize the visit
            for(int k=0;k<nants;k++){
                for(int j=0;j<cities;j++){
                    antXvisitY[k][j] = false;
                        path[k][j] = -1; //record the path of ant k by matrix
                }
            }
            //initialize the pheromone
            if(i!=0){ //We do not need to initialize the pheromone in the first iteration
                for(int k=0;k<cities;k++){
                    for(int j=0;j<cities;j++){
                        matrixpheromone[k][j] = tao * newmatrixpheromone[k][j];
                    }
                }
            }
            else{ //initialize the pheromone in the first iteration
               for(int k=0;k<cities;k++){
                    for(int j=0;j<cities;j++){
                        matrixpheromone[k][j] = ( (double)(rand()%100000)) /(double)100000 ;
                    }
               }
            }
            for(int k=0;k<nants;k++){
               int nextCity = 0,city = 0,Lk = 0;
               double deltatao =0;
               antXvisitY[0][city] = true;
               //visit all the city
               for(int j=0;j<cities-1;j++){ 
                    antXvisitY[0][city] = true;
                    nextCity = findBestCity(matrixpheromone,dist,city);
                    while(1){
                        //printf("next City %d\n",nextCity);
                        //nextCity = random()%cities;
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
                //printf("deltatao = %f\n",deltatao);
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
    }
    printf("The shortest path is %d\n",global_Max_interClonies);
    
}
