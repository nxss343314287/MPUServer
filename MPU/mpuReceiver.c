/*******************************************************************
 filename      mpuReceiver.c
 author        root
 created date  2012-1-9
 description   specifications of implemented functions
 warning
 modify history
 author        date        modify        description
 *******************************************************************/
#include "unistd.h"
#include "include/MPU.h"
#include "../log.h"
#include	 <fcntl.h>
static void *
receiver_TSfifo_running(void *arg);
const char *  LOG_NAME_MPUReceiver = "MPU.Receiver";

ATS_CO_MSG *
mpuReceiver_MsgUnpack(BYTE *packet, size_t len,MPU *mpu)
{
  //we think that buffer is serial bytes received,and length is PACKET_LEN.
  //in fact,if we received message from a pipe connected to IO module,we can
  //get the length of message.
  //unfortunately,when we read from a pipe , perhaps we don't know how to read buffer,
  //but it isn't the object of this task. here we just have known buffer and length.
  ATS_CO_MSG *this_message;
  int result = 0;

  this_message = malloc(sizeof(ATS_CO_MSG));
  memset(this_message,0,sizeof(ATS_CO_MSG));

  //initial members of message
  result = ats_co_initATSCOMessage(this_message,packet,len);
  if (result != true)
  {
	  //when net header unpack error, the buffer received will be all discard
	  if(result == ERROR_NET)
	  {
		  ELOG("<%s>netheader of msg is invalid!",LOG_NAME_MPUReceiver);
		  ats_co_freeATSCOMessage(this_message);
		  return NULL;
	  }
	  //when net header is ok, but appmsg is wrong or mpu does not containt this msg type,
	  //we only discard this singlemsg,and does not put it into the msgpool
	  if(result == false)
	  {
		  this_message->header.seqno = -1; //flag this appmsg is wrong
		  return this_message;

	  }
  }

    //ats_co_parseNetHeader(this_message, packet);

//    ILOG("<%s>packet header:seq=%d,length=%d,SrcNo=%d,DestNo=%d,Protocol=%d",
//    		LOG_NAME_MPUReceiver,
//    		this_message->header.seqno,this_message->header.datalen, (int)this_message->header.srcno,(int) this_message->header.destno,(int)this_message->header.protocoltype);





  return this_message;
}

int
fifo_daemon(MPU *mpu)
{
	fd_set fds;
	struct timeval tv;

	int i;

	/* Wait up to one mile seconds. */
	tv.tv_sec = 0;
	tv.tv_usec = RECEIVER_TIMEINTER;
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
		pthread_t MPUReceiver_TSfifo_Thread;
		int result = pthread_create(&MPUReceiver_TSfifo_Thread, NULL,receiver_TSfifo_running, mpu);

		if(result == 0)
		{
			ILOG("MPUReceiver_TSfifo_Thread is working ");
		}
		else
		{
			ILOG("MPUReceiver_TSfifo_Thread error ");
		}
	}

	if((mpu->pMPUCenter->net2io_pipe = fifo_init(NET2APP_PIPE))==false)
	{
		ELOG("<%s>open net2app FIFO error", LOG_NAME_MPUReceiver);
		return false;
	}
	else
	{
		ILOG("open net2app fifo OK");
	}
	int seqno = 0;

	//ATS_CO_MSG *this_message;
	while (1)
	{

		FD_ZERO(&fds);
		FD_SET(mpu->pMPUCenter->net2io_pipe, &fds);

		//select timeval is zero , to return immediately.
		//timeval is NULL , to blocked indefinitely
		i = select(FD_SETSIZE, &fds, (fd_set*) 0, (fd_set *) 0,(struct timeval *) NULL);

		if (i == -1)
		{
			ELOG("<%s>select receiver FIFO error ,i=%d", LOG_NAME_MPUReceiver,i);
		}


		if (FD_ISSET(mpu->pMPUCenter->net2io_pipe,&fds))
		{
			mpu->pMPUReceiver->len = read(mpu->pMPUCenter->net2io_pipe,
			mpu->pMPUReceiver->recv_buf,(size_t) BUFFER_SIZE);
			/*******************************************************
				here we should create ATS_CO_MSG node ,then add it to pool.
				When getting a packet from pipe,receiver transfer the package of bytes and
				length to buffer structure of ATS_CO_MSG handle, and add the handle to receiver
				pool as a node of linked list.so we need a caller to construct a ATS_CO_MSG here,
				also need some operations belong to it .
			*************************************************************/
			ATS_CO_MSG  *hdl;
			size_t msglenCount = 0;

			BYTE *single_packet = mpu->pMPUReceiver->recv_buf;
			//DLOG("<%s>read msg len =%d",LOG_NAME_MPUReceiver,mpu->pMPUReceiver->len);
			while(msglenCount < mpu->pMPUReceiver->len)
			{
				//int oldseqno = seqno;
				hdl=mpuReceiver_MsgUnpack(single_packet+msglenCount,mpu->pMPUReceiver->len,mpu);
				if(hdl!=NULL) //netheader is ok
				{
					seqno = hdl->header.seqno;
					mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);
					//check pool is full
					if(mpu->pMPUCenter->pReceiverPool->count>=POOL_SIZE)
					{
						ELOG("<%s>MPUReceiverPool is full,out of pool size,count=%d",LOG_NAME_MPUReceiver,
								mpu->pMPUCenter->pReceiverPool->count);
						mpuThreadSyncMutex_unlock(&mpu->log_mutex);
						mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
						sleep(10000000);
						continue;
					}

					if(hdl->header.seqno != -1)//mean: if this msg is invalid,no need to put it into receivepool
					{
						mpuReceiverPool_MSG_put(mpu->pMPUCenter->pReceiverPool,hdl);
						DLOG("Cnet receivepool count =%d",mpu->pMPUCenter->pReceiverPool->count);
					}
					else
					{
						ats_co_freeATSCOMessage(hdl);
					}
					mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
					msglenCount += hdl->header.datalen + MSG_HEADER_LEN;
				}
				else
				{
					//            	  ELOG("<%s>Error : receive invalid msg!",LOG_NAME_MPUReceiver);
					break;
				}
			}
		}
	}/****for while ***/
	return true;
}
/*******************************************************************
 Function name			receiver_TSfifo_running
 description            ts net fifo receive message thread
 parameter
 MPU *							IN/OUT			mpu
 Return value
 true or false
 *******************************************************************/
