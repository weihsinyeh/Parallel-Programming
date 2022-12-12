#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//定義平滑運算的次數
#define NSmooth 1000

/*********************************************************/
/*變數宣告：                                             */
/*  bmpHeader    ： BMP檔的標頭                          */
/*  bmpInfo      ： BMP檔的資訊                          */
/*  **BMPSaveData： 儲存要被寫入的像素資料               */
/*  **BMPData    ： 暫時儲存要被寫入的像素資料           */
/*********************************************************/
BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE **BMPData = NULL;
RGBTRIPLE **BMPRecv = NULL;
/*********************************************************/
/*函數宣告：                                             */
/*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData*/
/*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入  */
/*  swap       ： 交換二個指標                           */
/*  **alloc_memory： 動態分配一個Y * X矩陣               */
/*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory

int main(int argc,char *argv[])
{
/*********************************************************/
/*變數宣告：                                             */
/*  *infileName  ： 讀取檔名                             */
/*  *outfileName ： 寫入檔名                             */
/*  startwtime   ： 記錄開始時間                         */
/*  endwtime     ： 記錄結束時間                         */
/*********************************************************/
  char *infileName = "input.bmp";
  char *outfileName = "output.bmp";
  
  double startwtime = 0.0, endwtime=0;
  int size,rank;
  
  MPI_Status status;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  //讀取檔案
  if ( readBMP( infileName) )
        cout << "Read file successfully!!" << endl;
  else
        cout << "Read file fails!!" << endl;
	  
  //記錄開始時間
  startwtime = MPI_Wtime();  
  int local_counts[size],offsets[size],width;

  
  //compute the work distribution
  width = bmpInfo.biWidth;
  int remainder = bmpInfo.biHeight % size;
  int sum = 0;
  for(int i = 0;i < size;i++){
    local_counts[i] = bmpInfo.biHeight / size;
    if(remainder > 0){
      local_counts[i] += 1;
      remainder--;
    }
    offsets[i] = sum;
    sum += local_counts[i];
  }
  
  int lrows = local_counts[rank];
  int lcols = width;
  
  //動態分配記憶體給暫存空間
  BMPRecv = alloc_memory(offsets[rank],width);
  BMPData = alloc_memory(offsets[rank], width);
  MPI_Scatterv(BMPSaveData, local_counts, offsets, MPI_BYTE, BMPRecv,local_counts[rank]*width*3, MPI_BYTE, 0, MPI_COMM_WORLD);

  RGBTRIPLE **_old = alloc_memory( lrows+2, lcols+2);
  RGBTRIPLE **_new = alloc_memory( lrows+2, lcols+2);
  RGBTRIPLE **_buf = alloc_memory( lrows  , lcols);

  int next = rank+1;
  int prev = rank-1;

  if(next >= size)
    next = 0;
  if(prev < 0)
    prev = size-1;

  for(int i = 0;i < lrows + 2;i++){
    for(int j = 0;j < lcols + 2;j++){
        _old[i][j].rgbBlue  = 0;
        _old[i][j].rgbGreen = 0;
        _old[i][j].rgbRed   = 0;
    }
  }

  for(int i = 1;i < lrows + 1;i++){
    for(int j = 1;j < lcols + 1;j++)
        _old[i][j].rgbBlue  = BMPSaveData[i][j].rgbBlue;
        _old[i][j].rgbGreen = BMPSaveData[i][j].rgbGreen;
        _old[i][j].rgbRed   = BMPSaveData[i][j].rgbRed;
  }

  //進行多次的平滑運算
	for(int count = 0; count < NSmooth ; count ++){
		//把像素資料與暫存指標做交換
		swap(BMPRecv,BMPData);
		//進行平滑運算
        for(int i = 0; i < lrows + 2;i++){
            for(int j = 0;j > lcols + 2;j++){
                _new[i][j].rgbBlue = 0;
                _new[i][j].rgbGreen = 0;
                _new[i][j].rgbRed = 0;
            }
        }
		for(int i = =0; i< lrows + 1 ; i++){
			for(int j =0; j< lcols + 1; j++){
				/*********************************************************/
				/*設定上下左右像素的位置                                 */
				/*********************************************************/
				int Top = i>0 ? i-1 : bmpInfo.biHeight-1;
				int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
				int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
				int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
				/*********************************************************/
				/*與上下左右像素做平均，並四捨五入                       */
				/*********************************************************/
				_new[i][j].rgbBlue  =  (double) ( _old[i][j].rgbBlue     +
                                            _old[Top][j].rgbBlue   + _old[Top][Left].rgbBlue   + _old[Top][Right].rgbBlue   +
                                            _old[Down][j].rgbBlue  + _old[Down][Left].rgbBlue  + _old[Down][Right].rgbBlue  +
                                            _old[i][Left].rgbBlue  + _old[i][Right].rgbBlue)/9 + 0.5;
				_new[i][j].rgbGreen =  (double) (_old[i][j].rgbGreen    + 
                                            _old[Top][j].rgbGreen  + _old[Top][Left].rgbGreen  + _old[Top][Right].rgbGreen  +
                                            _old[Down][j].rgbGreen + _old[Down][Left].rgbGreen + _old[Down][Right].rgbGreen +
                                            _old[i][Left].rgbGreen + _old[i][Right].rgbGreen)/9+ 0.5;
				_new[i][j].rgbRed   =  (double) (  _old[i][j].rgbRed      +
                                            _old[Top][j].rgbRed    + _old[Top][Left].rgbRed    + _old[Top][Right].rgbRed    +
                                            _old[Down][j].rgbRed   + _old[Down][Left].rgbRed   + _old[Down][Right].rgbRed   +
                                            _old[i][Left].rgbRed   + _old[i][Right].rgbRed)/9  +0.5;
			}
        }
        for(int i = 2;i < lrows; i++){
            for(int j = 2;j < lcols;j++){
                _old[i][j].rgbBlue  = _new[i][j].rgbBlue;
                _old[i][j].rgbGreen = _new[i][j].rgbGreen;
                _old[i][j].rgbRed   = _new[i][j].rgbRed;
            }
        }
        MPI_Sendrecv(&_old[lrows][1],lcols*3,MPI_BYTE,next,1,      &_old[0][1],lcols*3,MPI_BYTE,prev,1,MPI_COMM_WORLD,&status);
        MPI_Sendrecv(&_old[1][1]    ,lcols*3,MPI_BYTE,prev,2,&_old[lrows+1][1],lcols*3,MPI_BYTE,next,2,MPI_COMM_WORLD,&status);
	}
    int cnter = 0;
    for(int i = 1; i < lrows + 1;i++){
        for(int j = 1;j < lcols + 1;j++){
            buf[i-1][j-1].rgbBlue = _old[i][j].rgbBlue;
            buf[i-1][j-1].rgbGreen = _old[i][j].rgbGreen;
            buf[i-1][j-1].rgbRed = _old[i][j].rgbRed;
            cnter++;
        }
    }
    MPI_Gatherv(&buf[0][0], cnter*3 , MPI_BYTE,&buffer[0][0], cnter*3, offsets, MPI_BYTE, 0, MPI_COMM_WORLD);
  
  
  
  
  
 	//寫入檔案
  
  if ( saveBMP( outfileName ) )
        cout << "Save file successfully!!" << endl;
  else
        cout << "Save file fails!!" << endl;
  
	//得到結束時間，並印出執行時間
  endwtime = MPI_Wtime();
  cout << "The execution time = "<< endwtime-startwtime <<endl ;

	free(BMPData[0]);
 	free(BMPSaveData[0]);
 	free(BMPData);
 	free(BMPSaveData);
  MPI_Type_free(&datatype);
 	MPI_Finalize();

    return 0;
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
	    bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);

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
        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

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
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}
