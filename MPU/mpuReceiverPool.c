/*******************************************************************
filename      mpuReceiverPool.c
author        root
created date  2012-1-9
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

#include "include/MPU.h"

const char *  LOG_NAME_MPUReceiverPool=	"MPU.ReceiverPool";

// PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP ==
static pthread_mutex_t  mpu_recv_pool_mutex = { { 0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, 0, { 0 } } };


MPUReceiverPoolNode *
mpuReceiverPoolNode_create(ATS_CO_MSG *msgHandle)
{
  MPUReceiverPoolNode *node;
  //if MSG structure of node is frank ,a node also can not be created .
  if (msgHandle == NULL  )
  return (MPUReceiverPoolNode *) NULL;
  node = malloc(sizeof(MPUReceiverPoolNode));
  memset(node, 0, sizeof(MPUReceiverPoolNode));
  node->pNext = NULL;
  node->pPrev = NULL;
  node->pMsgHandle = msgHandle;
  return node;
}

MPUReceiverPoolNode *
mpuReceiverPoolNode_destroy(MPUReceiverPoolNode *node)
{
  if(node != NULL)
  {
	  if(node->pMsgHandle != NULL)
	  {
		  ats_co_freeATSCOMessage(node->pMsgHandle);
	  }
//	if(node->pNext!=NULL)  mpuReceiverPoolNode_destroy(node->pNext);
//	if(node->pPrev!=NULL)  mpuReceiverPoolNode_destroy(node->pPrev);
		if(node->pNext != NULL)  node->pNext = NULL;
		if(node->pPrev != NULL)  node->pPrev = NULL;
		free((MPUReceiverPoolNode *)node);
		node = NULL;
  }
  return node;
}

//��ʼ�����ճ�
int
mpuReceiverPool_initialize(MPUReceiverPool *pool)
{
  //To initial a pool for receiving message from buffer of pipe,if the node given
  //is NULL, then create a pool which is contains NULL header, if the node given is
  //not NULL, then	create a pool which is contains a header node .
  pool->Size = BUFFER_SIZE;
  //if header is NULL ,to initial pool which contains NULL header.
  pool->count = 0;
  pool->pHeaderPointer = NULL;
  pool->pProcessingPointer = NULL;
  pool->pReceivingPointer = NULL;
  return true;
}

//�ͷŽ��ճ�
int
mpuReceiverPool_free(MPUReceiverPool *pool)
{
  //Cause receiving pointer is also the list tailer, by traveling pool list reversely,
  //we can free every node of pool linked-list.
  while (pool->pReceivingPointer != NULL)
    {
      if (pool->pReceivingPointer->pPrev != NULL)
              {
          pool->pReceivingPointer = pool->pReceivingPointer->pPrev;
          mpuReceiverPoolNode_destroy(pool->pReceivingPointer->pNext);
                }
      else
              {
    	  pool->pReceivingPointer=mpuReceiverPoolNode_destroy(pool->pReceivingPointer);
               }
    }
  return true;
}

MPUReceiverPoolNode *
mpuRecevierPool_prev(MPUReceiverPool *pool, MPUReceiverPoolNode *node)
{
  if (pool == NULL)
  {
      return NULL;
  }
  if(node->pPrev==NULL)
  {
	  return NULL;
  }
  return node->pPrev;
}
/*******************************************************************
 Function name			mpuReceiverPool_next
 description            get thr next node from the pool
 parameter
 MPUReceiverPool *							IN/OUT			pool
 MPUReceiverPoolNode *						IN/OUT			node
 Return value
 true or false
 *******************************************************************/
MPUReceiverPoolNode *
mpuReceiverPool_next(MPUReceiverPool *pool, MPUReceiverPoolNode *node)
{
  if ((pool == NULL)||(node==NULL))
  {
      return NULL;
  }
  if(node->pNext==NULL)
  {
	  return NULL;
  }
  return node->pNext;
}
/*******************************************************************
 Function name			mpuReceiverPool_insert
 description            insert  node into the pool
 parameter
 MPUReceiverPool *							IN/OUT			pool
 MPUReceiverPoolNode *						IN/OUT			linker
 MPUReceiverPoolNode *						IN/OUT			node
 Return value
 true or false
 *******************************************************************/
