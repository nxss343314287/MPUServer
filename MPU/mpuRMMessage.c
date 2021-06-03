/*******************************************************************
filename      mpuRMMessage.c
author        root
created date  2012-2-29
description   specifications of implemented RM interface
warning
modify history
    author        date        modify        description
*******************************************************************/
#include "unistd.h"
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/types.h>
#include	<fcntl.h>
#include 	"../log.h"
#include 	"include/MPU.h"
#include 	"include/MPURMMessage.h"

//extern int fifo_init(char *szFIFOName);

#define MESSAGE_TYPE_NUMBER 5
#define MESSAGE_ATTRIBUTER_NUMBER 3

const char *  LOG_NAME_MPUMS=	"MPU.RMMessage";

const char *ConversionConfig[MESSAGE_TYPE_NUMBER][MESSAGE_ATTRIBUTER_NUMBER] =
  {
    { "1", "2", "2" },
    { "2", "3", "2", },
    { "3", "4", "2", },
    { "4", "5", "2", },
    { "5", "6", "2", },
  };

/*******************************************************************
 Function name		ats_co_travelConversionConfig
 description			search configuration of message using message
 type and element type
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_mpu_travelConversionConfig(ATS_CO_MSG *handle)
{
//if we have set message_tpye and element_tpye value ,
//then we can travel configuration list to get index value and configuration of functions
  int i, re;

  for (i = 0; i < MESSAGE_TYPE_NUMBER; i++)
  {
	  re = atoi(ConversionConfig[i][1]);
	  if (re != 0)
	  {
		  if (((atoi(ConversionConfig[i][1]) == handle->config.message_type))
		     && ((atoi(ConversionConfig[i][2])== handle->config.element_type)))
		  {
			  handle->config.index = atoi(ConversionConfig[i][0]);
			  break;
		  }

	  }

  }
//  for (i = 0; i < MESSAGE_TYPE_NUMBER; i++)
//    {
//    for (j = 1; j < MESSAGE_ATTRIBUTER_NUMBER; j++)
//      {
//
//      re = atoi(ConversionConfig[i][j]);
//      if (re != 0)
//        if (((atoi(ConversionConfig[i][j]) == handle->config.message_type))
//            && ((atoi(ConversionConfig[i][j + 1])
//                == handle->config.element_type)))
//          {
////            ILOG( "<RM_MPU_MSG>packet type:message_tpye=%s,element_tpye=%s",
////                ConversionConfig[i][j], ConversionConfig[i][j+1]);
//            handle->config.index = atoi(ConversionConfig[i][j - 1]);
////            strcpy(handle->config.bufferConverToMsgFuncName,
////                ConversionConfig[i][j + 2]);
////            strcpy(handle->config.msgConverToBufferFuncName,
////                ConversionConfig[i][j + 3]);
////            strcpy(handle->config.ats_co_setDBFuncName,
////                ConversionConfig[i][j + 4]);
////            strcpy(handle->config.ats_co_getDBFuncName,
////                ConversionConfig[i][j + 5]);
//            continue;
//          }
//    }
//}

 //to check parse is correct
   if (handle->config.index <= 0)
   {
      ELOG( "<MPU-RM>Pattern mode(message_type=%d,element_type=%d) is not exist",
      handle->config.message_type, handle->config.element_type);
      return false;
   }
   return true;
}


/*******************************************************************
 Function name		ats_mpu_initATSRMMessage
 description			initialize data structure of message handle
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_mpu_rm_initATSCOMessage(ATS_CO_MSG *handle)
{


	//for packet of ATS RM
	handle->config.message_type =countValueofBits(handle->format[0],0,5);//message_type:msgid
	handle->config.element_type =countValueofBits(handle->format[0],5,3);//element_type:destid

	DLOG("destid = %d, msgid = %d",handle->config.element_type,handle->config.message_type);
	//get index value and names of all functions
	if (ats_mpu_travelConversionConfig(handle) == false)
	{

		return false;
	}


//to set addresses of functions and message body
//here we should do every things of initialization.
switch (handle->config.index)
{
case 1: //RM_CO_LOCK_REQ--RM_MPU_CHANGING_LOCK
  handle->message = malloc(sizeof(RM_MPU_LOCK_REQ));
  memset(handle->message, 0, sizeof(RM_MPU_LOCK_REQ));
  handle->bufferConvertToMsgFunc = (void*) RM_MPU_LOCK_REQ_ConvertFromBuffer;
  handle->msgConvertToBufferFunc = (void *) RM_MPU_LOCK_REQ_ConvertToBuffer;
  handle->logicalMsgFunc=(void*)rm_mpu_lock_req;
break;
case 2: //RM_CO_UNLOCK_IND--RM_MPU_CHANGING_UNLOCK
  handle->message = malloc(sizeof(RM_MPU_UNLOCK_IND));
  memset(handle->message, 0, sizeof(RM_MPU_UNLOCK_IND));
  handle->bufferConvertToMsgFunc = (void *) RM_MPU_UNLOCK_IND_ConvertFromBuffer;
  handle->msgConvertToBufferFunc = (void *) RM_MPU_UNLOCK_IND_ConvertToBuffer;
  handle->logicalMsgFunc=(void*)rm_mpu_unlock_ind;
break;
case 3: // RM_MPU_FRAME_SYN_REQ
		handle->message = malloc(sizeof(RM_MPU_FRAME_SYN_REQ));
		memset(handle->message, 0, sizeof(RM_MPU_FRAME_SYN_REQ));
		handle->bufferConvertToMsgFunc = (void *) RM_MPU_FRAME_SYN_REQ_ConvertFromBuffer;
		handle->msgConvertToBufferFunc = (void *) RM_MPU_FRAME_SYN_REQ_ConvertToBuffer;
		handle->logicalMsgFunc=(void *)rm_mpu_frame_syn_req;
break;
case 4: //RM_CO_SYNC_FILE_LOAD_REQ
		handle->message = malloc(sizeof(RM_MPU_SYN_FILE_LOAD_REQ));
		memset(handle->message, 0, sizeof(RM_MPU_SYN_FILE_LOAD_REQ));
		handle->bufferConvertToMsgFunc = (void *) RM_MPU_SYN_FILE_LOAD_REQ_ConvertFromBuffer;
		handle->msgConvertToBufferFunc = (void *) RM_MPU_SYN_FILE_LOAD_REQ_ConvertToBuffer;
		handle->logicalMsgFunc=(void *)rm_mpu_syn_file_load_req;
break;
case 5: //RM_MPU_ROLE_IND
		handle->message = malloc(sizeof(RM_MPU_ROLE_IND));
  	    memset(handle->message, 0, sizeof(RM_MPU_ROLE_IND));
  	    handle->bufferConvertToMsgFunc = (void *) RM_MPU_ROLE_IND_ConvertFromBuffer;
  	    handle->msgConvertToBufferFunc = (void *) RM_MPU_ROLE_IND_ConvertToBuffer;
  	    handle->logicalMsgFunc=(void *)rm_mpu_role_ind;
break;

default:
return ERROR_MESSAGE_TYPE;
break;
}

return true;
}


/*******************************************************************
 Function name		ats_mpu_unpackATSRMMessage
 description			initialize message handle totally
 parameter
 ATS_CO_MSG *		IN	 		handle
 const BYTE *		IN 			buf
 size_t 			IN 			len
 Return value
 true or false
 *******************************************************************/
