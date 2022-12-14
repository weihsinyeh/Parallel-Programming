//implement with condition variable OK with way1
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <malloc.h>
#include "bmp.h"

using namespace std;
#define NSmooth 1000

BMPHEADER bmpHeader;                        
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                                               
RGBTRIPLE **BMPData = NULL;                                                   

int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory

int counter;          /*Initialize to 0*/
pthread_mutex_t barrier_mutex;      /*Initialize to 1*/
pthread_cond_t cond_var;
typedef struct pass{
        int start;
        int end;
        int threadID;
        int numberOfThread;
        int width;
        int height;
}PASS;
void * child(void *arg){
        PASS *p = (PASS *)arg;
        int start = p->start;
        int end = p->end;
        int threadID = p->threadID;
        int width = p->width;
        int height = p->height;
        for(int count = 0; count < NSmooth ; count ++){
            //change data critical section
            /*Ｂarrier*/
            pthread_mutex_lock(&barrier_mutex);
            counter++;
            if(counter == p->numberOfThread){
                //way1
                swap(BMPSaveData, BMPData);
                counter = 0;
                pthread_cond_broadcast(&cond_var);
            }
            else{
                while(pthread_cond_wait(&cond_var, &barrier_mutex) != 0);
            }
            pthread_mutex_unlock(&barrier_mutex);
                
            if(count == 0){cout<<threadID<<endl;}
			for(int i = start; i<end; i++){
				for(int j =0; j<width ; j++){

					int Top   = i>0 ? i-1 : height-1;
					int Down  = i<height-1 ? i+1 : 0;
					int Left  = j>0 ? j-1 : width-1;
					int Right = j<width-1 ? j+1 : 0;
	
    				BMPSaveData[i][j].rgbBlue =  (double) (BMPData[i][j].rgbBlue+BMPData[Top][j].rgbBlue+BMPData[Top][Left].rgbBlue+BMPData[Top][Right].rgbBlue+BMPData[Down][j].rgbBlue+BMPData[Down][Left].rgbBlue+BMPData[Down][Right].rgbBlue+BMPData[i][Left].rgbBlue+BMPData[i][Right].rgbBlue)/9+0.5;
	    			BMPSaveData[i][j].rgbGreen =  (double) (BMPData[i][j].rgbGreen+BMPData[Top][j].rgbGreen+BMPData[Top][Left].rgbGreen+BMPData[Top][Right].rgbGreen+BMPData[Down][j].rgbGreen+BMPData[Down][Left].rgbGreen+BMPData[Down][Right].rgbGreen+BMPData[i][Left].rgbGreen+BMPData[i][Right].rgbGreen)/9+0.5;
		    		BMPSaveData[i][j].rgbRed =  (double) (BMPData[i][j].rgbRed+BMPData[Top][j].rgbRed+BMPData[Top][Left].rgbRed+BMPData[Top][Right].rgbRed+BMPData[Down][j].rgbRed+BMPData[Down][Left].rgbRed+BMPData[Down][Right].rgbRed+BMPData[i][Left].rgbRed+BMPData[i][Right].rgbRed)/9+0.5;
				}
            }
            //way2
            /*
            for(int i = start; i<end; i++){
                for(int j =0; j<width ; j++){
                    BMPData[i][j] = BMPSaveData[i][j];
                }
            }
            */  
	    }
        pthread_exit(NULL);
}
int main(int argc,char *argv[])
{
	char *infileName = "input.bmp";
        char *outfileName = "outputparallel20.bmp";
	double startwtime = 0.0, endwtime=0;
        struct timespec starttime,endtime;

        if ( readBMP( infileName) )
                cout << "Read file successfully!!" << endl;
        else 
                cout << "Read file fails!!" << endl;

        clock_gettime(CLOCK_MONOTONIC, &starttime);
        BMPData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        for(int i=0;i<bmpInfo.biHeight;i++){
                for(int j=0;j<bmpInfo.biWidth;j++){
                        BMPData[i][j] = BMPSaveData[i][j];
                }
        }
        int nthreads;
        printf("input the number of threads:");
        scanf("%d",&nthreads);
        int * displacement = (int *)malloc(sizeof(int)*nthreads);
        int * count = (int *)malloc(sizeof(int)*nthreads);
        int remainder = bmpInfo.biHeight%nthreads;
        int quotient = bmpInfo.biHeight/nthreads;
        int sum = 0;
        for(int i = 0; i < nthreads; i++){
                count[i] = quotient;
                if(remainder > 0){
                        count[i] += 1;
                        remainder--;
                }
                displacement[i] = sum;
                sum += count[i];
        }
        pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t)*nthreads);

        
        counter = 0;
        for(int i=0;i<nthreads;i++){
                PASS *p = (PASS *)malloc(sizeof(PASS));
                p->start = displacement[i];
                p->end = displacement[i]+count[i];
                p->threadID = i;
                p->numberOfThread = nthreads;
                p->width = bmpInfo.biWidth;
                p->height = bmpInfo.biHeight;
                pthread_create(&thread[i],NULL,child,(void *)p);
        }
        for(int i=0;i<nthreads;i++){
                pthread_join(thread[i],NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &endtime);
        
        if ( saveBMP( outfileName ) )
                cout << "Save file successfully!!" << endl;
        else
                cout << "Save file fails!!" << endl;
        cout<<"time:"<<endtime.tv_sec-starttime.tv_sec+(endtime.tv_nsec-starttime.tv_nsec)/1000000000.0<<endl;
	free(BMPData[0]);
 	free(BMPSaveData[0]);
 	free(BMPData);
 	free(BMPSaveData);

    return 0;
}

int readBMP(char *fileName)
{
        ifstream bmpFile( fileName, ios::in | ios::binary );
 
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }
 
   
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
 
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
 
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
        
        if ( bmpInfo.biBitCount != 24 ){
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        while( bmpInfo.biWidth % 4 != 0 )
        	bmpInfo.biWidth++;

        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);
	
        bmpFile.close();
 
        return 1;
 
}

int saveBMP( char *fileName)
{
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
        
        ofstream newFile( fileName,  ios:: out | ios::binary );
 
        if ( !newFile ){
                cout << "The File can't create!!" << endl;
                return 0;
        }
 	
        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

        newFile.close();
 
        return 1;
 
}

RGBTRIPLE **alloc_memory(int Y, int X )
{        
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	    RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

        for( int i = 0; i < Y; i++){
                temp[ i ] = &temp2[i*X];
        }
 
        return temp;
 
}

void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}