MPUReceiverPoolNode *
mpuReceiverPool_insert(MPUReceiverPool *pool, MPUReceiverPoolNode *linker,
    MPUReceiverPoolNode *node)
{
//	pthread_mutex_lock(&mpu_recv_pool_mutex);
  //if pool or node which inserted is NULL , then return NULL pointer.
  if ((pool == NULL) ||  (node == NULL))
    {
//	  pthread_mutex_unlock(&mpu_recv_pool_mutex);
      return NULL;
    }

  if(linker != NULL)
  {
	  node->pNext= linker->pNext;
	  if (linker->pNext != NULL )
	  {
		  DLOG("mpuReceiverPool_insert: seqno[%d]", node->pMsgHandle->header.seqno);
		  DLOG("mpuReceiverPool_insert: linker[%ld], linker->pPrev[%ld], linker->pNext[%ld]",
				  (int*)linker, (int*)linker->pPrev, (int*)linker->pNext);
 		  linker->pNext->pPrev = node;
	  }
	  node->pPrev= linker;
	  linker->pNext=node;
   }

//  pthread_mutex_unlock(&mpu_recv_pool_mutex);
 return node;
}

MPUReceiverPoolNode *
mpuReceiverPool_delete(MPUReceiverPool *pool, MPUReceiverPoolNode *node)
{
//	pthread_mutex_lock(&mpu_recv_pool_mutex);
  if ((pool == NULL) || (node == NULL))
    {
//	  pthread_mutex_unlock(&mpu_recv_pool_mutex);
      return NULL;
    }

  //node = mpuReceiverPool_search(pool, node);
  if(node->pNext != NULL)
  {
	  node->pNext->pPrev = node->pPrev;
  }
  else
  {
	  // the last node
	  DLOG("mpuReceiverPool_delete: last node[%ld], node->pPrev[%ld]",
			  (int*)node, (int*)node->pPrev);
	  pool->pReceivingPointer = node->pPrev;
  }
  if(node->pPrev != NULL)
  {
	  node->pPrev->pNext = node->pNext;
  }

  //if node is tail of pool , then set all of pointers to NULL.
  //if(node==pool->pHeaderPointer)

  	//if(pool->pHeaderPointer->pPrev==node)
  	if((node->pNext == NULL) && (node->pPrev == NULL))
  	{
	  	  pool->pHeaderPointer = NULL;
	  	  pool->pProcessingPointer = NULL;
	  	  pool->pReceivingPointer = NULL;
  	  }
  pool->count--;

  node->pNext = NULL;
  node->pPrev = NULL;

	node = mpuReceiverPoolNode_destroy(node);
	return node;
}

/*******************************************************************
 Function name			mpuReceiverPool_delete_RM
 description            delete msg for rm server
 parameter
 MPUReceiverPool *							IN/OUT			pool
 MPUReceiverPoolNode *						IN/OUT			node
 Return value
 true or false
 *******************************************************************/
MPUReceiverPoolNode *
mpuReceiverPool_delete_RM(MPUReceiverPool *pool, MPUReceiverPoolNode *node)
{
  if ((pool == NULL) || (node == NULL))
  {
      return NULL;
  }
  if(node->pPrev == NULL&&node->pNext!=NULL)
  {
	  pool->pHeaderPointer = node->pNext;
	  pool->pHeaderPointer->pPrev = NULL;
	  DLOG("delete header node");

  }else
  {
	  if(node->pNext != NULL)
	  {
		  node->pNext->pPrev = node->pPrev;
	  }
	  else
	  {
		  // the last node
		  DLOG("mpuReceiverPool_delete: last node[%ld], node->pPrev[%ld]",
				  (int*)node, (int*)node->pPrev);
		  pool->pReceivingPointer = node->pPrev;
		  node->pPrev->pNext = NULL;
	  }
	  if(node->pPrev != NULL)
	  {
		  node->pPrev->pNext = node->pNext;
	  }
  }
  	if((node->pNext == NULL) && (node->pPrev == NULL))
  	{
	  	  pool->pHeaderPointer = NULL;
	  	  pool->pProcessingPointer = NULL;
	  	  pool->pReceivingPointer = NULL;
  	  }
  pool->count--;
  node->pNext = NULL;
  node->pPrev = NULL;
  node = mpuReceiverPoolNode_destroy(node);
  return node;
}

