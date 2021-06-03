/*******************************************************************
 filename      mpuAnalyzer.c
 author        root
 created date  2012-1-9
 description   specifications of implemented functions
 warning
 modify history
 author        date        modify        description
 *******************************************************************/


#include"include/MPU.h"
#include "include/MPUReceiver.h"
#include "include/MPUReceiverPool.h"
#include "include/MPUSender.h"
#include "include/MPUMessage.h"
#include "include/MPUSendMessage.h"
//#include "include/MPU.h"
#include "../log.h"
#include <semaphore.h>

#define   SEM_NAME_NOTIFY_MPU    "cas2mpu"
#define   SEM_NAME_NOTIFY_CAS    "mpu2cas"

#define   FEP1   70
#define   FEP2   71
#define   FEP3   72
#define   FEP4   73
#define   TS     4

int Master_switch_init_finish = 0;
int alreadySendLockRsptoRM = 0;
int isInSLAVE =FALSE;

sem_t * semofcas2mpu;
sem_t * semofmpu2cas;
int casheartbeat=0; //add by quj
int TRCLoadOKflag = 0;
int TRCSendflag = 0;
time_t t_reciever[4] = {0}; //add by quj
time_t t_not_reciever[4] = {0};
int casheartstatus[4]={0};
int casheartstatusold[4]={0};
int casheart[4]={0};
void mpuAnalyzer_TrcOrCasHeartBeat_MSG(MPU *mpu,ATS_CO_MSG *this_message);
int mpuAnalyzer_LoadRqsIsALL_MSG(MPU *mpu,ATS_CO_MSG *this_message);
const char *  LOG_NAME_MPUAnalyzer = "MPU.Analyzer";

int replay_readthead_can_copy_db = 0;
int replay_writethead_can_import_db = 0;
char replay_import_result=1;/*MPU import replay data result*/
extern sqlite3 *db_replay_adapter;

/*******************************************************************
 Function name			mpuAnalyzer_process_batch_master
 description		    handle the receive pool msg
 parameter
 MPU *				IN/OUT 		mpu
 u32                IN          step
 MPU_RM_FRAME_SYN_IND *				OUT 			framesendtoslave
 u32*				OUT 		poolnodecount
 Return value
 true or false

 *******************************************************************/
