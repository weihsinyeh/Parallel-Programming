#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <linux/cn_proc.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <utime.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(int argc,char * argv[]) {

   srand(time(NULL));
   char * token1 = "hello";
   char * token2 = "parallel";
   char * token3 =  "programming";
   char * token[3] = {token1,token2,token3};    
   int num[3] = {3334,3333,3333};


   FILE *writeMatrix = fopen("test100.txt","w");
   fprintf(writeMatrix,"%d %d\n",100,100);
   for(int i=0;i<100;i++){
        for(int j=0;j<100;j++){
            int rand;
            while(1){
                rand = random()%3;
                if(num[rand] > 0){
                    num[rand]--;
                    break;
                }
            }
            fprintf(writeMatrix,"%s ",token[rand]);
        }
      fprintf(writeMatrix,"\n");
   }
   fclose(writeMatrix);
   
   return 0;
}
