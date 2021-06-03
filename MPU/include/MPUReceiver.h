/*******************************************************************
 filename      MPUReceiver.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPURECEIVER_H_
#define MPURECEIVER_H_

#include "MPUThreadSyncMutex.h"
//#include "MPU.h"

#define BUFFER_SIZE  2048000

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUReceiver
{

  //int pipe;
  BYTE recv_buf[BUFFER_SIZE];
  int len;


  pthread_t MPUReceiverThread;
  MPUTheadSyncMutex receiver_thread_mutex;
} MPUReceiver;



#ifdef  __cplusplus
}
#endif

#endif /* MPURECEIVER_H_ */
