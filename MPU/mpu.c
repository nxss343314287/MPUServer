/*******************************************************************
 filename      mpu.c
 author        root
 created date  2012-1-9
 description   specifications of implemented functions
 warning
 modify history
 author        date        modify        description
 *******************************************************************/



#include "include/MPU.h"
#include "../log.h"
#include <semaphore.h>
#include "../dllmain.h"
sqlite3 *mpuDB;
MPU *pMPU_PL;

#define   SEM_NAME_NOTIFYMAIN    "mpu2main"
sem_t * semofmpu2main;

const char *  LOG_NAME_MPU	= "MPU.INIT";


int
mpu_initialize(MPU *pMPU, sqlite3 *db, log4c_category_t *log)
{
	DLOG("PL mpu_initialize");
	char*  databasePathName = PLDB_NAME;
	pMPU_PL = NULL;
//	RegElementStatusRptInterface(ats_mpu_unpackElementStatus);

//	RegCommandConfirmInterface(ats_mpu_unpackCommandConfirm);



	DLOG("PL databasePathName=%s",PLDB_NAME);
	int i=1;
	for(;i<=INTERLOCK_REGION_COUNT;i++)
	{
		DLOG("PL InitPl INTERLOCK ID=%d",i);
//		InitPl(i, databasePathName);
	}

  //firstly,we should initial database handle and log4c category
  pMPU->pLogger=log;
  pMPU->pDB=db;

	mpuThreadSyncMutex_create(&pMPU->log_mutex);

  //allocate memory resource for MPU Center structure
  mpuThreadSyncMutex_lock(&pMPU->log_mutex);
  pMPU->pMPUCenter=mpuCenter_initialize(pMPU->pMPUCenter);
  mpuThreadSyncMutex_unlock(&pMPU->log_mutex);

  //start MPU Receiver thread
  pMPU->pMPUReceiver=mpuReceiver_initialize(pMPU);
  if(pMPU->pMPUReceiver == NULL)
	  return false;
  //start MPU Analyzer thread
  pMPU->pMPUAnalyzer=mpuAnalyzer_initialize(pMPU);
  if(pMPU->pMPUAnalyzer == NULL)
  	  return false;
  //start master slave synchronization thread
  pMPU->pMPUMasterSlaveSync=mpuMasterSlaveSync_initialize(pMPU);
  if(pMPU->pMPUMasterSlaveSync == NULL)
  	  return false;

  pMPU->pMPUExtesionRead=mpuThreadExtesionRead_initialize(pMPU);
  if(pMPU->pMPUExtesionRead == NULL)
	  return false;

  pMPU->pMPUExtesionWrite=mpuThreadExtesionWrite_initialize(pMPU);
  if(pMPU->pMPUExtesionWrite == NULL)
  	  return false;

  //pMPU->pMPUSender = malloc(sizeof(MPUSender));
  //memset(pMPU->pMPUSender,0,sizeof(MPUSender));
  //mpuSender_initialize(pMPU->pMPUSender);

  //We can monitor status of all threads and get parameters of working threads
  //by accessing information of threads which have registered in registration list.
  //If some threads are not running	correctly,we must to kill or restart the thread.
  //This thread scheduler also can initialize the working threads of MPU.

//  pMPU->pThreadScheduler=mpuThreadScheduler_initialize(pMPU->pThreadScheduler);
  if(pMPU->pMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID == MASTER)
  {
	   ILOG( "<%s>MPU initial role is: MASTER", LOG_NAME_MPU);
  }
  else
  {
	  ILOG( "<%s>MPU initial role is: SLAVE", LOG_NAME_MPU);
  }


  pMPU_PL=pMPU;
  return true;
}
//�ͷ�MPU

int
mpu_free(MPU *pMPU)
{
	if(pMPU->pMPUExtesionRead != NULL)
	{
		mpuThreadExtesionRead_free(pMPU->pMPUExtesionRead);
	    free(pMPU->pMPUExtesionRead);
	}

	if(pMPU->pMPUExtesionWrite != NULL)
	{
		mpuThreadExtesionWrite_free(pMPU->pMPUExtesionWrite);
		free(pMPU->pMPUExtesionWrite);
	}

  //free master slave synchronization thread
  if(pMPU->pMPUMasterSlaveSync != NULL)
  {
  mpuMasterSlaveSync_free(pMPU->pMPUMasterSlaveSync);
  free(pMPU->pMPUMasterSlaveSync);
  }
  //free MPU Receiver thread
  if(pMPU->pMPUReceiver != NULL)
  {
  mpuReceiver_free(pMPU->pMPUReceiver);
  free(pMPU->pMPUReceiver);
  }
  //free MPU Analyzer thread
  if(pMPU->pMPUAnalyzer != NULL)
  {
  mpuAnalyzer_free(pMPU->pMPUAnalyzer);
  free(pMPU->pMPUAnalyzer);
  }
  //free MPU Sender thread
  //MPUSender_free(pMPU->pMPUSender);
  //free MPU Scheduler thread
//  mpuThreadScheduler_free(pMPU->pThreadScheduler);
//  free(pMPU->pThreadScheduler);
  //free MPU Center area
  mpuCenter_free(pMPU->pMPUCenter);
  free(pMPU->pMPUCenter);

   if(&pMPU->log_mutex!=NULL)
	 mpuThreadSyncMutex_destroy(&pMPU->log_mutex);
  return true;
}
//

int
mpu_start(MPU *pMPU, sqlite3 *db, log4c_category_t *log)
{
	int result = false;
	result = mpu_initialize(pMPU, db, log);
	if(result == true)
	{
		semofmpu2main = sem_open(SEM_NAME_NOTIFYMAIN,0);
		if(semofmpu2main == SEM_FAILED)
		{
			ELOG( "<%s>MPU open semofmpu2main failed!", LOG_NAME_MPU);
			mpu_free(pMPU);
			return false;
		}
		else
		{
			sem_post(semofmpu2main);
			DLOG("<%s>mpu start working, notify main sucess!",LOG_NAME_MPU);
			return true;
		}
	}
	else
	{
		mpu_free(pMPU);
		ELOG("<%s>mpu start error,thread can not work!",LOG_NAME_MPU);
		return false;
	}
}
/*******************************************************************
 Function name			mpu_getMPUHandle
 description
 parameter

 Return value
 true or false

 *******************************************************************/
MPU *
mpu_getMPUHandle()
{
	return pMPU_PL;
}
