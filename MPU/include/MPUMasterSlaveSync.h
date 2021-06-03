/*******************************************************************
 filename      MPUMasterSlaveSync.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUMASTERSLAVESYNC_H_
#define MPUMASTERSLAVESYNC_H_

#include "MPUThreadSyncMutex.h"
#include "MPURMMessage.h"
//#include "MPU.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUMasterSlaveSync
{

  pthread_t MPUMasterSlaveSyncThread;
  MPUTheadSyncMutex sync_thread_mutex;
} MPUMasterSlaveSync;


//��ͣ
int mpuMasterSlaveSync_pause(MPUMasterSlaveSync *synchronizer);
//�λ
int mpuMasterSlaveSync_resume(MPUMasterSlaveSync *synchronizer);

int rm_mpu_lock_req(sqlite3 *db, RM_MPU_LOCK_REQ *msg);
int rm_mpu_unlock_ind(sqlite3 *db, RM_MPU_UNLOCK_IND *msg);
int rm_mpu_frame_syn_req(sqlite3 *db, RM_MPU_FRAME_SYN_REQ *msg);
int rm_mpu_syn_file_load_req(sqlite3 *db, RM_MPU_SYN_FILE_LOAD_REQ *msg);
int rm_mpu_role_ind(sqlite3 *db, RM_MPU_ROLE_IND *msg);

int mpu_rm_lock_rsp(MPU_RM_LOCK_RSP *msg);
int mpu_rm_lock_expired_ind(MPU_RM_LOCK_EXPIRED_IND *msg);
int mpu_rm_frame_syn_ind(MPU_RM_FRAME_SYN_IND *msg);
int mpu_rm_frame_syn_rsp(MPU_RM_FRAME_SYN_RSP *msg);
int mpu_rm_syn_file_load_cnf(MPU_RM_SYN_FILE_LOAD_CNF *msg);



#ifdef  __cplusplus
}
#endif

#endif /* MPUMASTERSLAVESYNC_H_ */
