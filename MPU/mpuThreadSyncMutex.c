/*******************************************************************
filename      mpuThreadSyncMutex.c
author        root
created date  2012-1-9
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/


#include "include/MPU.h"


int	mpuThreadSyncMutex_create(MPUTheadSyncMutex  *mutex)
{


   mutex->thread_mutex=malloc(sizeof(pthread_mutex_t));

   //mutex->thread_cond=malloc(sizeof(pthread_cond_t));
   pthread_mutex_init(mutex->thread_mutex, NULL);
  // pthread_cond_init(mutex->thread_cond, NULL);

   return true;
}

int	mpuThreadSyncMutex_lock(MPUTheadSyncMutex  *mutex)
{

	  pthread_mutex_lock(mutex->thread_mutex);

		return true;
}

int	mpuThreadSyncMutex_unlock(MPUTheadSyncMutex  *mutex)
{

		pthread_mutex_unlock(mutex->thread_mutex);
		return true;
}

int	mpuThreadSyncMutex_trylock(MPUTheadSyncMutex  *mutex)
{

		pthread_mutex_trylock(mutex->thread_mutex);


		return true;
}

int	mpuThreadSyncMutex_wait(MPUTheadSyncMutex  *mutex)
{


		pthread_cond_wait(mutex->thread_cond,mutex->thread_mutex);

		return true;
}


int	mpuThreadSyncMutex_destroy(MPUTheadSyncMutex  *mutex)
{


		pthread_mutex_destroy(mutex->thread_mutex);
		//pthread_cond_destroy(mutex->thread_cond);
		//free(mutex->thread_cond);
		free(mutex->thread_mutex);
		//free(mutex);

		return true;
}





