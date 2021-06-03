/*******************************************************************
 filename      MPUThreadRegistrationInfoList.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUREGISTRATIONINFOLIST_H_
#define MPUREGISTRATIONINFOLIST_H_

#include "MPUThreadSyncMutex.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUThreadRegistrationInfo
{
  void *pvParameter; //call in
  void *pvReturn; //call out
  void *pvProcessFun;
  pthread_t sThreadHandle;
  short bIsActive;
  NetTime sRegistrationTime;
  NetTime sUpdateTime;
  struct _MPUThreadRegistrationInfo *pNext;
} MPUThreadRegistrationInfo;

typedef struct _MPUThreadRegistrationInfoList
{
  MPUThreadRegistrationInfo *header;
  MPUThreadRegistrationInfo *tailer;
  MPUTheadSyncMutex stThreadRegistrationInfoMutex;
} MPUThreadRegistrationInfoList;

//��ע���ע��
int
mpuThreadRegistrationInfoList_create(MPUThreadRegistrationInfoList *list,MPUThreadRegistrationInfo *info);
int
mpuThreadRegistrationInfoList_destroy(MPUThreadRegistrationInfoList *list);
int
mpuThreadRegistrationInfoList_register(MPUThreadRegistrationInfoList *list,
    MPUThreadRegistrationInfo *info);
//ע��
int
mpuThreadRegistrationInfoList_unregister(MPUThreadRegistrationInfoList *list,
    MPUThreadRegistrationInfo *info);
//ע�����
int
mpuThreadRegistrationInfoList_check_alive(MPUThreadRegistrationInfoList *list,
    MPUThreadRegistrationInfo *info);
int
mpuThreadRegistrationInfoList_update(MPUThreadRegistrationInfoList *list,
    MPUThreadRegistrationInfo *info);
//ע����Ϣ�޸�

#ifdef  __cplusplus
}
#endif

#endif /* MPUREGISTRATIONINFOLIST_H_ */
