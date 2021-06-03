/*******************************************************************
filename      mpuMasterSlaveSync.c
author        root
created date  2012-1-9
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/

#include "include/MPU.h"
#include	 <fcntl.h>

#define SEND_TO_RM_MSGLEN 128

static int mpu_rm_fifo_fd = -1;

int changeCountTimes = 0;
MPUCenter *thisMPUCenter;
/*******************************************************************
 Function name			mpuGetMpuRmFifoFd
 description			open the rm fifo
 parameter

 Return value
 true or false

 *******************************************************************/
int mpuGetMpuRmFifoFd()
{
	if (-1 == mpu_rm_fifo_fd)
	{
		mpu_rm_fifo_fd = open(MPU_RM_PIPE, O_RDWR);
	}
	return mpu_rm_fifo_fd;
}

void mpuReopenMpuRmFifo()
{
	ELOG("mpuReopenMpuRmFifo: fifo error: %d", errno);
	mpu_rm_fifo_fd = open(MPU_RM_PIPE, O_RDWR);
}



/*******************************************************************
 Function name			mpuMasterSlaveSync_running
 description			implemented a synchronization thread for master and slave MPU.
 parameter
 MPU *         		 IN/OUT    	 	    mpu

 Return value
 true or false

 *******************************************************************/
static void *
mpuMasterSlaveSync_running(void *arg)
{
  BYTE sync_buf[BUFFER_SIZE];
  MPU *mpu = (MPU *) arg;

  rm_mpu_fifo_manager(mpu->pDB,sync_buf,BUFFER_SIZE);
  return mpu->pMPUMasterSlaveSync;
}

//��ʼ��ͬ����
MPUMasterSlaveSync * mpuMasterSlaveSync_initialize(MPU  *mpu)
{

	  int result;

	  //start MPU Receiver working thread
	  mpu->pMPUMasterSlaveSync = malloc(sizeof(MPUMasterSlaveSync));
	  memset(mpu->pMPUMasterSlaveSync, 0, sizeof(MPUMasterSlaveSync));

	  //1.create thread mutex
	  mpuThreadSyncMutex_create(&mpu->pMPUMasterSlaveSync->sync_thread_mutex);
	  //2.create receiver thread
	  result = pthread_create(&mpu->pMPUMasterSlaveSync->MPUMasterSlaveSyncThread, NULL,mpuMasterSlaveSync_running, mpu);
	  //receiver_thread(mpu);
	  thisMPUCenter = mpu->pMPUCenter;

	  if(result == 0)
		  return mpu->pMPUMasterSlaveSync;
	  else
		  return NULL;
}

//�ͷ�ͬ����
int mpuMasterSlaveSync_free(MPUMasterSlaveSync  *synchronizer)
{
		mpuThreadSyncMutex_destroy(&synchronizer->sync_thread_mutex);
		return true;
}

//��ͣ
int mpuMasterSlaveSync_pause(MPUMasterSlaveSync  *synchronizer)
{
		return true;
}


//��λ
int mpuMasterSlaveSync_resume(MPUMasterSlaveSync  *synchronizer)
{
		return true;

}


/*******************************************************************
 Function name			rm_mpu_lock_req
 description			rm mpu lock request
 parameter
 sqlite3 *         		 IN/OUT    	 	    db
 RM_MPU_LOCK_REQ *         		 IN/OUT    	 	    msg

 Return value
 true or false

 *******************************************************************/
int rm_mpu_lock_req(sqlite3 *db, RM_MPU_LOCK_REQ *msg)
{
	MPUMasterSlaveSyncInfo *info = malloc(sizeof(MPUMasterSlaveSyncInfo));
	memset(info, 0, sizeof(MPUMasterSlaveSyncInfo));
	mpuMasterSlaveSyncInfo_get(thisMPUCenter, info);
	mpuThreadSyncMutex_lock(&thisMPUCenter->stMasterSlaveSyncMutex);
	if(info->iMPUMasterSlaveID == MASTER)
	{
		if(msg->iLockReason == 1)
		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = MCHANGINGSWITCH;
		}
		else
		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = MCHANGINGINIT;
			changeCountTimes = 0;
		}
	}
	if(info->iMPUMasterSlaveID == SLAVE) // rm will not send lock request to slave
	{
		if(msg->iLockReason == 1)
		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = SCHANGINGSWITCH;
		}
		else
		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = SCHANGINGINIT;
		}
	}
	ILOG("rm_mpu_lock_req");
	mpuThreadSyncMutex_unlock(&thisMPUCenter->stMasterSlaveSyncMutex);
	mpuMasterSlaveSyncInfo_free(info);
	DLOG("receive lock request: iLockReason=%d",msg->iLockReason);
	DLOG("master lock");
	return true;
}