static void *
receiver_TSfifo_running(void *arg)
{
	MPU *mpu = (MPU *) arg;
	fd_set fds;
	struct timeval tv;

	int i;

	  /* Wait up to one mile seconds. */
	tv.tv_sec = 0;
	tv.tv_usec = RECEIVER_TIMEINTER;

	 if((mpu->pMPUCenter->TSnet2io_pipe = fifo_init(TSNET2APP_PIPE))==false)
	 {
		 ELOG("<%s>open TSNET2APP_PIPE FIFO error", LOG_NAME_MPUReceiver);
		 return false;
	 }
	 else
	 {
		 ILOG("open TSnet2app fifo OK");
	 }
	  int seqno = 0;

	  //ATS_CO_MSG *this_message;
	  while (1)
	  {

	      FD_ZERO(&fds);
	      FD_SET(mpu->pMPUCenter->TSnet2io_pipe, &fds);

	      i = select(FD_SETSIZE, &fds, (fd_set*) 0, (fd_set *) 0,(struct timeval *) NULL);

	      if (i == -1)
	          ELOG("<%s>select receiver FIFO error ,i=%d", LOG_NAME_MPUReceiver,i);

	      if (FD_ISSET(mpu->pMPUCenter->TSnet2io_pipe,&fds))
	      {
	    	  BYTE recv_buf[BUFFER_SIZE];
	          int len = read(mpu->pMPUCenter->TSnet2io_pipe,recv_buf,(size_t) BUFFER_SIZE);


	          ATS_CO_MSG  *hdl;
	          size_t msglenCount = 0;

	          BYTE *single_packet = recv_buf;
	          //DLOG("<%s>read msg len =%d",LOG_NAME_MPUReceiver,mpu->pMPUReceiver->len);
	          while(msglenCount < len)
	          {
	        	  //int oldseqno = seqno;
	        	  hdl=mpuReceiver_MsgUnpack(single_packet+msglenCount,len,mpu);

	              if(hdl!=NULL) //netheader is ok
	              {
	            	  seqno = hdl->header.seqno;

	                  mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);

	                  //check pool is full
	                  if(mpu->pMPUCenter->pReceiverPool->count>=POOL_SIZE)
	          	  	  {
	                	  ELOG(
	                			  "<%s>MPUReceiverPool is full,out of pool size,count=%d",
	                			  LOG_NAME_MPUReceiver,
	                			  mpu->pMPUCenter->pReceiverPool->count);
	                	  mpuThreadSyncMutex_unlock(&mpu->log_mutex);
	                	  mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
	                	  continue;
	          	  	  }

	                  if(hdl->header.seqno != -1)//mean: if this msg is invalid,no need to put it into receivepool
	                	  {
	                	  mpuReceiverPool_MSG_put(mpu->pMPUCenter->pReceiverPool,hdl);
	                	  DLOG("lnet receivepool count =%d",mpu->pMPUCenter->pReceiverPool->count);
	                	  }
	                  else
	                	  ats_co_freeATSCOMessage(hdl);

	                  mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);

                  msglenCount += hdl->header.datalen + MSG_HEADER_LEN;
              }
              else
              {
//            	  ELOG("<%s>Error : receive invalid msg!",LOG_NAME_MPUReceiver);
            	  break;
              }

          }
      }
  }/****for while ***/
  return true;
}

static void *
receiver_running(void *arg)
{
  //BYTE recv_buf[BUFFER_SIZE];
  MPU *mpu = (MPU *) arg;

  fifo_daemon(mpu);
  return (void *)mpu->pMPUReceiver;
}

//������������MPU��������������������
//MPUReceiver* mpuReceiver_initialize(MPU Receiver *receiver,int pipe, sqlite3 *pDB,log4c_category_t  *pLogger)
MPUReceiver*
mpuReceiver_initialize(MPU *mpu)
{

  int result;

  //start MPU Receiver working thread
  mpu->pMPUReceiver = malloc(sizeof(MPUReceiver));
  memset(mpu->pMPUReceiver, 0, sizeof(MPUReceiver));

  //1.create thread mutex
  mpuThreadSyncMutex_create(&mpu->pMPUReceiver->receiver_thread_mutex);
  //2.create receiver thread
  result = pthread_create(&mpu->pMPUReceiver->MPUReceiverThread, NULL,receiver_running, mpu);
  //receiver_thread(mpu);

  if(result == 0)
	  return mpu->pMPUReceiver;
  else
	  return NULL;
}

//��������MPU��������������������
int
mpuReceiver_free(MPUReceiver *receiver)
{
  mpuThreadSyncMutex_destroy(&receiver->receiver_thread_mutex);
  return true;
}

//MPU��������������������������
int
mpuReceiver_pause(MPUReceiver *receiver)
{
  return true;
}

//MPU��������������������������
int
mpuReceiver_resume(MPU *mpu)
{
  fifo_daemon(mpu);
  return true;
}
