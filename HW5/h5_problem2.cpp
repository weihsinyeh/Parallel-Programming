#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<omp.h>
int BUFFER_SIZE;      //the number in the buffer
int NITER;            //the number we need to tokenize in every line producer insert
char ** buffer[3000]; //the medium of producer and consumer
int fill = 0;         //the index the producer can input
int use = 0;          //the index the consumer can consume 
char * token[3];      //keywors of "hello" "parallel" "programming"
int num[3] = {0,0,0}; //global number record the frequency of three keywords
int prorow = 0;       //the number record how many producer produce
int conrow = 0;       //the number record how many consumer consume
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
    //first parameter is the number of threads
    int numberOfThreads = atoi(argv[1]);
    printf("number of Threads : %d\n",numberOfThreads);
    // Given a keyword file that contains many keywords.
    //second parameter is the filename
    char *filename = argv[2];
    FILE *fp = fopen(filename,"r");
    // fist line of the input file is the row and col of 2D array of input keywords
    fscanf(fp,"%d %d \n",&row,&col);
    BUFFER_SIZE = col * 13;
    NITER = col;
    
    // Keyword "hello" "parallel" "programming"
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
    //The producers read text from a collection of files, one per producer. 
    for(int i=0;i<row*2;i++){
          int tid = omp_get_thread_num();
          if(i %2 == 0){
              # pragma omp critical (pro)
              {
                while(buffer_is_full()); //thread safe queue
                //The producers insert lines of text into a single shared queue 
                //(please implement your own thread safe queue)
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
              //The consumers take the lines of text and tokenize them.
              next_consumed = remove_item();
            }
            next = *next_consumed;
        
            char * word;
            for(int k=0;k<NITER;k++){
                //Tokens are “words” separated by white space. 
                if(k==0)
                    word = strtok_r(next," ",&saveptr);
                else
                    word = strtok_r(NULL," ",&saveptr);
                //When a consumer finds a token that is keyword, the keyword count increases one. 
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
    //Print each keyword and its count.
    for(int i = 0;i < 3;i++){
      printf("frequency of %s : %d\n",token[i],num[i]);
    }
}

