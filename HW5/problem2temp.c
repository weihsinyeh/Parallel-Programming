#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
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
void consume_item(char* item){
    printf("consume %s \n",item);
    if(strcmp(item,"hello")==0){
        num[0]++;
    }
    else if(strcmp(item,"parallel")==0){
        num[1]++;
    }
    else if(strcmp(item,"programming")==0){
        num[2]++;
    }
}
void *consumer(){
    char* next_consumed;
    for(int i=0;i<NITER;i++){
        while(buffer_is_empty()){
            sleep(1);
        }
        next_consumed = remove_item();
        consume_item(next_consumed);
    }
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

int main(int argc,char * argv[]){
    int row,col;
    int numberOfThreads = atoi(argv[1]);
    char *filename = argv[2];
    FILE *fp = fopen(filename,"r");
    fscanf(fp,"%d %d\n",&row,&col);
    char * token1 = "hello";
    char * token2 = "parallel";
    char * token3 =  "programming";
    char * token[3] = {token1,token2,token3}; 
    
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
