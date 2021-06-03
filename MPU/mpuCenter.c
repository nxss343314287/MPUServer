/*******************************************************************
 filename      mpuCenter.c
 author        root
 created date  2012-1-9
 description   specifications of implemented functions
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#include<fcntl.h>
#include "include/MPU.h"
#include "../log.h"

const char *  LOG_NAME_MPUCenter = "MPU.Center";
/*******************************************************************
 Function name			mpuMasterSlaveSyncInfo_init
 description			init the master slave server sync information
 parameter
 MPUMasterSlaveSyncInfo *				IN/OUT 		info
 Return value
 true or false

 *******************************************************************/

int mpuMasterSlaveSyncInfo_init(MPUMasterSlaveSyncInfo *info) {

	info->iMPUMasterSlaveID = NOROLE;
	info->slaveReceiveFrame = FALSE;
	info->slaveReceiveFile = FALSE;
	info->frameSendFromMaster = malloc(sizeof(RM_MPU_FRAME_SYN_REQ));
	memset(info->frameSendFromMaster, 0, sizeof(RM_MPU_FRAME_SYN_REQ));
	return true;
}
/*******************************************************************
 Function name			mpuMasterSlaveSyncInfo_get
 description			get the master slave server sync information
 parameter
 MPUCenter *			IN/OUT 			center
 MPUMasterSlaveSyncInfo *				IN/OUT 		info
 Return value
 true or false

 *******************************************************************/
int mpuMasterSlaveSyncInfo_get(MPUCenter *center, MPUMasterSlaveSyncInfo *info) {

	mpuThreadSyncMutex_lock(&center->stMasterSlaveSyncMutex);
	info->iMPUMasterSlaveID = center->pMasterSlaveSyncInfo->iMPUMasterSlaveID;
	info->slaveReceiveFrame = center->pMasterSlaveSyncInfo->slaveReceiveFrame;
	info->slaveReceiveFile = center->pMasterSlaveSyncInfo->slaveReceiveFile;
	//info->frameSendFromMaster = center->pMasterSlaveSyncInfo->frameSendFromMaster;

	mpuThreadSyncMutex_unlock(&center->stMasterSlaveSyncMutex);
	return true;
}
/*******************************************************************
 Function name			mpuMasterSlaveSyncInfo_refresh
 description			refresh the master slave server sync information
 parameter
 MPUCenter *			IN/OUT 			center
 MPUMasterSlaveSyncInfo *				IN/OUT 		info
 Return value
 true or false

 *******************************************************************/
int mpuMasterSlaveSyncInfo_refresh(MPUCenter *center,
		MPUMasterSlaveSyncInfo *info) {
	mpuThreadSyncMutex_lock(&center->stMasterSlaveSyncMutex);
	center->pMasterSlaveSyncInfo->iMPUMasterSlaveID = info->iMPUMasterSlaveID;
	center->pMasterSlaveSyncInfo->slaveReceiveFrame = info->slaveReceiveFrame;
	center->pMasterSlaveSyncInfo->slaveReceiveFile = info->slaveReceiveFile;
	memcpy(center->pMasterSlaveSyncInfo->frameSendFromMaster,info->frameSendFromMaster,sizeof(RM_MPU_FRAME_SYN_REQ));
	mpuThreadSyncMutex_unlock(&center->stMasterSlaveSyncMutex);
	return true;
}
/*******************************************************************
 Function name			mpuMasterSlaveSyncInfo_check
 description
 parameter
 MPUCenter *			IN/OUT 			center
 u_int64 				IN/OUT 			counter
 Return value
 true or false

 *******************************************************************/
int mpuMasterSlaveSyncInfo_check(MPUMasterSlaveSyncInfo *info, u_int64 counter) {

	return true;

}
/*******************************************************************
 Function name			mpuMasterSlaveSyncInfo_free
 description			free the master slave sync info
 parameter
 MPUMasterSlaveSyncInfo *			IN/OUT 			info
 Return value
 true or false

 *******************************************************************/
