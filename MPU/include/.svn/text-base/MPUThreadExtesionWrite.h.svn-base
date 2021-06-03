/*******************************************************************
 filename      MPUThreadExtesionWrite.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUEXTESIONWRITE_H_
#define MPUEXTESIONWRITE_H_

#include"MPUThreadSyncMutex.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUThreadExtesionWrite
{
  pthread_t MPUThreadExtesionWriteThread;
  MPUTheadSyncMutex extesionwrite_mutex;
} MPUThreadExtesionWrite;



int mpuThreadExtesionWrite_free(MPUThreadExtesionWrite *thread_extesionwrite);

int mpuThreadExtesionWrite_pause(MPUThreadExtesionWrite *thread_extesionwrite);

int mpuThreadExtesionWrite_resume(MPUThreadExtesionWrite *thread_extesionwrite);


#ifdef  __cplusplus
}
#endif

#endif /* MPUEXTESIONWRITE_H_ */
