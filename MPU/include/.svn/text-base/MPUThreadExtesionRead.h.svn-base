/*******************************************************************
 filename      MPUThreadExtesionRead.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUEXTESIONREAD_H_
#define MPUEXTESIONREAD_H_

#include"MPUThreadSyncMutex.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUThreadExtesionRead
{
  pthread_t MPUThreadExtesionReadThread;
  MPUTheadSyncMutex extesionread_mutex;
} MPUThreadExtesionRead;



int mpuThreadExtesionRead_free(MPUThreadExtesionRead *thread_extesionread);

int mpuThreadExtesionRead_pause(MPUThreadExtesionRead *thread_extesionread);

int mpuThreadExtesionRead_resume(MPUThreadExtesionRead *thread_extesionread);


#ifdef  __cplusplus
}
#endif

#endif /* MPUEXTESIONREAD_H_ */