int
ats_mpu_rm_unpackATSCOMessage(ATS_CO_MSG *handle, const BYTE *buf, size_t len)
{
	//buffer contains net header ,length = net header size + packet size
	//to parse net header firstly
	handle->fixed_len = (int) bytes2int(buf + 1,SHT2BYTE);
	handle->format = malloc(handle->fixed_len + 3);
	memset(handle->format, 0, handle->fixed_len + 3);
	memcpy(handle->format, buf,handle->fixed_len + 3);
	handle->bufferConvertToMsgFunc = NULL;
	handle->msgConvertToBufferFunc = NULL;
	handle->logicalMsgFunc = NULL;
	handle->setDBFunc = NULL;
	handle->getDBFunc = NULL;
	if (ats_mpu_rm_initATSCOMessage(handle) == false)
	{
		return false;
	}
	return true;
}


int
ats_mpu_rm_Unpack(sqlite3 *db, BYTE *packet, size_t len)
{

  ATS_CO_MSG *RM_message;

  RM_message = malloc(sizeof(ATS_CO_MSG));
  memset(RM_message, 0, sizeof(ATS_CO_MSG));

  //init members of messagee
//  if (ats_mpu_rm_initATSCOMessage(RM_message) == false)
//    {
//      ELOG( "init RM Message error");
//      return false;
//    }

  //unpack synchronization message packet
  ats_mpu_rm_unpackATSCOMessage(RM_message,packet,len);
  if(RM_message->bufferConvertToMsgFunc!=NULL)
  {
 	RM_message->bufferConvertToMsgFunc(RM_message->format, RM_message->message);
  }
  //to call logical processor
  if (RM_message->logicalMsgFunc != NULL)
  {
    RM_message->logicalMsgFunc(NULL, RM_message->message);
  }
  ats_co_freeATSCOMessage(RM_message);

  return true;
}


