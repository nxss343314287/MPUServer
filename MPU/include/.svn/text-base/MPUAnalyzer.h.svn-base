/*******************************************************************
 filename      MPUAnalyzer.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUANALYZER_H_
#define MPUANALYZER_H_

#include "MPUThreadSyncMutex.h"
#include "MPU.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUAnalyzer
{

  pthread_t MPUAnalyzerThread;
  MPUTheadSyncMutex analyzer_mutex;
} MPUAnalyzer;

//���������߳�

int
mpuAnalyzer_free(MPUAnalyzer *analyzer);
//�����߳���ͣ
int
mpuAnalyzer_pause(MPUAnalyzer *analyzer);
//�����̸߳�λ
int
mpuAnalyzer_resume(MPUAnalyzer *analyzer);


//extern MPUReceiverPoolNode *
//mpuRecevierPool_next(MPUReceiverPool *pool, MPUReceiverPoolNode *node);

#ifdef  __cplusplus
}
#endif

#endif /* MPUANALYZER_H_ */
