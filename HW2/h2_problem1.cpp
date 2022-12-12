//F74109016 
#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"
#include <cstddef>
using namespace std;

#define NSmooth 1000

BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = 0;

int readBMP( char *fileName);                    
int saveBMP( char *fileName);                    
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        

int main(int argc,char *argv[]){
    /****** Initialize*********************/
    char *infileName = "input.bmp";
    char *outfileName = "output.bmp";
    double startwtime = 0.0, endwtime=0;
    int size,rank,width,height;
    
    int *offsets           =0;
    int *localNumOfElement =0;
    int *localHeight       =0;
    
    RGBTRIPLE **OLD = NULL;
    RGBTRIPLE **NEW = NULL;
    
    RGBTRIPLE **UP = NULL;
    RGBTRIPLE **DOWN = NULL;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Datatype MY_TRIPLE;
    MPI_Type_contiguous(3, MPI_UNSIGNED_CHAR, &MY_TRIPLE);
    MPI_Type_commit(&MY_TRIPLE);
    
    /****** Initialize*********************/
    
    /****** ReadFile  *********************/
    if(rank == 0){
        if ( readBMP( infileName) )
            cout << "Read file successfully!!" << endl;
        else
            cout << "Read file fails!!" << endl;  
        width  = bmpInfo.biWidth;
        height = bmpInfo.biHeight;
    }
    /****** ReadFile  *********************/
    
    
    /****** Broadcast information**********/
    MPI_Barrier(MPI_COMM_WORLD);  //Wair for the finish of root processor read file 
    
    if(rank == 0){                //Root Processor's timer start
      startwtime = MPI_Wtime();  
    }
    //Because only processor Root read file, the information of file need to be broadcast.
    MPI_Bcast(&height,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&width ,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    /****** Broadcast information**********/
    
    
    //Caution! althoung other processor don't need BMPSaveData, We still need to alloc memory,
    //otherwise it will terminate at signal 9.
    if(rank != 0){
        BMPSaveData   = alloc_memory(height, width);
    }
    
    //Every processor need to calculate how many element they need to work 
    offsets           = new int[size];     //the distance from the address of BMPSaveData
    localNumOfElement = new int[size];     //the number of element in this block
    localHeight       = new int[size];     //every block's height
    
    int remainder = height % size;
    int sum = 0;
    for(int i = 0;i < size;i++){
        localNumOfElement[i] = (height / size)* width;
        localHeight[i] = height / size;
    
        if(remainder > 0){
            localNumOfElement[i] += width;
            localHeight[i]        += 1;
            remainder--;
        }
        offsets[i] = sum;
        sum += localNumOfElement[i];
    }
    
    //calculate which number of processor every rank's need to receive one-dimension array from 
    //each processor need to receive one-dimension array from upNeighbor to smooth the top array of the block
    //each processor need to receive one-dimension array from downNeighbor to smooth the bottom array of the block
    int upNeighbor   = (rank == 0) ? size - 1 : rank - 1;
    int downNeighbor = (rank == size - 1) ? 0 : rank + 1;
    
    //memory allocated to receive the one-dimension send from its neighbor
    UP   = alloc_memory(1, width);
    DOWN = alloc_memory(1, width);
    
    //memeory allocated to store the middle result of smoothing 
    OLD = alloc_memory(localHeight[rank]+1, width);
    NEW = alloc_memory(localHeight[rank]+1, width);
    
    //Root processor scatter BMPSaveData (original file) to other processer.
    //every processor receive specific block and start smooth cooperatively.
    MPI_Scatterv(*BMPSaveData,                        //const void *sendbuf
                  localNumOfElement,                  //const int *sendcounts
                  offsets,                            //const int *displs
                  MY_TRIPLE,                          //MPI_Datatype sendtype
                  *NEW,                                //void *recvbuf
                  localNumOfElement[rank],            //int recvcount
                  MY_TRIPLE,                          //MPI_Datatype recvtype
                  0,                                   //int root
                  MPI_COMM_WORLD);                      //MPI_Comm comm

    /****************************Smooth**************************************************/
    for(int count = 0; count < NSmooth ; count ++){

        //Change information about the edge of the block with their neighbor
        
        /**example************************************************
          processorA send 1-D array (bottom of block A) to downNeighbor
          processorB receieve 1-D array (bottom of block A) from upNeighbor and store it in UP array 
        **********************************************************/
        
        MPI_Sendrecv(&NEW[localHeight[rank] - 1][0],   //const void *sendbuf
                      width,                           //int sendcount
                      MY_TRIPLE,                       //MPI_Datatype sendtype
                      downNeighbor,                    //int dest
                      1,                               //int sendtag
                      &UP[0][0],                       //void *recvbuf
                      width,                           //int recvcount,           
                      MY_TRIPLE,                       //MPI_Datatype recvtype
                      upNeighbor,                      //int source,          
                      1,                               //int recvtag
                      MPI_COMM_WORLD,                  //MPI_COMM_WORLD MPI_Comm comm
                      MPI_STATUS_IGNORE);              //MPI_Status *status) 
                     
         MPI_Sendrecv(&NEW[0][0],                      
                      width,                           
                      MY_TRIPLE,                        
                      upNeighbor,               
                      1,            
                      &DOWN[0][0],           
                      width,           
                      MY_TRIPLE,   
                      downNeighbor,             
                      1,             
                      MPI_COMM_WORLD,          
                      MPI_STATUS_IGNORE);
        
        //swap NEW and OLD so that we can use OLD to caculate NEW again
        swap(NEW, OLD);
        
        //Smooth
        for(int i = 0; i < localHeight[rank]; i++){
		          for(int j = 0; j < width; j++){
                    int Top   = i > 0 ? i-1 : localHeight[rank]-1;
				            int Down  = i < localHeight[rank] -1 ? i+1 : 0;
				            int Left  = j >1 ? j-1 : width -1;
				            int Right = j < width -1 ? j+1 : 0;
              
                    //Discuss the case of the edge
                    //check this 1D array is the bottom or not.If yes receive 1D array form downNeighbor
                    //check this 1D array is the top or not.If yes receive 1D array form upNeighbor
                    if( i == localHeight[rank] - 1){   //i is the bottom
                        NEW[i][j].rgbBlue  =  (double) (OLD[i][j].   rgbBlue  +
                                                        OLD[Top][j]. rgbBlue  + OLD[Top][Left].rgbBlue   + OLD[Top][Right].rgbBlue   +
                                                        DOWN[0][j].rgbBlue    + DOWN[0][Left].rgbBlue    + DOWN[0][Right].rgbBlue  +
                                                        OLD[i][Left].rgbBlue  + OLD[i][Right].rgbBlue)/9 + 0.5;
                                                        
				                NEW[i][j].rgbGreen =  (double) (OLD[i][j].   rgbGreen + 
                                                        OLD[Top][j]. rgbGreen + OLD[Top][Left].rgbGreen  + OLD[Top][Right].rgbGreen  +
                                                        DOWN[0][j].rgbGreen   + DOWN[0][Left].rgbGreen   + DOWN[0][Right].rgbGreen +
                                                        OLD[i][Left].rgbGreen + OLD[i][Right].rgbGreen)/9+ 0.5;
                                                        
				                NEW[i][j].rgbRed   =  (double) (OLD[i][j].rgbRed      +
                                                        OLD[Top][j].rgbRed    + OLD[Top][Left].rgbRed    + OLD[Top][Right].rgbRed    +
                                                        DOWN[0][j].rgbRed     + DOWN[0][Left].rgbRed     + DOWN[0][Right].rgbRed   +
                                                        OLD[i][Left].rgbRed   + OLD[i][Right].rgbRed)/9  +0.5;
                    }
                    else if( i == 0){                    //i is the top
                        NEW[i][j].rgbBlue  =  (double) (OLD[i][j].   rgbBlue  +
                                                        UP[0][j]. rgbBlue     + UP[0][Left].rgbBlue      + UP[0][Right].rgbBlue   +
                                                        OLD[Down][j].rgbBlue  + OLD[Down][Left].rgbBlue  + OLD[Down][Right].rgbBlue  +
                                                        OLD[i][Left].rgbBlue  + OLD[i][Right].rgbBlue)/9 + 0.5;
                                                        
				                NEW[i][j].rgbGreen =  (double) (OLD[i][j].   rgbGreen + 
                                                        UP[0][j]. rgbGreen    + UP[0][Left].rgbGreen     + UP[0][Right].rgbGreen  +
                                                        OLD[Down][j].rgbGreen + OLD[Down][Left].rgbGreen + OLD[Down][Right].rgbGreen +
                                                        OLD[i][Left].rgbGreen + OLD[i][Right].rgbGreen)/9+ 0.5;
                                                        
				                NEW[i][j].rgbRed   =  (double) (OLD[i][j].rgbRed      +
                                                        UP[0][j].rgbRed       + UP[0][Left].rgbRed       + UP[0][Right].rgbRed    +
                                                        OLD[Down][j].rgbRed   + OLD[Down][Left].rgbRed   + OLD[Down][Right].rgbRed   +
                                                        OLD[i][Left].rgbRed   + OLD[i][Right].rgbRed)/9  +0.5;
                    }
				            else{                               // i is the internal
                        NEW[i][j].rgbBlue  =  (double) (OLD[i][j].   rgbBlue  +
                                                        OLD[Top][j]. rgbBlue  + OLD[Top][Left].rgbBlue   + OLD[Top][Right].rgbBlue   +
                                                        OLD[Down][j].rgbBlue  + OLD[Down][Left].rgbBlue  + OLD[Down][Right].rgbBlue  +
                                                        OLD[i][Left].rgbBlue  + OLD[i][Right].rgbBlue)/9 + 0.5;
                                                        
				                NEW[i][j].rgbGreen =  (double) (OLD[i][j].   rgbGreen + 
                                                        OLD[Top][j]. rgbGreen + OLD[Top][Left].rgbGreen  + OLD[Top][Right].rgbGreen  +
                                                        OLD[Down][j].rgbGreen + OLD[Down][Left].rgbGreen + OLD[Down][Right].rgbGreen +
                                                        OLD[i][Left].rgbGreen + OLD[i][Right].rgbGreen)/9+ 0.5;
                                                        
				                NEW[i][j].rgbRed   =  (double) (OLD[i][j].rgbRed      +
                                                        OLD[Top][j].rgbRed    + OLD[Top][Left].rgbRed    + OLD[Top][Right].rgbRed    +
                                                        OLD[Down][j].rgbRed   + OLD[Down][Left].rgbRed   + OLD[Down][Right].rgbRed   +
                                                        OLD[i][Left].rgbRed   + OLD[i][Right].rgbRed)/9  +0.5;
                    }
               }
         }

    }
    /****************************Smooth**************************************************/
    // processor Root gather every block other processor calculate in the end
    MPI_Gatherv(*NEW,  
                localNumOfElement[rank], 
                MY_TRIPLE,
                *BMPSaveData,
                localNumOfElement, 
                offsets, 
                MY_TRIPLE, 
                0, 
                MPI_COMM_WORLD);
   
    MPI_Barrier(MPI_COMM_WORLD);
  
 	  //write file
    if(rank == 0){
        //terminate the timer and print
        endwtime = MPI_Wtime();  
        cout << "The execution time = "<< endwtime-startwtime <<endl ;
    
        if ( saveBMP( outfileName ) )
            cout << "Save file successfully!!" << endl;
        else
            cout << "Save file fails!!" << endl;
    }
    
    //Release memory
    delete[] BMPSaveData[0];
    delete[] BMPSaveData;
    
    delete[] NEW[0];
    delete[] NEW;
    
    delete[] OLD[0];
    delete[] OLD;
    
    delete[] UP[0];
    delete[] UP;
    
    delete[] DOWN[0];
    delete[] DOWN;
    
    MPI_Type_free(&MY_TRIPLE);
    MPI_Finalize();
    return 0;
}


// delete 2D array memory.
void Delete2DMemory(RGBTRIPLE **memory)
{
  // Construct and initialize pointer
  delete[] memory[0];
  delete[] memory;
}
/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName)
{
	//建立輸入檔案物件
        ifstream bmpFile( fileName, ios::in | ios::binary );

        //檔案無法開啟
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }

        //讀取BMP圖檔的標頭資料
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );

        //判決是否為BMP圖檔
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }

        //讀取BMP的資訊
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );

        //判斷位元深度是否為24 bits
        if ( bmpInfo.biBitCount != 24 ){
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        //修正圖片的寬度為4的倍數
        while( bmpInfo.biWidth % 4 != 0 )
        	bmpInfo.biWidth++;

        //動態分配記憶體
        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);

        //讀取像素資料
    	//for(int i = 0; i < bmpInfo.biHeight; i++)
        //	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	    bmpFile.read( reinterpret_cast<char*>(BMPSaveData[0]), bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);

        //關閉檔案
        bmpFile.close();

        return 1;

}
/*********************************************************/
/* 儲存圖檔                                              */
/*********************************************************/
int saveBMP( char *fileName)
{
 	//判決是否為BMP圖檔
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }

 	//建立輸出檔案物件
        ofstream newFile( fileName,  ios:: out | ios::binary );

        //檔案無法建立
        if ( !newFile ){
                cout << "The File can't create!!" << endl;
                return 0;
        }

        //寫入BMP圖檔的標頭資料
        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

	//寫入BMP的資訊
        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

        //寫入像素資料
        //for( int i = 0; i < bmpInfo.biHeight; i++ )
        //        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
        newFile.write( reinterpret_cast<char*>(BMPSaveData[0]), bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

        //寫入檔案
        newFile.close();

        return 1;

}


/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X )
{
	//建立長度為Y的指標陣列
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	    RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	//對每個指標陣列裡的指標宣告一個長度為X的陣列
        for( int i = 0; i < Y; i++){
                temp[ i ] = &temp2[i*X];
        }

        return temp;

}
/*********************************************************/
/* 交換二個指標                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b){
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}
