#include<iostream>
#include<omp.h>
#include<fstream>
#include<ctime>
#include<stdlib.h>
using namespace std;
#pragma warning(disable : 4996)
int cmp(const void * a, const void * b){
    return *(int *)a - *(int *)b;
}
int main(int argc,char **argv){
    if(argc<2)return 0;
    int i,j,count,n;
    cin >> n;
    int *a = new int[n];
    int *b = new int[n];
    int *c = new int[n];
    int *temp = new int[n];
    for(i = 0;i < n;i++) cin >> b[i];
    for(i = 0;i < n;i++){
        a[i] = b[i];
        c[i] = b[i];
    }
    clock_t starttime,endtime;
    /*** serial-qsort ***/
    starttime = clock();
    qsort(c,n,sizeof(int),cmp);
    endtime = clock();
    cout << "Total time serial-qsort: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;
    /*** serial-qsort ***/
    
    /*** serial ***/
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
    cout << "Total time serial: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;
    /*** serial ***/

    /*** parallel ***/
    starttime = clock();
#pragma omp parallel for private(i,j,count) shared(n,a,temp)
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
    cout << "Total time parallel: " << (double)(endtime - starttime)/CLOCKS_PER_SEC<<endl;
    /*** parallel ***/
}