u32 mpuAnalyzer_process_batch_master(MPU *mpu, u32 step, MPU_RM_FRAME_SYN_IND *framesendtoslave,u32 *poolnodecount)
{
	u32 i = 0;
	int seq = 0;
	int hostid = 0;
	int MainOrBak = 0;
	ATS_CO_MSG *this_message;
	/****************************************
	 * the receiver pool is empty or not
		 * msg_flag = 0  empty
		 *            1  not empty
	***************************************/
	int msg_flag = -1;


	ILOG("mpuAnalyzer_process_batch_master111");
	while (i < step)
	{
		//lock pool mutex

		//mpuThreadSyncMutex_lock(&mpu->log_mutex);
		mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);

		//check pool is full
		DLOG("MPU Analyzer Message pool=%d",mpu->pMPUCenter->pReceiverPool->count);
		if (mpu->pMPUCenter->pReceiverPool->count >= POOL_SIZE)
		{
		//  mpuThreadSyncMutex_lock(&mpu->log_mutex);
			DLOG(
					"<%s>MPUReceiverPool valid checking,out of pool size,count=%d",
					LOG_NAME_MPUAnalyzer,mpu->pMPUCenter->pReceiverPool->count);
		  //mpuThreadSyncMutex_unlock(&mpu->log_mutex);
			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
			msg_flag = 1;
			continue;
		}

		//if pool header is frank, then return false;
		if ((mpu->pMPUCenter->pReceiverPool->pHeaderPointer == NULL)
				||(mpu->pMPUCenter->pReceiverPool->count == 0))
		{
			//mpuThreadSyncMutex_unlock(&mpu->log_mutex);
			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
			framesendtoslave->listnum = 0;
			msg_flag = 0;
		}
		else
		{
			//set processing Pointer as header pointer
			msg_flag = 1;
			mpu->pMPUCenter->pReceiverPool->pProcessingPointer =
					mpu->pMPUCenter->pReceiverPool->pHeaderPointer;
		}


		if(msg_flag == 1)// msg is not empty
		{
		  this_message = malloc(sizeof(ATS_CO_MSG));
		  memset(this_message,0,sizeof(ATS_CO_MSG));
		  this_message->fixed_len = mpu->pMPUCenter->pReceiverPool->pProcessingPointer->pMsgHandle->fixed_len;
		  this_message->format = malloc(this_message->fixed_len);
		  memset(this_message->format, 0, this_message->fixed_len);
		  memcpy(this_message->format, mpu->pMPUCenter->pReceiverPool->pProcessingPointer->pMsgHandle->format ,this_message->fixed_len);
		  this_message->header = mpu->pMPUCenter->pReceiverPool->pProcessingPointer->pMsgHandle->header;
		  ILOG("app message head:seq=%d,host=%d,MainOrBak=%d",this_message->header.seqno,(int)this_message->header.srcno,(int)this_message->header.mainorbak);

		  ats_co_initMessageValue(this_message);

		  //mpuThreadSyncMutex_unlock(&mpu->log_mutex);
		  ILOG("app message config:config.index=%d,message_type=%d,element_type=%d",this_message->config.index,this_message->config.message_type,this_message->config.element_type);

		}
	  mpuAnalyzer_TrcOrCasHeartBeat_MSG(mpu,this_message);
	  mpuAnalyzer_LoadRqsIsALL_MSG(mpu,this_message);
	  if(msg_flag == 0)
	  {
			framesendtoslave->listnum =0;
			ILOG("framesendtoslave->listnum =%d",framesendtoslave->listnum );
			return false;
	  }
	  if(msg_flag == 1)// msg is not empty
	  {
		  if(this_message->config.index == 0)
		  {
			  ats_co_FepStatusMsg message_1;
			  ats_co_BufferConvertToFepStatusMsg(this_message->format ,&message_1);
			  if(message_1.fepID<=0 || message_1.fepID >=1000)
			  {
				  this_message->bufferConvertToMsgFunc = NULL;
				  this_message->setDBFunc = NULL;
			  }
		  }
			i++;
			//head pointer move to next
			mpu->pMPUCenter->pReceiverPool->pHeaderPointer =
					mpuReceiverPool_next(
					mpu->pMPUCenter->pReceiverPool,(MPUReceiverPoolNode *)
					mpu->pMPUCenter->pReceiverPool->pHeaderPointer);

			//delete processing pointer of pool
			mpu->pMPUCenter->pReceiverPool->pProcessingPointer=
			mpuReceiverPool_delete(mpu->pMPUCenter->pReceiverPool,
					mpu->pMPUCenter->pReceiverPool->pProcessingPointer);

			//if processing is end of ReceiverPool ,then return .
			if (mpu->pMPUCenter->pReceiverPool->pHeaderPointer == NULL)
			{
				*poolnodecount = mpu->pMPUCenter->pReceiverPool->count;
				mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);

				ats_co_unpackATSCOMessage(this_message);
				framesendtoslave->seqNo[i-1] = seq = this_message->header.seqno;
				framesendtoslave->iHostID[i-1] = hostid = this_message->header.srcno;
				framesendtoslave->MainOrBak[i-1] = MainOrBak = this_message->header.mainorbak;
				if (this_message->setDBFunc != NULL)
				{
					ats_co_refreshDBATSCOMessage(mpu->pDB,this_message);
				}
				if (this_message->logicalMsgFunc != NULL)
				{
					ats_co_logicalMsgFunc(mpu->pDB_replay,this_message);
				}
				if (this_message != NULL)
				{
					ats_co_freeATSCOMessage(this_message);
				}

				step = i;
				framesendtoslave->listnum = i;
				return seq;
			}

			*poolnodecount = mpu->pMPUCenter->pReceiverPool->count;
			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);

			ats_co_unpackATSCOMessage(this_message);
			framesendtoslave->seqNo[i-1] = seq = this_message->header.seqno;
			framesendtoslave->iHostID[i-1] = hostid = this_message->header.srcno;
			framesendtoslave->MainOrBak[i-1] = MainOrBak = this_message->header.mainorbak;
			if (this_message->setDBFunc != NULL)
			{
				ats_co_refreshDBATSCOMessage(mpu->pDB,this_message);
			}
			if (this_message->logicalMsgFunc != NULL)
			{
				ats_co_logicalMsgFunc(mpu->pDB_replay,this_message);
			}

			if (this_message != NULL)
			{
				ats_co_freeATSCOMessage(this_message);
			}
	  }

	}

	framesendtoslave->listnum = step;
	step = i;
	return seq;
}
/*******************************************************************
 Function name			mpuAnalyzer_LoadRqsIsALL_MSG
 description		    send the load request to the fep
 parameter
 MPU *				IN/OUT 		mpu
 ATS_CO_MSG*        IN          this_message
 Return value
 true or false

 *******************************************************************/
