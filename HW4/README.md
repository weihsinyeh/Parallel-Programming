# 多處理機平行程式設計HW4
###### tags: `多處理機平行程式設計`
## busy-waiting and a mutex
Implement a barrier using busy-waiting and a mutex
共享變數 : counter (protected by the mutex)
When the counter indicates that every thread has entered the critical section, thread can leave the critical section.
```c=
Busy-Waiting and a Mutex
/* shared and initialized by the main thread*/
int counter; //初始化為0
int thread_count;
pthread_mutex_t barrier_mutex
void Thread_work(...){
    ...
    /*Barrier */
    pthread_mutex_lock(&barrier_mutex)
    counter++;
    pthread_mutex_unlock(&barrier_mutex);
    while(counter<thread_count); //bust waiting
}
```
如果又都使用同個counter會有問題，所以counter 重複使用。
Thus we need one counter variable for each instance of the barrier, otherwise problems are likely to occur.
```c=
    counter[0] = 0;
    counter[1] = 0;
```
```c=
for(int count = 0; count < NSmooth ; count ++){
                //change data critical section
                /*Ｂarrier*/
                pthread_mutex_lock(&barrier_mutex);
                if(counter[count%2] == p->numberOfThread-1){
                        /**way1*********************************/ 
                        swap(BMPSaveData, BMPData);
                        counter[(count+1)%2] = 0;
                }
                counter[count%2]++;
                pthread_mutex_unlock(&barrier_mutex);
                while(counter[count%2]<p->numberOfThread);
    ...
```
## Semaphore
共享變數
```c=
int counter;       //初始化為0
sem_t count_sem;   //初始化為1 unlock
sem_t barrier_sem; //初始化為0 lock
```
```c=
void Thread_work(){
    sem_wait(&count_sem);
    if(counter == thread_count - 1){ //last thread
        counter = 0;
        sem_post(&count_sem); //下次要重複使用釋放掉
        for(int j = 0;j < thread_count - 1;j++){
            sem_post(&barrier_sem); 
            //如果重複使用這個barrier_sem 可能又因為
            //有的thread跑太快，再重新進入這裡，導致錯誤
        }
    }
    else{
        counter ++ ;
        sem_post(&count_sem);
        sem_wait(&barrier_sem);
    }
}
```
counter 會被跑太快的去改變，用barrier_sem
barrier_sem 又會被跑太快的thread去改變，
所以用交替的方法，多個barrier_sem交替使用
```c=
int counter;          /*Initialize to 0*/
sem_t count_sem;      /*Initialize to 1*/
sem_t barrier_sem[2];    /*Initialize to 0*/
```
```c=
    for(int count = 0; count < NSmooth ; count++){
        sem_wait(&count_sem);
        if(count == thread_count - 1){
            counter = 0;
            sem_post(&count_sem);
            for(int j = 0;j < thread_count;j++){
                sem_post(&barrier_sem[count%2]);
            }
        }
        else{
            sem_post(&count_sem);
            sem_wait(&barrier_sem[count%2]);
        }
    }
```
## Condition Varible
* A condition variable is a data object that allows a thread to suspend execution until a certain event or condition occurs.
* When the event or condition occurs another thread can signal the thread to "wake up".
* A condition variable is always associated with a mutex.

虛擬碼
```c=
lock mutex;
if( condition has occured ){ //看condition有沒有發生，如果是最後一個thread
    signal thread(s);
}
else{
    unlock the mutex and block;
    /* when thread is unblocked, mutex is relocked */
}
unlcok mutex;
```

condition variables in Pthreads have type ==pthread_cond_t==. The function 
```c=
int pthread_cond_signal(pthread_cond_t * cond_var_p /* in/out */ );
```
will unlock one of the thread 一次釋放一個

```c=
int pthread_cond_broadcast(pthread_cond_t * cond_var_p /* in/out */);
```
一次釋放多個

休眠
```c=
int pthread_cond_wait(
    pthread_cond_t * cond_var_p  /* in/out */,
    pthread_mutex_t * mutex_p    /* in/out */);
```
等於 這三個指令
```c=
pthread_mutex_unlock(&mutex_p);
wait_on_signal(&cond_var_p);      // 等待wake
pthread_mutex_lock(&mutex_p);     // mutex lock
```
Implementing a barrier with condition variables
不會有重覆使用lock的問題
```c=
/* shared */
int counter = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_var;
...
void * Thread_work(...){
    /* Barrier */
    pthread_mutex_lock(&mutex);             // BBBBBBB
    counter++;
    if(counter == thread_count){
        pthread_cond_broadcast(&cond_var);
    }
    else{
        //為了怕她不小心醒果來，所以用while
        while(pthread_cond_wait(&cond_var,&mutex)!=0); 
        //pthread_mutex_unlock(&mutex_p);   // BBBBBBB
        //wait_on_signal(&cond_var_p);      
        //pthread_mutex_lock(&mutex_p);     // AAAAAAA
    }
    pthread_mutex_unlock(&mutex);           // AAAAAAA
} 
```