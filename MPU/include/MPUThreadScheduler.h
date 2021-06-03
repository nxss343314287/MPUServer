/*******************************************************************
 filename      MPUThreadScheduler.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUSCHEDULER_H_
#define MPUSCHEDULER_H_

#include"MPUThreadSyncMutex.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUThreadScheduler
{
  pthread_t MPUThreadSchedulerThread;
  MPUTheadSyncMutex scheduler_mutex;
} MPUThreadScheduler;

//��ʼ��
MPUThreadScheduler *
mpuThreadScheduler_initialize(MPUThreadScheduler *pThreadScheduler);
//�ͷ�
int
mpuThreadScheduler_free(MPUThreadScheduler *pThreadScheduler);
//��ͣ
int
mpuThreadScheduler_pause(MPUThreadScheduler *pThreadSheduler);
//��λ
int
mpuThreadScheduler_resume(MPUThreadScheduler *pThreadScheduler);

#ifdef  __cplusplus
}
#endif

#endif /* MPUSCHEDULER_H_ */