int mpuAnalyzer_LoadRqsIsALL_MSG(MPU *mpu,ATS_CO_MSG *this_message)
{
	int rc;
    char *errMsg = NULL;
    int msg_empty = -1;
   // sqlite3_stmt *stmt;
    char sqlstr[MAX_SQL_STR_LEN];
    if (mpu->pDB == NULL)
	{
    	FLOG("mpuAnalyzer_LoadRqsIsALL_MSG fatal error1");
    	return -1;
	}
    if(this_message == NULL)
    {
    	ILOG("MSG empty");
    	msg_empty = 0;
//    	return -1;
    }
    if(msg_empty != 0)
	{
		if(this_message->config.index == 18 )
		{
			ILOG("receive 400 430msg Load rsq start from %d",(int)this_message->header.srcno);
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "update LoadRqsStatus set Loadstart = '%d' where loadid='%d'", 1,(int)this_message->header.srcno);
			rc = sqlite3_exec(mpu->pDB, sqlstr, 0, 0, &errMsg);
			if (SQLITE_OK != rc)
			{
				ELOG("SQLerror: %s while running mpuAnalyzer_LoadRqsIsALL_MSG", errMsg);
				return -1;
			}
			ILOG("update LoadRqsStatus set Loadstart = '%d' where loadid='%d'", 1,(int)this_message->header.srcno);
		}
		if(this_message->config.index == 19)
		{
			ILOG("receive 400 431msg Load rsq end from %d",(int)this_message->header.srcno);
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "update LoadRqsStatus set Loadend = '%d' where loadid='%d'",1,(int)this_message->header.srcno);
			rc = sqlite3_exec(mpu->pDB, sqlstr, 0, 0, &errMsg);
			if (SQLITE_OK != rc)
			{
				ELOG("SQLerror: %s while running mpuAnalyzer_LoadRqsIsALL_MSG", errMsg);
				return -1;
			}
			ILOG("update LoadRqsStatus set Loadend = '%d' where loadid='%d'",1,(int)this_message->header.srcno);
		}
	}
	time_t t_now;                                  // time_t type
	time_t t_old[4];
    sqlite3_stmt *stmt;
    int loadflag[4]= {0};
    int Loadstart[4] ={ 0};
    int Loadend[4] = {0};
    int i = 0;

	struct timeval tv;     // tm struct
	gettimeofday(&tv, NULL);                    // get the second
	t_now = tv.tv_sec;
	for(i=70;i<74;i++)
	{
		memset(sqlstr, 0, MAX_SQL_STR_LEN);
		sprintf(sqlstr, "select LoadStart, Loadend, Loadtime,Loadflag from Loadrqsstatus where loadid=%d",i);
		rc = sqlite3_prepare(mpu->pDB, sqlstr, strlen(sqlstr), &stmt, NULL);
		rc = sqlite3_step(stmt);
		if (SQLITE_ROW == rc)
		{
			Loadstart[i-70] = sqlite3_column_int(stmt, 0);
			Loadend[i-70] = sqlite3_column_int(stmt, 1);
			t_old[i-70] = sqlite3_column_int(stmt, 2);
			loadflag[i-70] = sqlite3_column_int(stmt, 3);

		}
		else
		{
			FLOG("sqlite3 error :mpuAnalyzer_LoadRqsIsALL_MSG");
			sqlite3_finalize(stmt);
			return -1;
		}
		sqlite3_finalize(stmt);
		if(loadflag[i-70] ==1 )
		{
			return 0; // send load rqs before
		}
		if(loadflag[i-70] != 1 && (t_now-t_old[i-70]) >= 5 && (Loadstart[i-70] != 1 || Loadend[i-70] != 1))
		{
			if(LOCAL_OR_CENTER == CENTER_CAS)
			{
				adapter_co_send_loadRequest(i,1,1,1,1,1);
			}
			if(LOCAL_OR_CENTER == LOCAL_TRC)
			{
				adapter_co_send_loadRequest(TS,1,1,1,1,1);
			}

			ILOG("send load rqs to after 5s from fep%d",i);
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "update LoadRqsStatus set Loadflag = '%d' where loadid=%d",1,i);
			rc = sqlite3_exec(mpu->pDB, sqlstr, 0, 0, &errMsg);
			if (SQLITE_OK != rc)
			{
				ELOG("SQLerror: %s while running mpuAnalyzer_LoadRqsIsALL_MSG", errMsg);
				return -1;
			}
			ILOG("update LoadRqsStatus set Loadend = '%d' where loadid=%d",i);
		}
	}
	return 0;
}

/*******************************************************************
 Function name			mpuAnalyzer_TrcOrCasHeartBeat_MSG
 description		    handle the cas heart beat msg from the TRC server
 parameter
 MPU *				IN/OUT 		mpu
 ATS_CO_MSG*        IN          this_message
 Return value
 true or false

 *******************************************************************/
