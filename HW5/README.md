# 多處理機平行程式設計 HW5
###### tags: `多處理機平行程式設計`
[作業五說明](https://hackmd.io/@ZZta/ByG45r8Qj)
[GeeksforGeeks](https://www.geeksforgeeks.org/counting-sort/)
[MyGitHub](https://github.com/weihsinyeh/parallelprogramming/tree/main/HW5)
## 題目1. Count Sort
Count sort is a simple serial sorting algorithm that can be implemented as follows:
```c=
void Count_sort(int a[], int n) { 
    int i, j, count;
    int* temp = malloc(n∗sizeof(int));
    for (i = 0; i < n; i++) { 
        count = 0; 
        for (j = 0; j < n; j++) 
            if (a[j] < a[i]) count++; 
        else if (a[j] == a[i] && j < i)
            count++;
        temp[count] = a[i]; 
    } 
    memcpy(a, temp, n∗sizeof(int)); 
    free(temp); 
} 
```
### Count sort 
* The basic idea is that for each element a[i] in the list a, we count the number of elements in the list that are less than a[i]. 
* Then we insert a[i] into a temporary list using the subscript determined by the count. 
* There’s a slight problem with this approach when the list contains equal elements, since they could get assigned to the same slot in the temporary list. 
* The code deals with this by incrementing the count for equal elements on the basis of the subscripts. 
* If both a[i] == a[j] and j <i, then we count a[j] as being “less than” a[i].
* After the algorithm has completed, we overwrite the original array with the temporary array using the string library function memcpy.

### 問題 :
Please answer the following questions:

#### 問題1 : 
If we try to parallelize the for i loop (the outer loop), which variables should be private and which should be shared?
#### 問題2 : 
If we parallelize the for i loop using the scoping you speciﬁed in the previous part,are there any loop-carried dependences? Explain your answer.
#### 問題3 : 
Can we parallelize the call to memcpy? Can we modify the code so that this part of the function will be parallelizable?
#### 問題4 : 
Write a C program that includes a parallel implementation of Count sort.
#### 問題5 : 
How does the performance of your parallelization of Count sort compare to serial Count sort? How does it compare to the serial qsort library function?
```c=
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
```

## 2. Producer-Consumer
![](https://i.imgur.com/yEj8PQl.png)

![](https://i.imgur.com/wlZ70Us.png)

Given a ==keyword file== that contains many keywords. 

Use OpenMP to implement a ==producer-consumer program== in which some of **the threads are producers and others are consumers. **

The producers read text from a collection of files, one per producer. 

They insert lines of text into a ==single shared queue== (please implement your own thread safe queue)NEW. 

The consumers take the lines of text and tokenize them. Tokens are “words” separated by white space. 

When a consumer finds a token that is keyword, the keyword count increases one. Please print each keyword and its count.

https://gist.github.com/weihsinyeh/290a5e7811ec8bab899c07313c4b7107

Producer produced [37].(Placed in index:in=0,out=0)
Producer produced [53].(Placed in index:in=1,out=0)
Producer produced [93].(Placed in index:in=2,out=0)
Producer produced [49].(Placed in index:in=3,out=0)
                Consumer consumed [37].(in=1,Consumed from index: out=0)
                Consumer consumed [53].(in=4,Consumed from index: out=1)
                Consumer consumed [93].(in=4,Consumed from index: out=2)
                Consumer consumed [49].(in=4,Consumed from index: out=3)
Producer produced [41].(Placed in index:in=4,out=1)
                Consumer consumed [41].(in=0,Consumed from index: out=4)
Producer produced [42].(Placed in index:in=0,out=4)
Producer produced [67].(Placed in index:in=1,out=0)
Producer produced [25].(Placed in index:in=2,out=0)
                Consumer consumed [42].(in=1,Consumed from index: out=0)
                Consumer consumed [67].(in=3,Consumed from index: out=1)
                Consumer consumed [25].(in=3,Consumed from index: out=2)
Producer produced [10].(Placed in index:in=3,out=0)
                Consumer consumed [10].(in=4,Consumed from index: out=3)
Producer produced [30].(Placed in index:in=4,out=3)
                Consumer consumed [30].(in=0,Consumed from index: out=4)
                
### Reference
[簡易的程式平行化－OpenMP（三）範例 parallel、section](https://kheresy.wordpress.com/2006/09/15/%E7%B0%A1%E6%98%93%E7%9A%84%E7%A8%8B%E5%BC%8F%E5%B9%B3%E8%A1%8C%E5%8C%96%EF%BC%8Dopenmp%EF%BC%88%E4%B8%89%EF%BC%89%E7%AF%84%E4%BE%8B-parallel%E3%80%81section/)

[Github](https://gist.github.com/weihsinyeh/290a5e7811ec8bab899c07313c4b7107)

[allocate array of pointers for strings by malloc in C](https://stackoverflow.com/questions/15686890/how-to-allocate-array-of-pointers-for-strings-by-malloc-in-c)

[strtok](https://pubs.opengroup.org/onlinepubs/9699919799.2016edition/functions/strtok.html)
