#include<iostream>
#include<omp.h>
#include<fstream>
#include<ctime>
#include<fstream>
#include <cstdlib>
using namespace std;
#pragma warning(disable : 4996)
int cmp(const void * a, const void * b){
    return *(int *)a - *(int *)b;
}
int main(int argc,char **argv){
    srand(time(NULL));
    
    int i,j,count;
    int numberOfThreads = atoi(argv[1]); // first parameter is the number of thread
    int n = atoi(argv[2]);               // second parameter is the number of element in array
    int numberOfThreads = atoi(argv[1]);
    // Print the info
    printf("number of element : %d\nnumber of thread : %d\n",n,numberOfThreads);
    // We use two array. a is the array of random number with range from 0 - 999
    // c is the array of the count (rank) we will use in the count sort 
    int *a = new int[n];
    int *c = new int[n];
    int *temp = new int[n];
    // create random array
    for(i = 0;i < n;i++)  a[i] = rand()%(n);
    for(i = 0;i < n;i++){
        c[i] = a[i];
    }
    clock_t starttime,endtime;


    /*** serial-qsort ***/
    // Just call the API of C library
    starttime = clock();
    qsort(c,n,sizeof(int),cmp);
    endtime = clock();
    cout << "Total time serial-qsort: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;

    /*** serial-qsort ***/
    
    /*** serial original count sort ***/
    // The count sort algo is to calculate the specific element's rank in total elements of array
    // After calculating, we push the specific element in their right position (index is just its rank - 1 )
    starttime = clock();
    for(i = 0;i < n;i++){
        count = 0;
        for(j = 0;j < n;j++){
            if(a[j] < a[i]) count++;
            else if(a[j] == a[i] && j < i)count++;
        }
        temp[count] = a[i];
    }
    for(i = 0;i < n;i++)a[i] = temp[i];
    endtime = clock();
    cout << "Total time original count sort: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;
    /*** serial original count sort ***/

    /*** parallel count sort ***/
    // Implement with Open MP
    // We use the same concept of "serial original count sort mentioned aboved，but we parallizied the for i loop.
    // Thus we can assign every thread to cacluate some elements evenly.
    starttime = clock();
#pragma omp parallel for num_threads(numberOfThreads) private(i,j,count) shared(n,a,temp)
    for(i = 0;i < n;i++){
        count = 0;
        for(j = 0;j < n;j++){
            if(a[j] < a[i]) count++;
            else if(a[j] == a[i] && j < i)count++;
        }
        temp[count] = a[i];
    }
#pragma omp parallel for
    for(i = 0;i < n;i++)a[i] = temp[i];
    endtime = clock();
    cout << "Total time parallel count sort: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;
    /*** parallel ***/
}
