/*******************************************************************
 filename      MPUThreadSyncMutex.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUTHREADSYNCMUTEX_H_
#define MPUTHREADSYNCMUTEX_H_

#include <pthread.h>

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUTheadSyncMutex
{
  pthread_mutex_t *thread_mutex;
  pthread_cond_t *thread_cond;
} MPUTheadSyncMutex;

//����������
int
mpuThreadSyncMutex_create(MPUTheadSyncMutex *mutex);
//��
int
mpuThreadSyncMutex_lock(MPUTheadSyncMutex *mutex);
//����
int
mpuThreadSyncMutex_unlock(MPUTheadSyncMutex *mutex);
//����
int
mpuThreadSyncMutex_trylock(MPUTheadSyncMutex *mutex);
//�ͷ���
int
mpuThreadSyncMutex_destroy(MPUTheadSyncMutex *mutex);

#ifdef  __cplusplus
}
#endif

#endif /* MPUTHREADSYNCMUTEX_H_ */