int rm_mpu_unlock_ind(sqlite3 *db, RM_MPU_UNLOCK_IND *msg)
{
	MPUMasterSlaveSyncInfo *info = malloc(sizeof(MPUMasterSlaveSyncInfo));
	memset(info, 0, sizeof(MPUMasterSlaveSyncInfo));
	mpuMasterSlaveSyncInfo_get(thisMPUCenter, info);
	mpuThreadSyncMutex_lock(&thisMPUCenter->stMasterSlaveSyncMutex);
	if(info->iMPUMasterSlaveID == MCHANGINGINIT
	   || info->iMPUMasterSlaveID == MCHANGINGSWITCH)
	{
		thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = MASTER;
		DLOG("master unlock");

	}
	mpuThreadSyncMutex_unlock(&thisMPUCenter->stMasterSlaveSyncMutex);
	mpuMasterSlaveSyncInfo_free(info);

	return true;
}

int rm_mpu_frame_syn_req(sqlite3 *db, RM_MPU_FRAME_SYN_REQ *msg)
{
	mpuThreadSyncMutex_lock(&thisMPUCenter->stMasterSlaveSyncMutex);
	thisMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFrame = TRUE;
	memcpy(thisMPUCenter->pMasterSlaveSyncInfo->frameSendFromMaster,msg,sizeof(RM_MPU_FRAME_SYN_REQ));
	mpuThreadSyncMutex_unlock(&thisMPUCenter->stMasterSlaveSyncMutex);

	DLOG("flag 1");
	return true;
}

int rm_mpu_syn_file_load_req(sqlite3 *db, RM_MPU_SYN_FILE_LOAD_REQ *msg)
{
	DLOG("receive RM_MPU_SYN_FILE_LOAD_REQ");
	mpuThreadSyncMutex_lock(&thisMPUCenter->stMasterSlaveSyncMutex);
	thisMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFile = TRUE;
	DLOG("mpu->pMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFile = %d",
			thisMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFile);
	mpuThreadSyncMutex_unlock(&thisMPUCenter->stMasterSlaveSyncMutex);

	return true;
}

int rm_mpu_role_ind(sqlite3 *db, RM_MPU_ROLE_IND *msg)
{
	MPUMasterSlaveSyncInfo *info = malloc(sizeof(MPUMasterSlaveSyncInfo));
	memset(info, 0, sizeof(MPUMasterSlaveSyncInfo));
	mpuMasterSlaveSyncInfo_get(thisMPUCenter, info);
	mpuThreadSyncMutex_lock(&thisMPUCenter->stMasterSlaveSyncMutex);
	if(info->iMPUMasterSlaveID == NOROLE)
	{
		if(msg->roleState == 1)

		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = MASTER;
		}
		if(msg->roleState == 2)
		{
			thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = SCHANGINGINIT;
		}
	}
	else
	{
		thisMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = msg->roleState;
	}
	mpuThreadSyncMutex_unlock(&thisMPUCenter->stMasterSlaveSyncMutex);
	mpuMasterSlaveSyncInfo_free(info);

	ILOG("MPU receive rm_mpu_role_ind: roleState = %d", msg->roleState);
	return true;
}

int mpu_rm_lock_rsp(MPU_RM_LOCK_RSP *msg)
{
	int fp;
	int re;
	BYTE  *buffer;
	buffer = malloc(SEND_TO_RM_MSGLEN);
	memset(buffer, 0, SEND_TO_RM_MSGLEN);
	MPU_RM_LOCK_RSP_ConvertToBuffer(msg, buffer);

//	if((fp = open(MPU_RM_PIPE, O_WRONLY)) < 0)
//	{
//		ELOG("open mpu2rm pipe for write error\n");
//		free(buffer);
//		return false;
//	}
	if (-1 != (fp = mpuGetMpuRmFifoFd()))
	{
		if((re = write(fp, buffer, 4)) < 0)
		{
			mpuReopenMpuRmFifo();
			ELOG("write mpu2rm pipe error,re=%d\n",re);
			free(buffer);
			return false;
		}
	}
//	if((re=write(fp,buffer,SEND_TO_RM_MSGLEN))<0)
	ILOG("mpu_rm_lock_rsp");

//	close(fp);
	free(buffer);
	return true;
}

