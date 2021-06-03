/*******************************************************************
filename      mpuThreadRegistrationInfoList.c
author        root
created date  2012-1-9
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/

#include "include/MPU.h"


int mpuThreadRegistrationInfoList_create(MPUThreadRegistrationInfoList *list,MPUThreadRegistrationInfo *info)
{


		return true;
}

int mpuThreadRegistrationInfoList_destroy(MPUThreadRegistrationInfoList *list)
{
  if (list==NULL)
      {
        return true;
      }

  while(list->header!=NULL)
            {
      free(list->header);
      free(list->tailer);
            }

    mpuThreadSyncMutex_destroy(&(list->stThreadRegistrationInfoMutex));


    return true;
}