int
rm_mpu_fifo_manager(sqlite3 *db, BYTE *buf, size_t len)
{
  int fp;
  fd_set fds;
//  int s;
  int retval;
  int count = 0;
  unsigned long long int starttime = 0;
  unsigned long long int endtime = 0;

  if((fp=fifo_init(MPU_RM_PIPE))==false)
  {
  	ELOG("<%s>open mpu2rm FIFO error ", LOG_NAME_MPUMS);
  	return false;
  }

  if((fp=fifo_init(RM_MPU_PIPE))==false)
  {
   	  ELOG("<%s>open rm2mpu FIFO error ", LOG_NAME_MPUMS);
   	  return false;
  }

  //waiting for read*/

   while(1)
   {
	   if(count == 0)
	   {
		   starttime = get_time_msecond();
	   }
	   count++;
	   endtime = get_time_msecond();
	   if((endtime - starttime) >= 60000)
	   {
		   DLOG("mpu-rm thread is alive");
		   count = 0;
	   }

   FD_ZERO(&fds);
   FD_SET(fp,&fds);
   
   retval=select(FD_SETSIZE,&fds,(fd_set*)0,(fd_set *)0,(struct timeval *)NULL);
   if (retval == -1)
     {
     ELOG("Select RM MPU FIFO error ,i=%d",retval);
     }
//   else if (retval)
//     {
//   DLOG("RM MPU sync data is receiving  now.");

   if(FD_ISSET(fp,&fds))
       {
	   int le=read(fp,buf,BUFFER_SIZE);
   
	   DLOG("sync control received buffer size=%d",le);
   
	   printf("msgid: %d, len: %d.%d, content: %d\n", buf[0], buf[1], buf[2], buf[3]);

	   ats_mpu_rm_Unpack(db,buf,le);

       }
//     }

//  else
//       {
//      DLOG("synchronization data between RM and MPU is received.");
//       }
   } //for while

  //close(fp);
  //}
  return true;
}


int
RM_MPU_LOCK_REQ_ConvertFromBuffer(const BYTE buf[], RM_MPU_LOCK_REQ *message)
{
		message->iDesModuleID = countValueofBits(buf[0],5,3);
		message->iMessageID = countValueofBits(buf[0],0,5);
		message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
		message->iLockReason=bytes2int(&buf[3],CHAR2BYTE);

	return true;
}

int
RM_MPU_LOCK_REQ_ConvertToBuffer(RM_MPU_LOCK_REQ *message,const BYTE buf[])
{
		if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
		return -1;
		if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[3], CHAR2BYTE, message->iLockReason) != 0)
		return -1;

	return true;
}

int
RM_MPU_UNLOCK_IND_ConvertFromBuffer(const BYTE buf[], RM_MPU_UNLOCK_IND *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	return true;
}

int
RM_MPU_UNLOCK_IND_ConvertToBuffer(RM_MPU_UNLOCK_IND *message, const BYTE buf[])
{

		if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
		return -1;
		if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
		return -1;

	return true;
}

int
RM_MPU_FRAME_SYN_REQ_ConvertFromBuffer(const BYTE buf[], RM_MPU_FRAME_SYN_REQ *message)
{
	int i = 0;
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
			message->sMPUSyncTime = (u_int32) bytes2int(&buf[3], INT2BYTE);
			message->listnum = (message->iLength-1-2-4)/4;

			while (i < message->listnum)
			{
		    message->iHostID[i] = (u_int32) bytes2int(&buf[7+4*i], SHT2BYTE);
		    message->seqNo[i] = (u_int64) bytes2int(&buf[9+4*i], SHT2BYTE);
			i++;
			}
	return true;
}

int
RM_MPU_FRAME_SYN_REQ_ConvertToBuffer(RM_MPU_FRAME_SYN_REQ *message,const BYTE buf[])
{
	int i = 0;
			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return false;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[3], INT2BYTE, message->sMPUSyncTime) != 0)
			return false;

			while (i < message->listnum)
			{
			if (int2bytes((BYTE*)&buf[7+4*i], SHT2BYTE, message->iHostID[i]) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[9+4*i], SHT2BYTE, message->seqNo[i]) != 0)
			return false;
			i++;
			}
	return true;
}

int
RM_MPU_SYN_FILE_LOAD_REQ_ConvertFromBuffer(const BYTE buf[], RM_MPU_SYN_FILE_LOAD_REQ *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);

	return true;
}

int
RM_MPU_SYN_FILE_LOAD_REQ_ConvertToBuffer(RM_MPU_SYN_FILE_LOAD_REQ *message,const BYTE buf[])
{
			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return -1;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return -1;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return -1;

	return true;
}

