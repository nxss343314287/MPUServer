/*******************************************************************
filename      ats_co_convert.h
author        root
created date  2012-1-9
description   specifications of implemented functions
warning       				
modify history
    author        date        modify        description
*******************************************************************/

#ifndef ATS_CO_CONVERT_HH_
#define ATS_CO_CONVERT_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include "../../public.h"


#ifdef  __cplusplus
        extern "C" {
#endif



//!!! don't alter below definition
#define LOW_ENDIAN       1   //0��Ч    //1��Ч
#define HIGH_ENDIAN      0   //0��Ч    //1��Ч

#define true            0
#define false           -1
#define TRUE            1
#define FALSE           0

//�ֽ���ת������
typedef enum _ConvertByteType
{
        CHAR2BYTE = 1, //char converse byte
        SHT2BYTE = 2, //short converse byte
        THR2BYTE = 3, //three bytes converse
        INT2BYTE = 4, //int converse byte
        LONGLONG2BYTE = 8,//long long converse byte
}ConvertByteType;

typedef unsigned char BYTE;
//typedef unsigned char byte;
typedef unsigned short u_int16;
typedef unsigned int u_int32;
typedef unsigned long long u_int64;
//typedef unsigned int u_int64;


u_int64 accumulate(int num, int times);
int setBitsOfByte(BYTE *bye, int posii, int width, int value);
int countValueofBits(BYTE b, int pos, int width);
int getBytesOfValue(BYTE * buffer, u_int64 value, int len);
int reverseBytes(const BYTE *src, BYTE *des, int len);
u_int64 countValueofBytes(const BYTE * buffer, int len);
u_int64 int2bytes(BYTE bye[], const int mode, u_int64 num);
u_int64 bytes2int(const BYTE bye[], int mode);





#ifdef  __cplusplus
        }
#endif



#endif /* ATS_CO_CONVERT_HH_ */
