
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<omp.h>
int BUFFER_SIZE;
int NITER;
char ** buffer[3000];
int fill = 0;
int use = 0;
char * token[3];    
int num[3] = {0,0,0};
int buffer_is_empty(){
    return fill == use;
}
int buffer_is_full(){
    return (fill+1)%BUFFER_SIZE == use;
}
void insert_item(char** item){
    buffer[fill] = item;
    fill = (fill+1)%BUFFER_SIZE;
}
char** remove_item(){
    char** item = buffer[use];
    use = (use+1)%BUFFER_SIZE;
    return item;
}

void producer(char** lines,int line_count,int thread_count){
    while(buffer_is_full());
    insert_item(lines);
}
int prorow = 0;
int conrow = 0;
void *consumer(){
    char * my_token;
    //When a consumer finds a token that is keyword, the keyword count increases one. Please print each keyword and its count.
    char ** next_consumed;
    char * next;
    char * saveptr = NULL;

    while(buffer_is_empty());
        next_consumed = remove_item();
        next = *next_consumed;
 
        char * word;
        for(int i=0;i<NITER;i++){
            if(i==0)
                word = strtok_r(next," ",&saveptr);
            else
                word = strtok_r(NULL," ",&saveptr);
            if(strcmp(word,token[0])==0){
                # pragma omp critical(hello)
                num[0]++;
            }
            else if(strcmp(word,token[1])==0){
                # pragma omp critical(parallel)
                num[1]++;
            }
            else if(strcmp(word,token[2])==0){
                # pragma omp critical(programming)
                num[2]++;
            }
        }
    
}
int main(int argc,char * argv[]){
    int row,col,numOfke;
    int numberOfThreads = atoi(argv[1]);
    // Given a keyword file that contains many keywords.
    char *filename = argv[2];
    FILE *fp = fopen(filename,"r");
    fscanf(fp,"%d %d \n",&row,&col);
    BUFFER_SIZE = col * 13;
    NITER = col;
    
    // Keyword
    token[0] = "hello";
    token[1] = "parallel";
    token[2] =  "programming";

    char * lines[row];
    for(int i=0;i<row;i++){
      lines[i] = (char*)malloc(sizeof(char)*BUFFER_SIZE);
      fgets(lines[i],BUFFER_SIZE,fp);
    }
    
    //Use OpenMP to implement a producer-consumer program in which 
    //some of the threads are producers and others are consumers. 
    
    #pragma omp parallel for num_threads(numberOfThreads) schedule(static,1)
    for(int i=0;i<row*2;i++){
          int tid = omp_get_thread_num();
          if(i %2 == 0){
              printf("row = %d",i);
              # pragma omp critical (pro)
              {
                while(buffer_is_full());
                insert_item(&lines[i/2]);
              }  
          }
          else{
            char * my_token;
            char ** next_consumed;
            char * next;
            char * saveptr = NULL;
            # pragma omp critical (con)
            {
              while(buffer_is_empty());
              next_consumed = remove_item();
            }
            next = *next_consumed;
            //printf("%s\n",next);
        
            char * word;
            for(int k=0;k<NITER;k++){
                if(k==0)
                    word = strtok_r(next," ",&saveptr);
                else
                    word = strtok_r(NULL," ",&saveptr);
                if(strcmp(word,token[0])==0){
                    # pragma omp critical(hello)
                    num[0]++;
                }
                else if(strcmp(word,token[1])==0){
                    # pragma omp critical(parallel)
                    num[1]++;
                }
                else if(strcmp(word,token[2])==0){
                    # pragma omp critical(programming)
                    num[2]++;
                }
            }
        }
    }
    
    for(int i = 0;i < 3;i++){
      printf("frequency of %s : %d\n",token[i],num[i]);
    }
}

