/*
 * mpusendconvert.h
 *
 *  Created on: 2012-6-22
 *      Author: root
 */
#ifndef MPUSENDCONVERT_H_
#define MPUSENDCONVERT_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "../../log.h"
#include "public.h"

//#include "com/ats_public.h"
//#include "com/ats_common.h"

//#define  NET2IO                     "tmp/net2io.fifo"
//#define  IO2NET                     "tmp/io2net.fifo"
//#define  APP2IO                     "tmp/app2io.fifo"
//#define  IO2APP                     "tmp/io2app.fifo"

#define APP2NET						"/fifo/app2net"
#define NET2APP						"/fifo/net2app"
#define APP2TSNET                   "/fifo/app2jnet"

#define BUFF_COMM               2400            /** buffer length for each layer */
#define  NET_PACKET_LEN          BUFF_COMM

/* Return Value */
#define  RT_ERR                 -1
#define  RT_OK                  0

enum _machine_no_list
{
	CAS = 150, CFEP, LFEP, SSI
};

enum _date_type
{
	INSIDE = 1, OUTSIDE = 2,
};

enum _ats_net_protocol_type
{
	PROTOCOL_HEART_BEAT = 1, /** 1:heartbeat */
	PROTOCOL_TCP, /** 2:tcp */
	PROTOCOL_UDP, /** 3:udp */
	PROTOCOL_ACK, /** 4:ack */
	PROTOCOL_ASK_SEQ, /** 5:ask for the sequence */
	PROTOCOL_HEARTBEAT_DISCONNECT, /** 6:heartbeat disconnect */
	PROTOCOL_HEARTBEAT_CONNECT,
	PROTOCOL_MULTICAST,
/** 7:heartbeat connect */
};

//typedef unsigned char u8;
//typedef unsigned short int u16;
//typedef unsigned int u32;
//typedef unsigned long long int u64;
//typedef unsigned long long int NetTime;

/*-- Global typedefs --*/
//typedef unsigned char BYTE; /**< @brief  Type of 1-byte.  */
//typedef unsigned short int WORD; /**< @brief  Type of 2-byte  */
//typedef unsigned long int DWORD; /**< @brief  Type of 4-byte  */

//typedef unsigned int UInt; /**< @brief  Platform related interger type  */
//typedef unsigned short int UShort; /**< @brief  Type of 2-byte  */
//typedef unsigned long int ULong; /**< @brief  Type of 4-byte  */

//typedef struct _message_header
//{
//	unsigned long long int timestamp; /** the timestamp of sending the message */
//	unsigned int seqno; /** sequence no */
//	unsigned char srcno; /** source ip no */
//	unsigned char destno; /** destination ip no */
//	unsigned char srcmodule; /** source module no */
//	unsigned char destmodule; /** destination module no */
//	unsigned short datatype; /** type of the data */
//	unsigned char protocoltype; /** type of the protocol */
//	unsigned char mainorbak; /** server flag */
//	unsigned short remain;
//	unsigned short datalen; /** length of the data */
//} message_header_t;

typedef struct _message_header_byte
{
	unsigned char timestamp_one; /** the timestamp of sending the message */
	unsigned char timestamp_two;
	unsigned char timestamp_three;
	unsigned char timestamp_four;
	unsigned char timestamp_five;
	unsigned char timestamp_six;
	unsigned char timestamp_seven;
	unsigned char timestamp_eight;
	unsigned char seqno_one; /** sequence no */
	unsigned char seqno_two;
	unsigned char seqno_three;
	unsigned char seqno_four;
	unsigned char srcno; /** source ip no */
	unsigned char destno; /** destination ip no */
	unsigned char srcmodule; /** source module no */
	unsigned char destmodule; /** destination module no */
	unsigned char datatype_one; /** type of the data */
	unsigned char datatype_two;
	unsigned char protocoltype; /** type of the protocol */
	unsigned char mainorbak; /** server flag */
	unsigned char remain_one;
	unsigned char remain_two;
	unsigned char datalen_one; /** length of the data */
	unsigned char datalen_two;
} message_header_byte;

typedef message_header_t StructNetHead;
//typedef  message_header_byte  NetHead;
typedef message_header_byte NetHead;

typedef struct
{
	unsigned char pdi; /** process data interface */
	unsigned char datatype_low; /** type of the data:low byte */
	unsigned char datatype_high; /** type of the data:high byte */
	unsigned char datalen; /** length of the data */
} AppHeadOut;

typedef struct _APP_HeadIn
{
	unsigned char datatype_low; /** type of the data:low byte */
	unsigned char datatype_high; /** type of the data:high byte */
	unsigned char datalen_one; /** length of the data */
	unsigned char datalen_two; /** length of the data */
	unsigned char datalen_three; /** length of the data */
	unsigned char datalen_four; /** length of the data */
} AppHeadIn;

/**************************************************************//**
 @brief           Conversion struct NetHead to package.
 @param[out]	  pStNetHead : a struct
 @param[in]       destno  : destination number
 @param[in]       datalen: date length
 @param[in]       protocoltype: protocol type
 @return          void
 @detail -#       Conversion struct NetHead to package..
 *******************************************************************/
void adapter_net_head_package(NetHead *pStNetHead, unsigned char destno,
		unsigned short datatype, unsigned short datalen, unsigned char protocoltype);

/**************************************************************//**
 @brief           Conversion struct AppHeadIn to package.
 @param[out]	  AppHeadIn : a struct
 @param[in]       datetype  : date type
 @param[in]       datalen: date length
 @param[in]       protocoltype: protocol type
 @return          void
 @detail -#       Conversion struct AppHeadIn to package..
 *******************************************************************/
void adapter_app_head_package(AppHeadIn* pAppHeadIn, unsigned short datetype,
		unsigned int datelen);
void adapter_sys_err(void);
ssize_t adapter_writen(int fd, const void *vptr, size_t n);
void printhex(const char* msg, int msglen);
unsigned char short_to_byte(unsigned short ssrc, int num);
unsigned char int_to_byte(unsigned int isrc, int num);
char int_to_byte_sign(int isrc, int num);
unsigned char long_long_to_byte(unsigned long long llsrc, int num);
unsigned char extract(int isrc, int num);
extern int app2netfd;

#endif
