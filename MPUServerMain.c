#include "log.h"
//#include "CO/ats_co_sql.h"
#include "MPU/include/MPU.h"

sqlite3 *db = 0;
extern log4c_category_t *log_category_p;
MPU myMPU;

int test_myself = 0;
int main() {
	if (log4c_init())
	{
		printf("log4c_init() failed");
	}



	log_set_category("mpuserver");


	//int k = sqlite3_open("/dbofwork/ATRSQL.db", &db);
	int k = sqlite3_open(DB_NAME, &db);
	sqlite3_exec(db,"PRAGMA synchronous=OFF;",0,0,0);
	if(k == 0)
		printf("open sqlite3 sucess!\n");
	else
	{
		printf("open sqlite3 failed! error=%d\n",k);
		DLOG("open sqlite3 failed! error=%d",k);
		return -1;
	}

	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "delete from ATRInputTask");
	sqlite3_exec(db, sqlstr, 0, 0,NULL);
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "delete from TMTInputTask");
	sqlite3_exec(db, sqlstr, 0, 0,NULL);
	sqlite3_close(db);

//	int len = 0;
//	sqlite3_exec(db , "begin transaction" , 0 , 0 ,NULL);
//		for(;len<100;len++)
//		{
//			memset(sqlstr, 0, MAX_SQL_STR_LEN);
//			sprintf(sqlstr, "insert into LogicalTrack(TrackID) values('%d')", len);
//			sqlite3_exec(db, sqlstr, 0, 0, NULL);
//		}
//	sqlite3_exec(db , "commit transaction" , 0 , 0 , NULL);

	if(test_myself == 1)
		mpu_initialize(&myMPU,NULL,log_category_p);
	else
	{
		int rc = mpu_start(&myMPU,NULL,log_category_p);
		if(rc == true)
			printf("MPUServer Start!\n");
		else
			printf("MPUServer Can not Start!\n");
	}


	while(1)
	{
		sleep(-1);
	}

	sqlite3_close(db);
	mpu_free(&myMPU);
	return 1;
}
