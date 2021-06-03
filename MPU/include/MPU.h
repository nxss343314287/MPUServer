/*******************************************************************
 filename      MPU.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef _MPU_H_
#define _MPU_H_

//#include "assert.h"
#include "MPUtility.h"
#include "MPUMessage.h"
#include "MPUConvert.h"
#include "MPUCenter.h"
#include "MPUMasterSlaveSync.h"
#include "MPUReceiver.h"
#include "MPUReceiverPool.h"
#include "MPUSender.h"
#include "MPUSenderPool.h"
#include "MPUThreadRegistrationInfoList.h"
#include "MPUThreadScheduler.h"
#include "MPUThreadSyncMutex.h"
#include "MPUThreadExtesionRead.h"
#include "MPUThreadExtesionWrite.h"
#include "MPUAnalyzer.h"
//#include "../../log.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

#define   LOG_CATEGORY    "cas"
//#define   DB_NAME         "ATRSQL.db"
//#define   BUFFER_SIZE   	2048
#define   POOL_SIZE        10240
#define   ANALYZER_SIZE    100
#define   ANALYZER_PERIOD  400
#define   MPU_PERIOD       400
#define   CLOCK_DELAY      20
#define   RECEIVER_TIMEINTER  2000
#define   ANALYZER_TIMEINTER  4000
#define   ADB_TRAININFO_UNIT_LEN 26  //added by lfc
#define   INTERLOCK_REGION_COUNT 4
#define   INTERLOCK_EQUIPMENT_ID_BASE 59
#define   DNP_PROTOCOL_TYPE 12
#define   PL_ACTIVE FALSE


//#define   CAS_TRC_SWITCH 0 //added by quj 0 for CAS
                           //             1 for TRC
//#define	  CAS_MODE 0
//#define   TRC_MODE 1

//#define   SYSTEM_MODE 2        //CBTC or CIRCUIT added by lfc 20121211
//#define   MODE_CBTC 1
//#define   MODE_CIRCUIT 2
#define ARRIVAL 1
#define DEPARTURE 2

/*************************************************************/
#define LOCAL_OR_CENTER 1 /*1:center; 0:local*/
#define LOCAL_TRC 0
#define CENTER_CAS 1

#define PROJECT_MODE 0 /*0:CBTC 1:ITC_HEB1;2:ITC_NJ10*/
#define CBTC 0
#define ITC_HEB1 1
#define ITC_NJ10 2
/************************************************************/

#define   LOGICAL_FREE 1
#define   LOGICAL_OCCUPY 0
#define   AXLE_FREE 2
#define   AXLE_OCCUPY 1

#define   ATS_WORK_DB_NAME      "/dev/shm/ATSSQL.db"
#define   ATS_RM_DB_NAME        "/home/ATS/ATSSQL.db"/*used for RM file save*/

int MpuStartFlag_i;/*0:no finish start ;1:finish start*/

int MpuFinishLoadFlag_i;/*0:no finish start;4:finish start*/

int MpuReceiveLoadNum_i;/*MPU received load end number*/



#define MPU_FINISH_LOAD_NUM 4 /*HaEbin=4;SZL2=2;NJ3=3;NJ10=2*/
#define MPU_FINISH_LOAD_NUM_TRC 1
typedef enum {
	NOROLE = 0,
    MASTER = 1,
    SLAVE = 2,
    MCHANGINGSWITCH,
    MCHANGINGINIT,
    SCHANGINGSWITCH,
    SCHANGINGINIT,
  }ROLE;


typedef struct _MPU
{

  sqlite3 *pDB;
  sqlite3 *pDB_replay;

  log4c_category_t  *pLogger;

  MPUTheadSyncMutex log_mutex;

     //��Ԫ����
  MPUCenter *pMPUCenter;
  //����ͬ���߳�
  MPUMasterSlaveSync *pMPUMasterSlaveSync;
  MPUThreadScheduler *pThreadScheduler;

  //�����߳�
  MPUReceiver *pMPUReceiver;
  //�����߳�
  MPUAnalyzer *pMPUAnalyzer;

  MPUThreadExtesionRead *pMPUExtesionRead;

  MPUThreadExtesionWrite *pMPUExtesionWrite;

  //�����߳�
  //MPUSender *pMPUSender;

}MPU;

//��ʼ��MPU
int
mpu_initialize(MPU *pMPU,sqlite3 *db, log4c_category_t *log);
//�ͷ�MPU
int
mpu_free(MPU *pMPU);

int
mpu_start(MPU *pMPU, sqlite3 *db, log4c_category_t *log);

int
mpuMasterSlaveSyncInfo_get(MPUCenter *center, MPUMasterSlaveSyncInfo *info);


//����MPU�����߳�
MPUReceiver* mpuReceiver_initialize(MPU *mpu);


//�ͷ�MPU�����߳�
int
mpuReceiver_free(MPUReceiver *receiver);
//MPU��������ͣ
int
mpuReceiver_pause(MPUReceiver *receiver);
//MPU��������λ

int
mpuReceiver_resume(MPU *mpu);


MPUAnalyzer*
mpuAnalyzer_initialize(MPU *mpu);
//�ͷŷ����߳�


MPUMasterSlaveSync * mpuMasterSlaveSync_initialize(MPU  *mpu);
int mpuMasterSlaveSync_free(MPUMasterSlaveSync  *synchronizer);

MPUThreadExtesionRead* mpuThreadExtesionRead_initialize(MPU *mpu);

MPUThreadExtesionWrite* mpuThreadExtesionWrite_initialize(MPU *mpu);


#ifdef  __cplusplus
}
#endif

#endif