MPUReceiverPoolNode *
mpuReceiverPool_search(MPUReceiverPool *pool, MPUReceiverPoolNode *node)
{
  MPUReceiverPoolNode *search;

  search = pool->pHeaderPointer;
  while (search != NULL)
    {
      //have found the same MSG bytes of node
   if (ats_co_MSG_compare(node->pMsgHandle, search->pMsgHandle) == 0)
        {
      return search;
        }
   search = search->pNext;
    }
  return NULL;
}

MPUReceiverPoolNode *
mpuReceiverPool_swap(MPUReceiverPool *pool, MPUReceiverPoolNode *src,
    MPUReceiverPoolNode *des)
{
  MPUReceiverPoolNode *tmp_pre, *tmp_next;
  //src=mpuReceiverPool_search(pool,src);
  //des=mpuReceiverPool_search(pool,des);
  if ((pool == NULL) || (src == NULL) || (des == NULL))
    return NULL;
  tmp_pre = src->pPrev;
  tmp_next = src->pNext;
  src->pPrev = des->pPrev;
  src->pNext = des->pNext;
  des->pNext = tmp_next;
  des->pPrev = tmp_pre;
  tmp_pre = NULL;
  tmp_next = NULL;
  return des;
}

int
mpuReceiverPool_check_valid(MPUReceiverPool *pool)
{

  MPUReceiverPoolNode *travel;
  if(pool->pHeaderPointer!=NULL)
  	  {
	  travel = pool->pHeaderPointer;

	  //1.Check node data is not NULL ,and travel over is successes.
	  while (travel != NULL)
	  	  {

		  assert(travel->pMsgHandle);
		  if ((travel->pMsgHandle == NULL) )
		  	  {
			  ELOG("<%s>Check ReceiverPool InValid: message handle is null!",LOG_NAME_MPUReceiverPool);
			  mpuReceiverPool_delete(pool, travel);
			  return false;
		  	  }

			if(travel->pNext!=NULL)
			{
				travel = travel->pNext;
			}
			else
			{
				break;
			}
		}

  //2.counting the number of all of nodes ,and check it .
	  //firstly,we havn't processing analyze.so we can't access processing pointer directly.
	  //this procedure can be called when we locating processing handle.
  if(pool->pHeaderPointer!=NULL)
  DLOG("<%s>Check ReceiverPool Valid",LOG_NAME_MPUReceiverPool);
  	}//for if
  return true;
}

//�����Ϣ�����ճ�
MPUReceiverPoolNode *
mpuReceiverPool_MSG_put(MPUReceiverPool *pool, ATS_CO_MSG *handle)
{
	pthread_mutex_lock(&mpu_recv_pool_mutex);
  MPUReceiverPoolNode *node;

  if ((pool == NULL) || (handle == NULL))
    {
	  pthread_mutex_unlock(&mpu_recv_pool_mutex);
	  return NULL;
    }
  //1.to create a node
  node = mpuReceiverPoolNode_create(handle);

	//2. to add to pool's tailer
	if(node!=NULL)
	if((pool->pReceivingPointer=mpuReceiverPool_insert(pool, pool->pReceivingPointer, node))!=NULL)
	{
		if(pool->pHeaderPointer==NULL)
		{
			pool->pHeaderPointer=pool->pReceivingPointer;
		}
		pool->count++;
	}

  pthread_mutex_unlock(&mpu_recv_pool_mutex);
  return pool->pReceivingPointer;
}

//�ӽ��ճ�ɾ����Ϣ
int
mpuReceiverPool_MSG_delete(MPUReceiverPool *pool, ATS_CO_MSG *handle)
{
  MPUReceiverPoolNode *node;
  //1.to search node
  node = mpuReceiverPool_MSG_get(pool, handle);
  //2.to delete it from pool's header
  if (node != NULL)
    {
      mpuReceiverPool_delete(pool, node);
    }
  else
    return false;
  return true;
}

//�ڽ��ճ��в�����Ϣ
MPUReceiverPoolNode *
mpuReceiverPool_MSG_get(MPUReceiverPool *pool, ATS_CO_MSG *handle)
{
  MPUReceiverPoolNode *search;
  search = pool->pHeaderPointer;
  while (search != NULL)
    {
      //have found the same MSG bytes of node
      if (ats_co_MSG_compare(search->pMsgHandle, handle) == 0)
        {
          return search;
        }
      search = search->pNext;
    }
  return NULL;
}