void mpuAnalyzer_TrcOrCasHeartBeat_MSG(MPU *mpu,ATS_CO_MSG *this_message)
{
	struct timeval tv;
	time_t t_now;
	sqlite3_stmt * stmt;
	int casheart;
	int i = 0;
	if ((NULL == mpu->pDB)||(NULL == this_message))
	{
		ILOG("not recieber any msg ");
		return;
	}
	else
	{
		if(this_message->config.index == 114)
		{
			ILOG("MSG index 114 handle");
			int rc;
			char *errMsg = NULL;
			char sqlstr[MAX_SQL_STR_LEN];
			gettimeofday(&tv, NULL);                    // get the second
			t_now = tv.tv_sec;

			int casheartstatusold = 0;
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "select cas_heartbeat_old from controlrightauto where fepid = '%d'",(int)this_message->header.srcno);
			rc = sqlite3_prepare(mpu->pDB, sqlstr, strlen(sqlstr), &stmt, NULL);
			rc = sqlite3_step(stmt);

			if(SQLITE_ROW == rc)
			{
				//t_reciever = sqlite3_column_int(stmt, 0);
				casheartstatusold = sqlite3_column_int(stmt, 0);
			}
			sqlite3_finalize(stmt);


			if(casheartstatusold == 1)
			{
				casheart = 4;//1-->1
			}
			else
			{
				casheart = 2;//0-->1
			}
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "update controlrightauto set cas_heartbeat_old = 1,heartbeatstatus=%d,receive_time = %d where fepid='%d'",casheart,t_now,(int)this_message->header.srcno);
			rc = sqlite3_exec(mpu->pDB, sqlstr, 0, 0, &errMsg);
			if (SQLITE_OK != rc)
			{
				ELOG("SQLerror: %s while running cas fep heart", errMsg);
			}
			else
			{
				DLOG("update controlrightauto set heartbeatstatus = %d where fepid='%d'",casheart,i);
			}
		}
		else
		{
			ILOG("MSG index is not 114 handle");
			int rc;
			char *errMsg = NULL;
			char sqlstr[MAX_SQL_STR_LEN];
			gettimeofday(&tv, NULL);                    // get the second
			t_now = tv.tv_sec;
			for(i=70;i<74;i++)
			{
				int t_reciever = 0;
				int casheartstatusold = 0;
				memset(sqlstr, 0, MAX_SQL_STR_LEN);
				sprintf(sqlstr, "select receive_time,cas_heartbeat_old from controlrightauto where fepid = '%d'",i);
				rc = sqlite3_prepare(mpu->pDB, sqlstr, strlen(sqlstr), &stmt, NULL);
				rc = sqlite3_step(stmt);

				if(SQLITE_ROW == rc)
				{
					t_reciever = sqlite3_column_int(stmt, 0);
					casheartstatusold = sqlite3_column_int(stmt, 1);
				}
				sqlite3_finalize(stmt);
				if((t_now - t_reciever) >= 5)
				{
					if(casheartstatusold == 1)
					{
						casheart = 3;//1-->0
					}
					else
					{
						casheart = 1;//0-->0
					}
					memset(sqlstr, 0, MAX_SQL_STR_LEN);
					sprintf(sqlstr, "update controlrightauto set cas_heartbeat_old = 0,heartbeatstatus = %d where fepid='%d'",casheart,i);
					rc = sqlite3_exec(mpu->pDB, sqlstr, 0, 0, &errMsg);
					if (SQLITE_OK != rc)
					{
						ELOG("SQLerror: %s while running cas fep heart", errMsg);
					}
					else
					{
						DLOG("update controlrightauto set heartbeatstatus = %d where fepid='%d'",casheart,i);
					}
				}
			}
		}
	}
}
/*******************************************************************
 Function name			mpuAnalyzer_process_batch_slaver
 description		    slaver cas handle the receive pool msg
 parameter
 MPU *				IN/OUT 		mpu
 int      			IN          hostidlist
 int      			IN          seqlist
 int      			IN          listnum
 u32 *   			IN          poolnodecount
 Return value
 true or false

 *******************************************************************/
u32 mpuAnalyzer_process_batch_slaver(MPU *mpu, int hostidlist[], int seqlist[], int listnum, u32 *poolnodecount) {
	u32 i = 0;
	ATS_CO_MSG *handle;
	ATS_CO_MSG *this_message;

	while (i < listnum) {
		//lock pool mutex

		mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);

		//if pool header is frank, then return false;
		if ((mpu->pMPUCenter->pReceiverPool->pHeaderPointer == NULL)) {
			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
			return false;
		}
		else {
		//set processing Pointer as header pointer
		mpu->pMPUCenter->pReceiverPool->pProcessingPointer =
				mpu->pMPUCenter->pReceiverPool->pHeaderPointer;
		}

		//now ,all of things are ready,so we can start to control progress of parsing...
		//unpack message packet
		handle = mpuReceiverPool_getMsg_byHostSeq(mpu->pMPUCenter->pReceiverPool, hostidlist[i], seqlist[i]);
		if(handle != NULL)
		{
			this_message = malloc(sizeof(ATS_CO_MSG));
			memset(this_message,0,sizeof(ATS_CO_MSG));
		    this_message->fixed_len = handle->fixed_len;
			this_message->format = malloc(this_message->fixed_len);
			memset(this_message->format, 0, this_message->fixed_len);
			memcpy(this_message->format, handle->format ,this_message->fixed_len);
			this_message->header = handle->header;
			ILOG("this_message head:seq=%d,host=%d",this_message->header.seqno,(int)this_message->header.srcno);
			ats_co_initMessageValue(this_message);

			ILOG("this_message config:message_type=%d,element_type=%d",this_message->config.message_type,this_message->config.element_type);

			mpuReceiverPool_MSG_deleteSmallSeq_bySingleMsg(mpu->pMPUCenter->pReceiverPool, handle);
			*poolnodecount = mpu->pMPUCenter->pReceiverPool->count;

			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);

			ats_co_unpackATSCOMessage(this_message);
			if (this_message->setDBFunc != NULL)
				ats_co_refreshDBATSCOMessage(mpu->pDB,this_message);

			if (this_message != NULL)
				ats_co_freeATSCOMessage(this_message);

		}
		else
		{
			ILOG("miss msg:seq=%d,host=%d", seqlist[i], hostidlist[i]);
			mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
			return false;
		}


		i++;
	}

	return true;
}
/*******************************************************************
 Function name			mpuAnalyzer_running
 description		    the cas analyzer msg server thread
 parameter
 MPU *				IN/OUT 		mpu
 Return value
 true or false

 *******************************************************************/