int mpu_rm_lock_expired_ind(MPU_RM_LOCK_EXPIRED_IND *msg)
{
	int fp;
	int re;
	BYTE  *buffer;
	buffer=malloc(SEND_TO_RM_MSGLEN);
	memset(buffer,0,SEND_TO_RM_MSGLEN);
	MPU_RM_LOCK_EXPIRED_IND_ConvertToBuffer(msg, buffer);

//	if((fp = open(MPU_RM_PIPE, O_WRONLY)) < 0)
//	{
//		printf("open mpu2rm pipe for write error\n");
//		free(buffer);
//		return false;
//	}
//	if((re=write(fp,buffer,SEND_TO_RM_MSGLEN))<0)
	if (-1 != (fp = mpuGetMpuRmFifoFd()))
	{
		if((re = write(fp, buffer, 4))<0)
		{
			mpuReopenMpuRmFifo();
			printf("write mpu2rm pipe error,re=%d\n",re);
			free(buffer);
			return false;
		}
	}

//	close(fp);
	free(buffer);
	return true;
}


int mpu_rm_frame_syn_ind(MPU_RM_FRAME_SYN_IND *msg)
{
	int fp;
	int re, msg_len = 0;
	BYTE  *buffer;
	buffer = malloc(SEND_TO_RM_MSGLEN + PERIOD_PROCESS_MSGNUM * 4);
	memset(buffer,0,SEND_TO_RM_MSGLEN + PERIOD_PROCESS_MSGNUM * 4);
	MPU_RM_FRAME_SYN_IND_ConvertToBuffer(msg,buffer);

//	if((fp=open(MPU_RM_PIPE,O_WRONLY))<0)
//	{
//		printf("open mpu2rm pipe for write error\n");
//		free(buffer);
//		return false;
//	}

	msg_len = 3 + sizeof(int) + (sizeof(short int) + sizeof(short int)) * msg->listnum;
//	if((re=write(fp,buffer,SEND_TO_RM_MSGLEN))<0)
	if (-1 != (fp = mpuGetMpuRmFifoFd()))
	{
		if((re = write(fp, buffer, msg_len)) < 0)
		{
			mpuReopenMpuRmFifo();
			printf("write mpu2rm pipe error,re=%d\n",re);
			free(buffer);
			return false;
		}
	}
	DLOG("write to mpu2rm pipe sucess, listnum = %d, lastseq = %d",
			msg->listnum, msg->seqNo[msg->listnum - 1]);
//	close(fp);
	free(buffer);
	return true;
}

int mpu_rm_frame_syn_rsp(MPU_RM_FRAME_SYN_RSP *msg)
{
	int fp;
	int re;
	BYTE  *buffer;
	buffer = malloc(SEND_TO_RM_MSGLEN);
	memset(buffer, 0, SEND_TO_RM_MSGLEN);
	MPU_RM_FRAME_SYN_RSP_ConvertToBuffer(msg, buffer);

//	if((fp = open(MPU_RM_PIPE, O_WRONLY)) < 0)
//	{
//		printf("open mpu2rm pipe for write error\n");
//		free(buffer);
//		return false;
//	}
//	if((re = write(fp, buffer, SEND_TO_RM_MSGLEN)) < 0)
	if (-1 != (fp = mpuGetMpuRmFifoFd()))
	{
		if((re = write(fp, buffer, 3)) < 0)
		{
			mpuReopenMpuRmFifo();
			printf("write mpu2rm pipe error,re=%d\n",re);
			free(buffer);
			return false;
		}
	}

//	close(fp);
	free(buffer);
	return true;
}

int mpu_rm_syn_file_load_cnf(MPU_RM_SYN_FILE_LOAD_CNF *msg)
{
	int fp;
	int re;
	BYTE  *buffer;
//	close(fp);

	buffer=malloc(SEND_TO_RM_MSGLEN);
	memset(buffer,0,SEND_TO_RM_MSGLEN);
	MPU_RM_SYN_FILE_LOAD_CNF_ConvertToBuffer(msg,buffer);

//	if((fp=open(MPU_RM_PIPE,O_WRONLY))<0)
//	{
//		printf("open mpu2rm pipe for write error\n");
//		free(buffer);
//		return false;
//	}
//	if((re=write(fp,buffer,SEND_TO_RM_MSGLEN))<0)
	if (-1 != (fp = mpuGetMpuRmFifoFd()))
	{
		if((re = write(fp, buffer, 3)) < 0)
		{
			mpuReopenMpuRmFifo();
			printf("write mpu2rm pipe error,re=%d\n",re);
			free(buffer);
			return false;
		}
		else
		{
			ILOG("Send load confirm success");
		}
	}

//	close(fp);
	free(buffer);
	return true;
}