ATS_CO_MSG *
mpuReceiverPool_getMsg_byHostSeq(MPUReceiverPool *pool, int hostid, int seq)
{
  MPUReceiverPoolNode *search;
  search = pool->pHeaderPointer;
  while (search != NULL)
    {
      //have found the same MSG bytes of node
      if ((search->pMsgHandle->header.srcno == hostid) && (search->pMsgHandle->header.seqno == seq))
        {
          return search->pMsgHandle;
        }
      search = search->pNext;
    }

  return NULL;
}

int
mpuReceiverPool_MSG_deleteSmallSeq_bySingleMsg(MPUReceiverPool *pool, ATS_CO_MSG *handle)
{
	pthread_mutex_lock(&mpu_recv_pool_mutex);
	MPUReceiverPoolNode *search;
	ATS_CO_MSG *this_message;
	search = pool->pHeaderPointer;
	while (search != NULL)
	{
	      //have found the small seq MSG bytes of node
	      if ((search->pMsgHandle->header.srcno == handle->header.seqno)  && (search->pMsgHandle->header.seqno <=  handle->header.seqno))
	        {
	    	  if (NULL != mpuReceiverPool_delete(pool, search))
	    	  {
	    		  DLOG("mpuReceiverPool_MSG_deleteSmallSeq_bySingleMsg: delete success: addr[%ld], seqno = %d",
	    				  (int*)search ,search->pMsgHandle->header.seqno);
	    	  }
	        }
	      search = search->pNext;
	}
	pthread_mutex_unlock(&mpu_recv_pool_mutex);
  return true;
}

int
mpuReceiverPool_MSG_deleteSmallSeq_byMultiMsg(MPUReceiverPool *pool, int hostidlist[], int seqlist[],int MainOrBak[], int listnum)
{
	pthread_mutex_lock(&mpu_recv_pool_mutex);
	int i = 0;
	MPUReceiverPoolNode *search;
	MPUReceiverPoolNode *pre;
	int delete_flag = 0;
	int search_delete_is_head = 0;
	search = pool->pHeaderPointer;

	DLOG("db----seqno = %d, listnum = %d",seqlist[0],listnum);
	DLOG("pool count = %d",pool->count);
	while (search != NULL)
	{
		DLOG("pool----srcno = %d seqno = %d mainorbak = %d",
				search->pMsgHandle->header.srcno,
				search->pMsgHandle->header.seqno,
				search->pMsgHandle->header.mainorbak);
		search = search->pNext;
	}
	search = pool->pHeaderPointer;
	while(i < listnum)
	{
		search = pool->pHeaderPointer;
		while (search != NULL)
		{
			  //have found the small seq MSG bytes of node
			 if ( (search->pMsgHandle->header.mainorbak == MainOrBak[i]) &&
					 (search->pMsgHandle->header.srcno == hostidlist[i])  &&
					 (search->pMsgHandle->header.seqno <=  seqlist[i]))
			 {
				 if (search->pPrev != NULL)
					 pre = search->pPrev;
				 else
				 {
					 pre = search->pNext;
					 search_delete_is_head = 1;
				 }

				  delete_flag = 1;
				  DLOG("delete seqno = %d srcno=%d mainorbak=%d ",
					 search->pMsgHandle->header.seqno,
					 search->pMsgHandle->header.srcno,
					 search->pMsgHandle->header.mainorbak);
				 mpuReceiverPool_delete_RM(pool, search);

			 }
			 if(delete_flag == 0)
			 {
				 search = search->pNext;
			 }
			 else
			 {
				 if (search_delete_is_head == 1)
				 {
					 search = pre;
				 }
				 else
				 {
					 search = pre->pNext;
				 }
				 delete_flag = 0;
				 search_delete_is_head = 0;
			 }
		}
		i++;
	}

	search = pool->pHeaderPointer;
	while (search != NULL)
	{
		DLOG("pool----srcno = %d seqno = %d mainorbak = %d",
				search->pMsgHandle->header.srcno,
				search->pMsgHandle->header.seqno,
				search->pMsgHandle->header.mainorbak);
		search = search->pNext;
	}
	DLOG("1111");
	pthread_mutex_unlock(&mpu_recv_pool_mutex);
  return true;
}
