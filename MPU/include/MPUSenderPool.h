/*******************************************************************
 filename      MPUSenderPool.h
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef MPUSENDERPOOL_H_
#define MPUSENDERPOOL_H_

#include "MPUMessage.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

typedef struct _MPUSenderPoolNode
{
  ATS_CO_MSG *pMsgHandle;
  struct _MPUSenderPoolNode *pPrev;
  struct _MPUSenderPoolNode *pNext;
} MPUSenderPoolNode;

typedef struct _MPUSenderPool
{
  MPUSenderPoolNode *pHeaderPointer;
  MPUSenderPoolNode *pProcessingPointer;
  MPUSenderPoolNode *pReceivingPointer;
  int Size;
  int count;
} MPUSenderPool;

//��ʼ��
int
mpuSenderPool_initialize(MPUSenderPool *pool);
//�ͷ�
int
mpuSenderPool_free(MPUSenderPool *pool);
//�����Ϣ�����ͳ�
int
mpuSenderPool_MSG_add(ATS_CO_MSG *handle, MPUSenderPool *pool);
//�ڷ��ͳز�����Ϣ
int
mpuSenderPool_MSG_search(ATS_CO_MSG *handle, MPUSenderPool *pool);
//�ڷ��ͳ���ɾ����Ϣ
int
mpuSenderPool_MSG_delete(ATS_CO_MSG *handle, MPUSenderPool *pool);

#ifdef  __cplusplus
}
#endif

#endif /* MPUSENDERPOOL_H_ */