static void *
mpuAnalyzer_running(void *arg) {
	MPU *mpu = (MPU *) arg;
	sqlite3 *db = 0;
	sqlite3 *db_replay = 0;
	MPU_RM_FRAME_SYN_IND frameSyncMsg;
	u32 deal_number = 0, poolnodecount = 0;
	MPUMasterSlaveSyncInfo info;
//    int alreadySendLockRsptoRM = 0;
    int changeCountTimes = 0;
    int expire_flag = 0;
    int period_count = 0;
    int period_setoracle_count=0;
    int period_send_heart = 0;
    int replaydb_close_flag = 0;
    int rc = 0;

//    int Master_switch_init_finish = 0;

    unsigned long long int starttime = 0;
    unsigned long long int endtime = 0;

    if(adapter_init() == -1)
    	return NULL;

    //int slave_to_master_flag = 0;
    semofcas2mpu = sem_open(SEM_NAME_NOTIFY_MPU, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
    if(semofcas2mpu == SEM_FAILED)
    {
    	ELOG("mpuAnalyzer_running: sem_open(SEM_NAME_NOTIFYMPU,0) failed! errno: %d", errno);
    	return -1;
    }
    semofmpu2cas = sem_open(SEM_NAME_NOTIFY_CAS, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
    if(semofmpu2cas == SEM_FAILED)
    {
    	ELOG("mpuAnalyzer_running: sem_open(SEM_NAME_NOTIFYCAS,0) failed! errno: %d", errno);
    	return -1;
    }

    int isInit =TRUE;
	while (TRUE) {
        //entrance: get role info from MasterSlaveSyncInfo
		mpuMasterSlaveSyncInfo_get(mpu->pMPUCenter, &info);

		//1------> analyze  processing as master mode

		if (info.iMPUMasterSlaveID == MASTER)
		{
			if(isInSLAVE==FALSE&&isInit==TRUE)
			{
				ats_co_import_init_replayData("ATSForSZL2", "CASForSZL2", "CASForSZL2");
				isInit=FALSE;
			}
			ILOG("NOW IS MASTER");

			if(Master_switch_init_finish == 0)
			{
				ILOG("MASTER:semofcas2mpu START");
				if (0 != sem_wait(semofcas2mpu))
				{
					ELOG("mpuAnalyzer_running: sem_wait(semofcas2mpu) failed! errno: %d", errno);
					sem_close(semofcas2mpu);
					semofcas2mpu = sem_open(SEM_NAME_NOTIFY_MPU, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
					if (0 != semofcas2mpu)
					{
						ELOG("mpuAnalyzer_running: sem_open(semofcas2mpu) failed! errno: %d", errno);
//				        		unlink(SEM_NAME_NOTIFYMPU);
						usleep(1000);
						continue;
					}
					usleep(1000);
					continue;
				}
				ILOG("MASTER:semofcas2mpu OK");
			}
			else/*master and slaver finish syn,then mpu handle right now*/
			{
				Master_switch_init_finish = 0;
			}
			//DLOG("<%s>mpu receive notify from cas,start analyse msg!",LOG_NAME_MPUAnalyzer);

			// init period value
			alreadySendLockRsptoRM = 0;
			expire_flag = 0;
			replaydb_close_flag = 0;
			if(sqlite3_open(DB_NAME, &db) != SQLITE_OK)/*if open sqlite failed close,and send signal to cas*/
			{
				DLOG("mpu open db error!");
				sqlite3_close(db);
				while(0 != sem_post(semofmpu2cas))
				{
					ELOG("sem_post(semofmpu2cas)1 errno=%d! ",errno);
					sem_close(semofmpu2cas);
					semofmpu2cas = sem_open(SEM_NAME_NOTIFY_CAS, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
					if(0 != semofmpu2cas)
					{
						ELOG("sem_open(semofmpu2cas) 2errno=%d! ",errno);

					}
					usleep(1000);
				}
				continue;
			}

			rc=sqlite3_open(ATS_WORK_DB_FOR_REPLAY_NAME, &db_replay);
			if(rc != SQLITE_OK)
			{
				DLOG("mpu open db_replay error!");
				sqlite3_close(db);
				sqlite3_close(db_replay);
				while(0 != sem_post(semofmpu2cas))
				{
					ELOG("sem_post(semofmpu2cas)3 errno=%d! ",errno);
					sem_close(semofmpu2cas);
					semofmpu2cas = sem_open(SEM_NAME_NOTIFY_CAS, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
					if(0 != semofmpu2cas)
					{
						ELOG("sem_open(semofmpu2cas)4 errno=%d! ",errno);

					}
					usleep(1000);
				}
				continue;
			}
			mpu->pDB = db;
			mpu->pDB_replay = db_replay;
			db_replay_adapter = db_replay;

			mpuSender_SendTask_process (mpu->pDB);

			//(2).receive task: get msg from receivepool and update db
			unsigned long long int starttime = get_time_msecond();

			NetTime frameSyncDataTime = get_time_msecond();
			ILOG("CO DB begin immediate transaction");
			rc = sqlite3_exec(mpu->pDB , "begin immediate transaction" , 0 , 0 ,NULL);
			if(SQLITE_OK!=rc)
			{
				ELOG("CO DB begin immediate transaction error rc=%d",rc);
				while(TRUE)
				{
					rc = sqlite3_exec(mpu->pDB , "begin immediate transaction" , 0 , 0 ,NULL);
					if(SQLITE_OK==rc)
					{
						break;
					}
					sleep(1);
				}
			}
			ILOG("CO DB begin immediate transaction sucess");
			ILOG("replay DB begin immediate transaction");
			rc = sqlite3_exec(mpu->pDB_replay , "begin immediate transaction" , 0 , 0 ,NULL);
			if(SQLITE_OK!=rc)
			{
				ELOG("replay DB begin immediate transaction error rc=%d",rc);
				while(TRUE)
				{
					rc = sqlite3_exec(mpu->pDB , "begin immediate transaction" , 0 , 0 ,NULL);
					if(SQLITE_OK==rc)
					{
						break;
					}
					sleep(1);
				}
			}
			ILOG("replay DB begin immediate transaction success");
			if((deal_number = mpuAnalyzer_process_batch_master(mpu, PERIOD_PROCESS_MSGNUM, &frameSyncMsg, &poolnodecount))!=false)
			{

				rc = sqlite3_exec(mpu->pDB , "commit transaction" , 0 , 0 , NULL);
				if(SQLITE_OK!=rc)
				{
					ELOG("CO DB commit transaction error rc=%d",rc);
				}
				rc = sqlite3_exec(mpu->pDB_replay , "commit transaction" , 0 , 0 , NULL);
				if(SQLITE_OK!=rc)
				{
					ELOG("replay DB commit transaction error rc=%d",rc);
				}
//				unsigned long long int endtime = get_time_msecond();
//					DLOG("<%s>mpu spend %lld mssend to process msg!",LOG_NAME_MPUAnalyzer,endtime-starttime);

				//(3).rm task : send frameSyncMsg to SLAVE machine
				frameSyncMsg.iDesModuleID = SELFRM_MODULE;
				frameSyncMsg.iMessageID = CO_RM_FRAME_SYN_IND;
				frameSyncMsg.iLength = 3+ 4+4*frameSyncMsg.listnum;
				frameSyncMsg.sMPUSyncTime = frameSyncDataTime;
				ats_co_FramesyncMsgSetDB(mpu->pDB, &frameSyncMsg);
//				    unsigned long long int endtime = get_time_msecond();
//				    DLOG("<%s>mpu spend %lld mssend to process msg!",LOG_NAME_MPUAnalyzer,endtime-starttime);
			}
			else
			{
//					DLOG(
//							"<%s>ReceiverPool has no msg in this period!",LOG_NAME_MPUAnalyzer);
			}
			++period_count;
			++period_send_heart;
			++period_setoracle_count;
			if(period_count%5 == 0)/*5*300ms=1.5S send alarm to oracle*/
			{
				sqlite3_close(mpu->pDB);/*close db,the shell will operator the db*/
				mpu->pDB = NULL;
				sqlite3_close(mpu->pDB_replay);
				mpu->pDB_replay = NULL;

				/*update alarmlist to oracle*/
				char command[MAX_SQL_STR_LEN];
				memset(command, 0, MAX_SQL_STR_LEN);
				DLOG("bash %s %s %s  > /dev/null", DUMPSH_PATH, DB_NAME, TABLE_NEED_CLEAR_SEC);
				sprintf(command, "bash %s %s %s", DUMPSH_PATH, DB_NAME, TABLE_NEED_CLEAR_SEC);
				DLOG("command=%s",command);
				//int returnValue=system(command);
				//DLOG("AlarmList =%d",returnValue);

				memset(command, 0, MAX_SQL_STR_LEN);
				sprintf(command, "bash %s  %s & > /dev/null", SYNC_ORCL,TABLE_NEED_CLEAR_SEC);
				//system(command);
				DLOG("<%s>mpu output %s SQL Text sucess!",LOG_NAME_MPUAnalyzer,"AlarmList");

				/*update ATT to oracle*/
				char command_shell[MAX_SQL_STR_LEN];
				memset(command_shell, 0, MAX_SQL_STR_LEN);
				sprintf(command_shell, "bash %s %s %s > /dev/null", DUMPSH_PATH, DB_NAME, TABLE_NEED_CLEAR_ONE);
				//system(command_shell);
				DLOG("ATT");
				memset(command_shell, 0, MAX_SQL_STR_LEN);
				sprintf(command_shell, "bash %s  %s & > /dev/null", SYNC_ORCL,TABLE_NEED_CLEAR_ONE);
				//system(command_shell);
				DLOG("<%s>mpu output %s SQL Text sucess!",LOG_NAME_MPUAnalyzer,"ATT");

				unsigned long long int endtime = get_time_msecond();
				//DLOG("<%s>mpu output command: %s",LOG_NAME_MPUAnalyzer, command);
				DLOG("<%s>mpu spend %lld mssend to output SQL Text!",LOG_NAME_MPUAnalyzer,endtime-starttime);
				period_setoracle_count=0;
			}
			else if(period_count == 18)/*20*300ms=6s send net status to HMI*/
			{
				int net_re = mpuSender_SendNetStatus_process (mpu->pDB);
				if(net_re == -1)
				{
					ELOG("mpuSender_SendNetStatus_process :error");
				}
				sqlite3_close(mpu->pDB);
				mpu->pDB = NULL;



				 mpuThreadSyncMutex_lock(&mpu->pMPUExtesionRead->extesionread_mutex);
				 sqlite3_close(mpu->pDB_replay);
				 replaydb_close_flag = 1;
				 replay_readthead_can_copy_db = 1;
				 mpuThreadSyncMutex_unlock(&mpu->pMPUExtesionRead->extesionread_mutex);
				 ILOG("send net status process");
				 period_count = 0;
			 }
			 else
			 {
				 sqlite3_close(mpu->pDB_replay);
				 mpu->pDB_replay = NULL;

				int closedb_result = sqlite3_close(mpu->pDB);
				mpu->pDB = NULL;
				if(closedb_result != SQLITE_OK)
				DLOG("MPU close db error!");
			 }

			//add send cas heartbeat to fep msg by quj
			if( period_send_heart >= 3)
			{
				ILOG("send heart beat");
				adapter_tmt_send_CasheartbeatMSG();
				period_send_heart = 0;
			}
			while(0 != sem_post(semofmpu2cas))
			{
				ELOG("sem_post(semofmpu2cas)5 errno=%d! ",errno);
				sem_close(semofmpu2cas);
				semofmpu2cas = sem_open(SEM_NAME_NOTIFY_CAS, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , 0);
				if(0 != semofmpu2cas)
				{
					ELOG("sem_open(semofmpu2cas)6 errno=%d! ",errno);

				}
				usleep(1000);
			}

			 //   sem_post(semofmpu2cas);
			    DLOG("<%s>mpu notify cas sucess!",LOG_NAME_MPUAnalyzer);

		} //for master processing

		//2------> analyzer processing as slave mode
		else if (info.iMPUMasterSlaveID == SLAVE)
		{
//			ILOG("NOW IS SLAVE");
			isInSLAVE=TRUE;
			if(info.slaveReceiveFile == TRUE)
			{
				ILOG("NOW IS SLAVE1111");
				//start to load sql.db from indicated dir
				copy_sourceFile_to_destFile(ATS_RM_DB_NAME, ATS_WORK_DB_NAME);

				sqlite3 *db_temp = 0;
				//clear pool by delete small frame msgnode
				if(sqlite3_open(DB_NAME, &db_temp) != SQLITE_OK)
				{
					sqlite3_close(db_temp);
					DLOG("mpu open db error!");
//					return NULL;
				}
				ILOG("NOW IS SLAVE222");
				//mpu->pDB = db_temp;
				ats_co_FramesyncMsgGetDB(db_temp, &frameSyncMsg);
				mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);
				mpuReceiverPool_MSG_deleteSmallSeq_byMultiMsg(mpu->pMPUCenter->pReceiverPool,
						frameSyncMsg.iHostID, frameSyncMsg.seqNo,frameSyncMsg.MainOrBak, frameSyncMsg.listnum);
				mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);

				int closedb_result = sqlite3_close(db_temp);
				if(closedb_result != SQLITE_OK)
					DLOG("Slave_MPU close db error!");

				DLOG("222");
				MPU_RM_SYN_FILE_LOAD_CNF fileloadcnfmsg;
				fileloadcnfmsg.iDesModuleID = SELFRM_MODULE;
				fileloadcnfmsg.iMessageID = CO_RM_SYN_FILE_LOAD_CNF;
				fileloadcnfmsg.iLength = 0;
				DLOG("333");
				mpu_rm_syn_file_load_cnf(&fileloadcnfmsg);//send file load cnf to rm
				DLOG("444");

				mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stMasterSlaveSyncMutex);
				mpu->pMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFile = FALSE;
				//mpu->pMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = SLAVE;
				mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stMasterSlaveSyncMutex);
				DLOG("555");
			}
			else
			{
//				ILOG("SLAVE wait RM");
				usleep(1000);
			}

		} //for slave processing

		//3------> analyzer processing as temp mode
		else
		{
			if(info.iMPUMasterSlaveID == MCHANGINGSWITCH)
			{
				//ILOG("NOW IS MASTER--SWITCH");
				if(alreadySendLockRsptoRM == 0)
				{
					MPU_RM_LOCK_RSP lockrspmsg;
					lockrspmsg.iDesModuleID = SELFRM_MODULE;
					lockrspmsg.iMessageID = CO_RM_LOCK_RSP;
					lockrspmsg.iLength = 1;
					lockrspmsg.iLockReason = 1;
					mpu_rm_lock_rsp(&lockrspmsg);//send chang rsp to rm
					alreadySendLockRsptoRM = 1;
				}


				if(changeCountTimes == 0)
				{
					starttime = get_time_msecond();
					changeCountTimes++;
				}

				if(expire_flag == 0)
				{
					endtime = get_time_msecond();
					if((endtime - starttime) >= 10000)
					{
						MPU_RM_LOCK_EXPIRED_IND lockexpiredmsg;
						lockexpiredmsg.iDesModuleID = SELFRM_MODULE;
						//lockexpiredmsg.iMessageID = CO_RM_LOCK_RSP;
						lockexpiredmsg.iMessageID = CO_RM_LOCK_EXPIRED_IND;
						lockexpiredmsg.iLength = 1;
						lockexpiredmsg.iLockReason = 1;
						mpu_rm_lock_expired_ind(&lockexpiredmsg);//send change expire to rm
						expire_flag = 1;
					}
				}
				usleep(1000);

			}

			if(info.iMPUMasterSlaveID == MCHANGINGINIT)
			{
				//ILOG("NOW IS MASTER--INIT REQURE");
//				ILOG("alreadySendLockRsptoRM--%d",alreadySendLockRsptoRM);

				if(alreadySendLockRsptoRM == 0)
		        {
					DLOG("<%s>mpu start to wait semofcalreadySendLockRsptoRMas2mpu!",LOG_NAME_MPUAnalyzer);
					sem_wait(semofcas2mpu);
					DLOG("<%s>mpu receive notify from cas,start analyse msg!",LOG_NAME_MPUAnalyzer);
					//start to copy sql.db to indicated dir
					copy_sourceFile_to_destFile(ATS_WORK_DB_NAME, ATS_RM_DB_NAME);
					MPU_RM_LOCK_RSP lockrspmsg;
					lockrspmsg.iDesModuleID = SELFRM_MODULE;
					lockrspmsg.iMessageID = CO_RM_LOCK_RSP;
					lockrspmsg.iLength = 1;
					lockrspmsg.iLockReason = 2;
					mpu_rm_lock_rsp(&lockrspmsg);//send chang rsp to rm
					alreadySendLockRsptoRM = 1;

					Master_switch_init_finish = 1;
		        }

				changeCountTimes++;
				if(changeCountTimes >= 10000)
				{
					ILOG("lockexpiredmsg");
					MPU_RM_LOCK_EXPIRED_IND lockexpiredmsg;
					lockexpiredmsg.iDesModuleID = SELFRM_MODULE;
					//lockexpiredmsg.iMessageID = CO_RM_LOCK_RSP;
					lockexpiredmsg.iMessageID = CO_RM_LOCK_EXPIRED_IND;
					lockexpiredmsg.iLength = 1;
					lockexpiredmsg.iLockReason = 2;
					mpu_rm_lock_expired_ind(&lockexpiredmsg);//send change expire to rm
					changeCountTimes = 0;
				}
				usleep(1000);
			}

			if(info.iMPUMasterSlaveID == SCHANGINGINIT)
			{
				if(info.slaveReceiveFile == TRUE)
				{
					//start to load sql.db from indicated dir
					copy_sourceFile_to_destFile(ATS_RM_DB_NAME, ATS_WORK_DB_NAME);
					MPU_RM_SYN_FILE_LOAD_CNF fileloadcnfmsg;
					fileloadcnfmsg.iDesModuleID = SELFRM_MODULE;
					fileloadcnfmsg.iMessageID = CO_RM_SYN_FILE_LOAD_CNF;
					fileloadcnfmsg.iLength = 0;
					mpu_rm_syn_file_load_cnf(&fileloadcnfmsg);//send file load cnf to rm

					sqlite3 *db_temp = 0;
					if(sqlite3_open(DB_NAME, &db_temp) != SQLITE_OK)
					{
						sqlite3_close(db_temp);
						DLOG("mpu open db error!");
					}
					//clear pool by delete small frame msgnode
					int getdbrc_temp = ats_co_FramesyncMsgGetDB(db_temp, &frameSyncMsg);
					if(getdbrc_temp != -1)
					{
					   mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);
					   mpuReceiverPool_MSG_deleteSmallSeq_byMultiMsg(mpu->pMPUCenter->pReceiverPool,
							frameSyncMsg.iHostID, frameSyncMsg.seqNo,frameSyncMsg.MainOrBak, frameSyncMsg.listnum);
					   mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
					}
					int closedb_result = sqlite3_close(db_temp);
					if(closedb_result != SQLITE_OK)
						DLOG("SCHANGINGINIT close db error!");

					//role change to slave myself
					mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stMasterSlaveSyncMutex);
					mpu->pMPUCenter->pMasterSlaveSyncInfo->slaveReceiveFile = FALSE;
					mpu->pMPUCenter->pMasterSlaveSyncInfo->iMPUMasterSlaveID = SLAVE;
					mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stMasterSlaveSyncMutex);
				}
				else
				{
					usleep(1000);
				}
			}

		}

	} //for while
	return mpu->pMPUAnalyzer;
}
/*******************************************************************
 Function name			mpuAnalyzer_initialize
 description		    init cas analyzer msg server
 parameter
 MPU *				IN/OUT 		mpu
 Return value
 true or false

 *******************************************************************/
