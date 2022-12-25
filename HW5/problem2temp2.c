#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<omp.h>
#define BUFFER_SIZE 100
#define NITER 10000
char buffer[BUFFER_SIZE];
int fill = 0;
int use = 0;
char * token1;
char * token2;
char * token3;
char * token[3];    
int num[3] = {0,0,0};
int buffer_is_empty(){
    return fill == use;
}
int buffer_is_full(){
    return (fill+1)%BUFFER_SIZE == use;
}
void insert_item(char* item){
    buffer[fill] = item;
    fill = (fill+1)%BUFFER_SIZE;
}
char* remove_item(){
    char* item = buffer[use];
    use = (use+1)%BUFFER_SIZE;
    return item;
}
void tokenize(char* lines[],int line_count,int thread_count){
    char * saveptr = NULL;
    int my_rank,i,j;
    char * my_token;
#   pragma omp parallel num_threads(thread_count) private(my_rank,my_token,i,j)
    {
        my_rank = omp_get_thread_num();
#       pragma omp for schedule(static,1)

        for(i=0;i<line_count;i++){
            my_token = strtok_r(lines[i]," ",&saveptr);
            while(my_token != NULL){
                while(buffer_is_full()){
                    sleep(1);
                }
                insert_item(my_token);
                my_token = strtok_r(NULL," ",&saveptr);
            }
        }
    }
}
void *consumer(){
    int next_consumed;
    for(int i=0;i < NITER;i++){
        while(buffer_is_empty());
        next_consumed = remove_item();
        printf("consume %s \n",next_consumed);
        if(strcmp(next_consumed,token[0])==0){
            # pragma omp critical(hello)
            num[0]++;
        }
        else if(strcmp(next_consumed,token[1])==0){
            # pragma omp critical(parallel)
            num[1]++;
        }
        else if(strcmp(next_consumed,token[2])==0){
            # pragma omp critical(programming)
            num[2]++;
        }
    }

}
int main(int argc,char * argv[]){
    int row,col;
    int numberOfThreads = atoi(argv[1]);
    char *filename = argv[2];
    FILE *fp = fopen(filename,"r");
    fscanf(fp,"%d %d\n",&row,&col);
    token[0] = "hello";
    token[1] = "parallel";
    token[2] =  "programming";

    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            char * lines[row];
            for(int i=0;i<row;i++){
                lines[i] = (char*)malloc(sizeof(char)*BUFFER_SIZE);
                fgets(lines[i],BUFFER_SIZE,fp);
            }
            tokenize(lines,row,numberOfThreads);
        }
        #pragma omp section
        {
            consumer();
        }
    }
}
