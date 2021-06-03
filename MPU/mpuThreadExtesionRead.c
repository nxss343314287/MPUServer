/*******************************************************************
filename      mpuExtesionRead.c
author        zs
created date  2012-11-3
description   copy an additional replay_db for write_phread and refresh replay_db
warning
modify history
    author        date        modify        description
*******************************************************************/

#include"include/MPU.h"

extern int replay_readthead_can_copy_db;
extern int replay_writethead_can_import_db;
extern char replay_import_result;
/*******************************************************************
 Function name		ats_co_RefreshReplayDB
 description        refresh the replay sqlite db
 parameter
 sqlite3 *				IN/OUT			db
 Return value
 true or false
 *******************************************************************/
int ats_co_RefreshReplayDB (sqlite3 *db)
{
	if ((NULL == db))
	{
		return -1;
	}
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "delete from T_INTERLOCKDATA");
    sqlite3_exec(db, sqlstr, 0, 0, NULL);

    memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "delete from T_TMTDATA");
	sqlite3_exec(db, sqlstr, 0, 0, NULL);

	return 0;
}

static void *
mpuExtesionRead_running(void *arg)
{
	MPU *mpu = (MPU *) arg;
	//three step:
	//1.copy replaydb from memory-dev/shm to hard-disk
	//2.delete data form replaydb(memory-dev/shm)
	//3.notify write-thread to import data from sqlite3-replaydb(in hard-disk) to oracle
	while (TRUE)
	{
		usleep(1000);
		int result=mpuThreadSyncMutex_trylock(&mpu->pMPUExtesionWrite->extesionwrite_mutex);
		if(0==result)
		{
			//DLOG("trylock extesionwrite_mutex success");
			mpuThreadSyncMutex_lock(&mpu->pMPUExtesionRead->extesionread_mutex);
			if(replay_readthead_can_copy_db == 1&&replay_import_result!=-1)
			{
				unsigned long long int starttime = get_time_msecond();
				copy_sourceFile_to_destFile(ATS_WORK_DB_FOR_REPLAY_NAME, ATS_IMPORTK_DB_FOR_REPLAY_NAME);
				//DLOG("read-threadcopy finish replaydb from memory-dev/shm to hard-disk");
				printf("read-threadcopy finish replaydb from memory-dev/shm to hard-disk!\n");
				sqlite3 *replay_db_dev = 0;
				int k = sqlite3_open(ATS_WORK_DB_FOR_REPLAY_NAME, &replay_db_dev);
				if(k == 0)
				{
					printf("open sqlite3_repalydb_dev sucess!\n");
				}
				else
				{
					printf("open sqlite3_repalydb_dev failed! error=%d\n",k);
					//DLOG("open sqlite3_repalydb_dev failed! error=%d",k);
					sqlite3_close(replay_db_dev);
					mpuThreadSyncMutex_unlock(&mpu->pMPUExtesionRead->extesionread_mutex);
					continue;
				}
				ats_co_RefreshReplayDB (replay_db_dev);
				sqlite3_close(replay_db_dev);
				replay_readthead_can_copy_db = 0;
				unsigned long long int endtime = get_time_msecond();
				DLOG("read-threadcopy spend %lld mssend to copy-db and refresh-devdb!",endtime-starttime);
				replay_writethead_can_import_db = 1;
				replay_import_result=-1;

			}
			mpuThreadSyncMutex_unlock(&mpu->pMPUExtesionRead->extesionread_mutex);
			mpuThreadSyncMutex_unlock(&mpu->pMPUExtesionWrite->extesionwrite_mutex);
		}
		else
		{
			DLOG("trylock extesionwrite_mutex false");
		}

	}
	return NULL;
}

MPUThreadExtesionRead* mpuThreadExtesionRead_initialize(MPU *mpu)
{
	int result;
	mpu->pMPUExtesionRead = malloc(sizeof(MPUThreadExtesionRead));
	memset(mpu->pMPUExtesionRead, 0, sizeof(MPUThreadExtesionRead));

	//1.create thread mutex
	mpuThreadSyncMutex_create(&mpu->pMPUExtesionRead->extesionread_mutex);
	//2.create receiver thread
	result = pthread_create(&mpu->pMPUExtesionRead->MPUThreadExtesionReadThread, NULL,
			mpuExtesionRead_running, mpu);
	if(result == 0)
	{
		return mpu->pMPUExtesionRead;
	}
	else
	{
		return NULL;
	}
}



int mpuThreadExtesionRead_free(MPUThreadExtesionRead *thread_extesionread)
{
	mpuThreadSyncMutex_destroy(&thread_extesionread->extesionread_mutex);
	return true;
}



int mpuThreadExtesionRead_pause(MPUThreadExtesionRead *thread_extesionread)
{

		return true;
}


int mpuThreadExtesionRead_resume(MPUThreadExtesionRead *thread_extesionread)
{

		return true;
}