int mpuMasterSlaveSyncInfo_free(MPUMasterSlaveSyncInfo *info) {

	if (info->frameSendFromMaster != NULL)
		free(info->frameSendFromMaster);
	if (info != NULL)
		free(info);
	return true;
}
/*******************************************************************
 Function name			fifo_init
 description			init the fifo
 parameter
 char *			IN 			szFIFOName
 Return value
 true or false

 *******************************************************************/
int fifo_init(char *szFIFOName) {
	int fp;
	int s;

	if ((fp = open(szFIFOName, O_RDWR)) < 0)
	{
		if ((s = mkfifo(szFIFOName, 0666)) < 0)
		{
			ELOG("create file error,pipe name : %s",szFIFOName);
		}
		else
		{
		   if ((fp = open(szFIFOName, O_RDWR)) < 0)
		   {

			   ELOG("<%s>mpuserver open FIFO error ,pipe name : %s", LOG_NAME_MPUCenter, szFIFOName);
			   return false;
		   }
		}
	}
	else
	{
		ILOG("open pipe ok, pipename=%s,fp=%d",szFIFOName,fp);
	}
	return fp;
}

/*******************************************************************
 Function name			mpuCenter_FIFO_initial
 description
 parameter
 MPUCenter *			IN 			pMPUCenter
 Return value
 true or false

 *******************************************************************/
int mpuCenter_FIFO_initial(MPUCenter *pMPUCenter) {

	return true;
}
/*******************************************************************
 Function name			mpuCenter_initialize
 description			init the MPUCenter
 parameter
 MPUCenter *			IN 			pMPUCenter
 Return value
 MPUCenter *

 *******************************************************************/
MPUCenter *
mpuCenter_initialize(MPUCenter *pMPUCenter) {
	//below is structure initial

	//apply for memory
	pMPUCenter = malloc(sizeof(MPUCenter));
	memset(pMPUCenter, 0, sizeof(MPUCenter));
	//mpuCenter_FIFO_initial(pMPUCenter);

	//create master slave sync info
	pMPUCenter->pMasterSlaveSyncInfo = malloc(sizeof(MPUMasterSlaveSyncInfo));
	memset(pMPUCenter->pMasterSlaveSyncInfo, 0, sizeof(MPUMasterSlaveSyncInfo));

	//create master slave mutex
	mpuThreadSyncMutex_create(&pMPUCenter->stMasterSlaveSyncMutex);
	mpuThreadSyncMutex_lock(&pMPUCenter->stMasterSlaveSyncMutex);
	mpuMasterSlaveSyncInfo_init(pMPUCenter->pMasterSlaveSyncInfo);
	mpuThreadSyncMutex_unlock(&pMPUCenter->stMasterSlaveSyncMutex);
	//below is thread control
	//create receiver pool
	pMPUCenter->pReceiverPool = malloc(sizeof(MPUReceiverPool));
	memset(pMPUCenter->pReceiverPool, 0, sizeof(MPUReceiverPool));
	mpuReceiverPool_initialize(pMPUCenter->pReceiverPool);
	//create receiver pool mutex

	mpuThreadSyncMutex_create(&pMPUCenter->stReceiverPoolMutex);
	//create thread scheduler,where should we create a receiver and a analysis thread?
	//then,we can control accessing process of all threads of MPUCenter.Cause the threads are
	//belong to MPU, scheduler should get MPU handler for to implement manage threads of MPU.

	return pMPUCenter;
}
/*******************************************************************
 Function name			mpuCenter_free
 description			free the MPUCenter
 parameter
 MPUCenter *			IN 			pMPUCenter
 Return value
 MPUCenter *

 *******************************************************************/
int mpuCenter_free(MPUCenter *pMPUCenter) {

	//free receiver pool
	//if(pMPUCenter->pReceiverPool!=NULL)
	mpuReceiverPool_free(pMPUCenter->pReceiverPool);
	free(pMPUCenter->pReceiverPool);
	//free receiver pool mutex
	mpuThreadSyncMutex_destroy(&pMPUCenter->stReceiverPoolMutex);
	//free master slave sync info
	free(pMPUCenter->pMasterSlaveSyncInfo);
	//create master slave mutex
	mpuThreadSyncMutex_destroy(&pMPUCenter->stMasterSlaveSyncMutex);

	return true;
}
