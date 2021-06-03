/*
 * mpusendconversion.c
 *
 *  Created on: 2012-6-22
 *      Author: root
 */
#include "include/MPUSendConvert.h"

#define LITTLE 1
#define BIG 0
/**************************************************************//**
 @brief           get a char from a unsigned short number
 @param[in]       ssrc:  a short int number
 @param[in]       num :  char array number
 @return          unsigned char
 @details -#
 *******************************************************************/
unsigned char short_to_byte(unsigned short ssrc, int num)
{
	assert(0 < num && num <= sizeof(short));
	unsigned char cret = 0;
	cret = ssrc >> (8 * --num) & 0xFF;
	return cret;
}

/**************************************************************//**
 @brief           get a char from a unsigned int number
 @param[in]       isrc:  a int number
 @param[in]       num :  char array number
 @return          unsigned char
 @details -#
 *******************************************************************/
unsigned char int_to_byte(unsigned int isrc, int num)
{
	assert(0 < num && num <= sizeof(int));
	unsigned char cret = 0;
	cret = isrc >> (8 * --num) & 0xFF;
	return cret;
}

char int_to_byte_sign(int isrc, int num)
{
	assert(0 < num && num <= sizeof(int));
	char cret = 0;
	cret = isrc >> (8 * --num) & 0xFF;
	return cret;
}

/**************************************************************//**
 @brief           get a char from a unsigned long long int number
 @param[in]       llsrc:  a long long int number
 @param[in]       num  :  char array number
 @return          unsigned char
 @details -#
 *******************************************************************/
unsigned char long_long_to_byte(unsigned long long llsrc, int num)
{
	assert(0 < num && num <= sizeof(long long));
	unsigned char cret = 0;
	cret = llsrc >> (8 * --num) & 0xFF;
	return cret;
}

/**************************************************************//**
 @brief           get errno
 @param[in]       void
 @return          void
 *******************************************************************/
void adapter_sys_err(void)
{
	int errornum = errno;
	ELOG("errno = %d, strerror(errno)= %s", errornum, strerror(errornum));
}

/**************************************************************//**
 @brief           Write "n" bytes to a descriptor.
 @param[in]       fd  : a descriptor
 @param[in]       vptr: buffer
 @param[in]       n   : the number of bytes
 @return          ssize_t
 @detail -#       Write "n" bytes to a descriptor.
 *******************************************************************/
ssize_t adapter_writen(int fd, const void *vptr, size_t n)
{
	if (NULL == vptr)
	{
		ELOG("func adapter_writen const void *vptr is NULL");
		return RT_ERR;
	}
	const char *ptr = NULL;
	int nleft, nwritten;

	if (RT_OK >= fd)
	{
		printf("File description is invalid.fd[%d]\n", fd);
		ELOG("File description is invalid.fd[%d]", fd);
		return RT_ERR;
	}
//	printf("enter adapter_writen. n[%d]\n", n);
//	DLOG("enter adapter_writen. n[%d]", n);

	ptr = vptr;
	nleft = n;

	while (nleft > 0)
	{
		//DLOG("enter while (nleft > 0) . nleft[%d], fd[%d]", nleft, fd);

		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			ELOG("nwritten[%d]", nwritten);
			adapter_sys_err();

			if (nwritten < 0 && errno == EINTR)
			{
				//DLOG("errno == EINTR, nwritten[%d]", nwritten);
				nwritten = 0; /** and call write() again */
			}
			else
			{
				//DLOG("nwritten < 0, nwritten[%d]", nwritten);
				return RT_ERR; /** error */
			}
		}
		DLOG("nwritten[%d]", nwritten);
		nleft -= nwritten;
		ptr += nwritten;

		//DLOG("nwritten[%d], nleft[%d], fd[%d]", nwritten, nleft, fd);
	}//DLOG("n[%d]", n);
//	printhex(vptr, n);
	//DLOG("finish 1");
	return (n);
}

/**************************************************************//**
 @brief           print logs to a log file
 @param[in]       msg        print char array messages in hex
 @return          void
 *******************************************************************/
void printhex(const char* msg, int msglen)
{
	if (NULL == msg)
	{
		ELOG("func printhex const char* msg is NULL");
		return;
	}
	unsigned char msgtemp[NET_PACKET_LEN * 4] ={	'\0'};

	if(	BUFF_COMM < msglen)
	{
		ELOG("(BUFF_SOCK < msglen), msglen[%d]", msglen);
	}
	else
	{
		unsigned char* p1 = msgtemp;
		unsigned char* p = (unsigned char*)msg;
		int i = 0;
		for (;i < msglen; i++) /** convert the message */
		{
			if (0 == i % 16)
			{
				sprintf((unsigned char*)p1, "\n%-4d :  ", i);
				p1 += 8;
			}
			sprintf((unsigned char*)p1, "%02x ", *p);
			p1 += 3;
			p++;
		}
		ILOG("[%d] -- %s: %d: %s:%s\n", getpid(), __FILE__, __LINE__, __FUNCTION__, msgtemp);
	}
}


void adapter_net_head_package(NetHead *pStNetHead, unsigned char destno,
		unsigned short datatype, unsigned short datalen, unsigned char protocoltype)
{
	assert(NULL != pStNetHead);
	pStNetHead->srcno = CAS;
	pStNetHead->datatype_one = int_to_byte(datatype, 1);
	pStNetHead->datatype_two = int_to_byte(datatype, 2);
	pStNetHead->protocoltype = protocoltype;
	pStNetHead->destno = destno;
	pStNetHead->datalen_one = short_to_byte(datalen, 1);
	pStNetHead->datalen_two = short_to_byte(datalen, 2);
	return;
}

void adapter_app_head_package(AppHeadIn* pAppHeadIn, unsigned short datetype,
		unsigned int datelen)
{
	assert(NULL != pAppHeadIn);
	pAppHeadIn->datatype_low = short_to_byte(datetype, 1);
	pAppHeadIn->datatype_high = short_to_byte(datetype, 2);
	pAppHeadIn->datalen_one = int_to_byte(datelen, 1);
	pAppHeadIn->datalen_two = int_to_byte(datelen, 2);
	pAppHeadIn->datalen_three = int_to_byte(datelen, 3);
	pAppHeadIn->datalen_four = int_to_byte(datelen, 4);
	return;
}



typedef union
{
	unsigned short sh;
	char ch;
} Endian;

int checkendian(void)
{
	Endian endian;
	endian.sh = 1;
	if (endian.ch == 1)
	{
		return LITTLE;
	}
	return BIG;
}

/* 截取变量中右起第N个byte */
unsigned char extract(int isrc, int num)
{
	assert(0 < num);

	if (checkendian())
	{
		isrc = isrc >> (8 * --num) & 0xFF;
	}
	else
	{
		isrc = (isrc >> (8 * (sizeof(int) - num))) & 0xFF;
	}
	return isrc;
}