MPUAnalyzer*
mpuAnalyzer_initialize(MPU *mpu) {
	int result;
	mpu->pMPUAnalyzer = malloc(sizeof(MPUAnalyzer));
	memset(mpu->pMPUAnalyzer, 0, sizeof(MPUAnalyzer));

	//1.create thread mutex
	mpuThreadSyncMutex_create(&mpu->pMPUAnalyzer->analyzer_mutex);
	//2.create receiver thread
	result = pthread_create(&mpu->pMPUAnalyzer->MPUAnalyzerThread, NULL,
			mpuAnalyzer_running, mpu);
	//receiver_thread(mpu);
	if(result == 0)
		return mpu->pMPUAnalyzer;
	else
		return NULL;
}
/*******************************************************************
 Function name			mpuAnalyzer_free
 description		    free cas analyzer msg server
 parameter
 MPUAnalyzer *				IN/OUT 		analyzer
 Return value
 true or false

 *******************************************************************/
int mpuAnalyzer_free(MPUAnalyzer *analyzer) {
	mpuThreadSyncMutex_destroy(&analyzer->analyzer_mutex);
	return true;
}
/*******************************************************************
 Function name			mpuAnalyzer_pause
 description		    pause cas analyzer msg server
 parameter
 MPUAnalyzer *				IN/OUT 		analyzer
 Return value
 true or false

 *******************************************************************/
int mpuAnalyzer_pause(MPUAnalyzer *analyzer) {

	return true;
}

/*******************************************************************
 Function name			mpuAnalyzer_resume
 description
 parameter
 MPUAnalyzer *				IN/OUT 		analyzer
 Return value
 true or false

 *******************************************************************/
int mpuAnalyzer_resume(MPUAnalyzer *analyzer) {

	return true;
}