int
RM_MPU_ROLE_IND_ConvertFromBuffer(const BYTE buf[], RM_MPU_ROLE_IND *message)
{
	//printf("msgid: %d, len: %d.%d, role: %d\n", buf[0], buf[1], buf[2], buf[3]);
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength =  (u_int32)bytes2int(&buf[1], SHT2BYTE);
			message->roleState = (u_int32)bytes2int(&buf[3], CHAR2BYTE);
			//ILOG("message->roleState = %d", message->roleState);
	return true;
}

int
RM_MPU_ROLE_IND_ConvertToBuffer(RM_MPU_ROLE_IND *message,const BYTE buf[])
{

			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return -1;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return -1;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return -1;
			if (int2bytes((BYTE*)&buf[3], CHAR2BYTE, message->roleState) != 0)
			return -1;
	return true;
}







int
MPU_RM_LOCK_RSP_ConvertFromBuffer(const BYTE buf[], MPU_RM_LOCK_RSP *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
			message->iLockReason=bytes2int(&buf[3],CHAR2BYTE);

	return true;
}

int
MPU_RM_LOCK_RSP_ConvertToBuffer( MPU_RM_LOCK_RSP *message,const BYTE buf[])
{
		if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
		return -1;
		if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[3], CHAR2BYTE, message->iLockReason) != 0)
		return -1;

	return true;
}

int
MPU_RM_LOCK_EXPIRED_IND_ConvertFromBuffer(const BYTE buf[], MPU_RM_LOCK_EXPIRED_IND *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
			message->iLockReason=bytes2int(&buf[3],CHAR2BYTE);

	return true;
}

int
MPU_RM_LOCK_EXPIRED_IND_ConvertToBuffer( MPU_RM_LOCK_EXPIRED_IND *message,const BYTE buf[])
{
		if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
		return -1;
		if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
		return -1;
		if (int2bytes((BYTE*)&buf[3], CHAR2BYTE, message->iLockReason) != 0)
		return -1;

	return true;
}

int
MPU_RM_FRAME_SYN_IND_ConvertFromBuffer(const BYTE buf[], MPU_RM_FRAME_SYN_IND *message)
{
	int i = 0;
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);
			message->sMPUSyncTime = (u_int32) bytes2int(&buf[3], INT2BYTE);
			message->listnum = (message->iLength-1-2-4)/4;
			while (i < message->listnum)
			{
		    message->iHostID[i] = (u_int32) bytes2int(&buf[7+4*i], SHT2BYTE);
		    message->seqNo[i] = (u_int64) bytes2int(&buf[9+4*i], SHT2BYTE);
			i++;
			}
	return true;
}

int
MPU_RM_FRAME_SYN_IND_ConvertToBuffer(MPU_RM_FRAME_SYN_IND *message,const BYTE buf[])
{
	int i = 0;
			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return false;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[3], INT2BYTE, message->sMPUSyncTime) != 0)
			return false;

			while (i < message->listnum)
			{
			if (int2bytes((BYTE*)&buf[7+4*i], SHT2BYTE, message->iHostID[i]) != 0)
			return false;
			if (int2bytes((BYTE*)&buf[9+4*i], SHT2BYTE, message->seqNo[i]) != 0)
			return false;
			i++;
			}
	return true;
}

int
MPU_RM_FRAME_SYN_RSP_ConvertFromBuffer(const BYTE buf[], MPU_RM_FRAME_SYN_RSP *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);

	return true;
}

int
MPU_RM_FRAME_SYN_RSP_ConvertToBuffer(MPU_RM_FRAME_SYN_RSP *message,const BYTE buf[])
{

			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return -1;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return -1;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return -1;
	return true;
}

int
MPU_RM_SYN_FILE_LOAD_CNF_ConvertFromBuffer(const BYTE buf[], MPU_RM_SYN_FILE_LOAD_CNF *message)
{
			message->iDesModuleID = countValueofBits(buf[0],5,3);
			message->iMessageID = countValueofBits(buf[0],0,5);
			message->iLength = (u_int32) bytes2int(&buf[1], SHT2BYTE);

	return true;
}

int
MPU_RM_SYN_FILE_LOAD_CNF_ConvertToBuffer(MPU_RM_SYN_FILE_LOAD_CNF *message,const BYTE buf[])
{
			if (setBitsOfByte((BYTE*)&buf[0], 5, 3, message->iDesModuleID) != 0)
			return -1;
			if (setBitsOfByte((BYTE*)&buf[0], 0, 5, message->iMessageID) != 0)
			return -1;
			if (int2bytes((BYTE*)&buf[1], SHT2BYTE, message->iLength) != 0)
			return -1;

	return true;
}
