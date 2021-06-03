/*******************************************************************
filename      mpuCOUpdateDB.c
author        root
created date  2012-7-1
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/

#include "include/MPUMessage.h"
#include "include/MPU.h"
#include "include/MPUSendMessage.h"

// mpuCOLogicaltrack..................................................
int
ats_co_TrackMsgSetDB(sqlite3 *db, ats_co_TrackMsg *message)
{
  int rc;
  char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];
  int oldFreestatus = -1;
  int changingFlag = -1;
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "select FreeStatus from LogicalTrack where TrackID = '%d'", message->trackid);
  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);

  if (SQLITE_ROW == rc)
  {
	  oldFreestatus = sqlite3_column_int(stmt, 0);

  }
  ILOG("oldFreestatus=%d,TrackID=%d",oldFreestatus, message->trackid);
  sqlite3_finalize(stmt);

  //if freeStatus is not changed or inital, keep changingflag value
  if((oldFreestatus == message->freeStatus) || ((oldFreestatus == -1) &&(message->freeStatus == 2)))
  {
	  changingFlag = 0;
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d' where TrackID = '%d'",
		  message->lockFlag, message->freeStatus,message->trackid);
  }
  else
  {
	  changingFlag = 1;
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(
	        sqlstr,
	        "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d',ChangingFlag = '%d' where TrackID = '%d'",
	        message->lockFlag, message->freeStatus, changingFlag, message->trackid);

  }
  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
    {
      ELOG("SQLerror: %s while running ats_co_TrackMsgSetDB\n", errMsg);
      return -1;
    }

  ILOG("TrackMsgSetDB sucess: trackid=%d,LockFlag= %d,FreeStatus=%d ChangingFlag=%d",
          message->trackid,message->lockFlag, message->freeStatus,changingFlag);

  if((message->leftClaim == 1) && (message->rightClaim == 1)&&
		  (oldFreestatus != -1) && (message->freeStatus == 2))//if track is unlock(not inital time) and is free,then track-status of RouteToTrack = 0
  {
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "update RouteToTrack set Status = '%d' where AxleCounterID = '%d'", 0, message->trackid);
	  sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
  }

  if((message->leftClaim == 1) && (message->rightClaim == 1)&&
		  (oldFreestatus != -1)&& (message->freeStatus == 2)) //if track is unlock(not inital time) and is free,then track direction = -1
  {
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "update LogicalTrack set Direction = '%d' where TrackID = '%d'",-1, message->trackid);
	  sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	  ILOG("trackid = %d is free and routestatus is closed, trackdirection = -1", message->trackid);
  }
  return 0;
}
/*******************************************************************
 Function name		ats_co_LogicalTrackMsgSetDB
 description		search configuration of message using message
                    used only for cbtc,added by lfc 20121211
 type and element type
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_LogicalTrackMsgSetDB(sqlite3 *db, ats_co_logicalTrackMsg *message)
{
  int rc;
  char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];
  int oldFreestatus = -1;
  int oldAtpVacancy = -1;
  int changingFlag = -1;
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "select FreeStatus,atpVacancy from LogicalTrack where TrackID = '%d'", message->trackid);
  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);

  if (SQLITE_ROW == rc)
  {
	  oldFreestatus = sqlite3_column_int(stmt, 0);
	  oldAtpVacancy = sqlite3_column_int(stmt, 1);

  }
  ILOG("oldFreestatus=%d,TrackID=%d",oldFreestatus, message->trackid);
  sqlite3_finalize(stmt);

  //if freeStatus is not changed or inital, keep changingflag value
  	    //Modified by lfc message->freeStatus =2, CBTC is same as CIRCUIT, 1 occupy and 2 free
  	    if(((oldFreestatus == message->freeStatus) || ((oldFreestatus == -1) &&(message->freeStatus == LOGICAL_FREE)))&&
  	       ((oldAtpVacancy == message->atpVacancy) || ((oldAtpVacancy == -1) &&(message->atpVacancy == 0))))
  	    {
  	  	  changingFlag = 0;
  	  	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  	  	  //For siemens CBTC system, atpVacancy status always be sent 3 times during short
  	  	  //time(100ms),and atpVacancy status was changed 2 3 3, in the second update process
  	  	  //old status will be omit, so we should save the status before last time,value 2.
  	  	  if(oldAtpVacancy == message->atpVacancy)
  	  	  {
  	  		sprintf(sqlstr, "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
				  "atpVacancy ='%d',direction='%d',TVDFailure='%d' where TrackID = '%d'" ,message->lockFlag,
				  message->freeStatus,message->atpAvailable,message->atpVacancy,message->direction,message->TVDFailure,message->trackid);

  	  	  }
  	  	  else
  	  	  {
  	  	    sprintf(sqlstr, "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
  	  			  "atpVacancy ='%d',direction='%d',TVDFailure='%d',OldStatus='%d' where TrackID = '%d'" ,message->lockFlag,
  	  			  message->freeStatus,message->atpAvailable,message->atpVacancy,message->direction,message->TVDFailure,oldAtpVacancy,message->trackid);
  	  	  }
  	    }
  	    else
  	    {
  			changingFlag = 1;
  			memset(sqlstr, 0, MAX_SQL_STR_LEN);
  			if(oldAtpVacancy == message->atpVacancy)
  			{
  				sprintf(sqlstr,"update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
  					  "atpVacancy ='%d',direction='%d',changingFlag='%d' where TrackID = '%d'" ,message->lockFlag,
  					  message->freeStatus,message->atpAvailable,message->atpVacancy,message->direction,
  					  changingFlag,message->trackid);
  			}
  			else
  			{
  			  sprintf(sqlstr,"update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
  			  "atpVacancy ='%d',direction='%d',OldStatus='%d',changingFlag='%d' where TrackID = '%d'" ,message->lockFlag,
  			  message->freeStatus,message->atpAvailable,message->atpVacancy,message->direction,oldAtpVacancy,
  			  changingFlag,message->trackid);
  			}

  	    }
  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
    {
      ELOG("SQLerror: %s while running ats_co_TrackMsgSetDB\n", errMsg);
      return -1;
    }

  ILOG("TrackMsgSetDB sucess: trackid=%d,LockFlag= %d,FreeStatus=%d, direction=%d,ChangingFlag=%d"
		  "atpVacancy=%d,atpAvailable=%d",
          message->trackid,message->lockFlag, message->freeStatus,message->direction,changingFlag
          ,message->atpVacancy,message->atpAvailable);

  return 0;
}

/*******************************************************************
 Function name			ats_co_TrackMsgGetDB
 description			get the track msg from the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_TrackMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int
ats_co_TrackMsgGetDB(sqlite3 *db, ats_co_TrackMsg *message)
{

  int rc;
  //char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];

  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  //prepare for sql statement	 	 	
  memset(sqlstr, 0, MAX_SQL_STR_LEN);

  sprintf(
      sqlstr,
      "select LockFlag, FreeStatus from LogicalTrack where TrackID = '%d'",
      message->trackid);

  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  if (rc != SQLITE_OK)
    {
	  sqlite3_finalize(stmt);
      ELOG("SQL prepare error: %s\n", sqlite3_errmsg(db));
      return -1;
    }

  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);

  if (rc == SQLITE_NOTFOUND)
    {
	  sqlite3_finalize(stmt);

      return SQLITE_NOTFOUND;
    }

  if (ncols != 1)
    {
	  sqlite3_finalize(stmt);
      //printf("SQLerror: %s while running ats_co_log_TrackMsgSetDB\n", errMsg);
      ELOG("result is not correct one\n");
      return -1;
    }

  while (rc == SQLITE_ROW)
    {

      message->lockFlag = sqlite3_column_int(stmt, 0);
      message->freeStatus = sqlite3_column_int(stmt, 1);

      rc = sqlite3_step(stmt);
    }

  sqlite3_finalize(stmt);

  return 0;
}



// mpuCOAxlecounter...................................................
int
ats_co_AxleCounterMsgSetDB(sqlite3 *db, ats_co_AxleCounterMsg *message)
{
  int rc;
  char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];
  int oldFreestatus = -1;
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "select FreeStatus from AxleCounter where AxleCounterID = '%d'", message->axleCounterId);
  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);
  if (SQLITE_ROW == rc)
  {
	  oldFreestatus = sqlite3_column_int(stmt, 0);

  }
  sqlite3_finalize(stmt);

  if(oldFreestatus == message->freeStatus)
  {
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(
      sqlstr,
      "update AxleCounter set AxleCounterFault = '%d',FreeStatus = '%d' where AxleCounterID = '%d'",
      message->axleCounterFault, message->freeStatus, message->axleCounterId);
  }
  else
  {
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(
	        sqlstr,
	        "update AxleCounter set AxleCounterFault = '%d',FreeStatus = '%d',ChangingFlag = '%d' where AxleCounterID = '%d'",
	        message->axleCounterFault, message->freeStatus, 1, message->axleCounterId);
  }
  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
    {
      ELOG("SQLerror: %s while running ats_co_AxlCounterMsgSetDB\n", errMsg);
      return -1;
    }
  return 0;
}

int
ats_co_AxleCounterMsgGetDB(sqlite3 *db, ats_co_AxleCounterMsg *message)
{

  return 0;
}



// mpuCOPlatform...................................................
int
ats_co_PlatformMsgSetDB(sqlite3 *db, ats_co_PlatformMsg *message)
{

  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  char *errMsg = NULL;
  char sqlstr[MAX_SQL_STR_LEN];
  sqlite3_stmt *stmt;
  int enforceSkip_old = 0;
  int enforceSkip_new = 0;
  int enforceHold_old = 0;
  int enforceHold_new = 0;
  char timestr[TIME_SIZE];

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
if(PROJECT_MODE == ITC_HEB1)
  {
  sprintf(sqlstr, "select ISSKIP,ISHOLD from Platform where PTIDFORJS = '%d'", message->platform);
  }
  else if(PROJECT_MODE == CBTC)
  {
	  //add by quj
  sprintf(sqlstr, "select ISSKIP,ISHOLD from Platform where PLATFORMID = '%d'", message->platform);
  }


  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  if (SQLITE_ROW == rc)
  {
	  enforceSkip_old = sqlite3_column_int(stmt, 0);
	  enforceHold_old= sqlite3_column_int(stmt, 1);
  }
  sqlite3_finalize(stmt);

  if(message->enforceSkipOrHold == 2 || message->enforceSkipOrHold == 4)
  {
	  enforceHold_new = 6;
	  enforceSkip_new=enforceSkip_old;
  }
  if(message->enforceSkipOrHold == 1 || message->enforceSkipOrHold == 3)
  {
  	  enforceSkip_new = 7;
  	  enforceHold_new=enforceHold_old;
  }
  if(message->enforceSkipOrHold == 0)
  {
	  enforceHold_new = 0;
	  enforceSkip_new = 0;
  }


  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  if(PROJECT_MODE == ITC_HEB1)
    {
	  sprintf(
	        sqlstr,
	        "update Platform set ISSKIP = '%d',ISHOLD = '%d' where PTIDFORJS = '%d'",
	        enforceSkip_new, enforceHold_new, message->platform);
    }
    else if(PROJECT_MODE == CBTC)
    {
  	  //add by quj
    	sprintf(
    	      sqlstr,
    	      "update Platform set ISSKIP = '%d',ISHOLD = '%d' where PLATFORMID = '%d'",
    	      enforceSkip_new, enforceHold_new, message->platform);
    }

//  sprintf(
//      sqlstr,
//      "update Platform set ISSKIP = '%d',ISHOLD = '%d' where PTIDFORJS = '%d'",
//      enforceSkip_new, enforceHold_new, message->platform);

  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
    {
      ELOG("SQLerror: %s while running ats_co_log_PlatformMsgSetDB\n",errMsg);
      return -1;
    }
  ILOG("PlatformMsgSetDB success: platformid=%d, ISSKIP=%d,ISHOLD= %d",
          message->platform, enforceSkip_new,enforceHold_new);

  if(PROJECT_MODE == ITC_HEB1)/*send hold and skip event to ATR*/
  {
	struct tm *tm_loc;
	time_t t;
	t = time(NULL);
	tm_loc = localtime(&t);
	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S", tm_loc);

  if(enforceHold_old == 0 && enforceHold_new == 6)
  {
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field4,Stream1) values('%d', '%d', '%d', '%s')",
			  434,10,message->platform,timestr);
	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	  if (SQLITE_OK != rc)
	  {
		  ELOG("SQLerror: %s while running ats_co_PlatformMsgSetDB", errMsg);
		  return -1;
	  }
	  DLOG("set hold event OK!");
  }
  if(enforceHold_old == 6 && enforceHold_new == 0)
    {
  	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  	  sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field4,Stream1) values('%d', '%d', '%d', '%s')",
  			  434,11,message->platform,timestr);
  	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
  	  if (SQLITE_OK != rc)
  	  {
  		  ELOG("SQLerror: %s while running ats_co_PlatformMsgSetDB", errMsg);
  		  return -1;
  	  }
  	  DLOG("set cancle hold event OK!");
    }

  if(enforceSkip_old == 0 && enforceSkip_new == 7)
      {
    	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
    	  sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field4,Stream1) values('%d', '%d', '%d', '%s')",
    			  434,12,message->platform,timestr);
    	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
    	  if (SQLITE_OK != rc)
    	  {
    		  ELOG("SQLerror: %s while running ats_co_PlatformMsgSetDB", errMsg);
    		  return -1;
    	  }
      }

  if(enforceSkip_old == 7 && enforceSkip_new == 0)
        {
      	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
      	  sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field4,Stream1) values('%d', '%d', '%d', '%s')",
      			  434,13,message->platform,timestr);
      	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
      	  if (SQLITE_OK != rc)
      	  {
      		  ELOG("SQLerror: %s while running ats_co_PlatformMsgSetDB", errMsg);
      		  return -1;
      	  }
      	DLOG("set skip event OK!");
        }
  }
  return 0;
}

int
ats_co_PlatformMsgGetDB(sqlite3 *db, ats_co_PlatformMsg *message)
{

  return 0;
}



// mpuCOSignal....................................................
int
ats_co_SignalMsgSetDB(sqlite3 *db, ats_co_SignalMsg *message)
{
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  char *errMsg = NULL;
  char sqlstr[MAX_SQL_STR_LEN];

  //prepare for sql statement
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  if(PROJECT_MODE == ITC_HEB1)
  {
	  sprintf(
	        sqlstr,
	        "update SIGNAL set SignalStatus = '%d',SignalAutoStatus = '%d',LockFlag = '%d',SignalFault = '%d' where SignalID = '%d'",
	        message->signalStatus, message->signalAutoStatus, message->lockFlag,
	        0, message->signalId);
  }
  else if(PROJECT_MODE == CBTC)
  {
	  sprintf(
	        sqlstr,
	        "update SIGNAL set SignalStatus = '%d',SignalAutoStatus = '%d',LockFlag = '%d',SignalFault = '%d' where SignalID = '%d'",
	        message->signalStatus, message->signalAutoStatus, message->lockFlag,
	        message->signalFault, message->signalId);
  }
  else
  {}


  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
    {
      ELOG("SQLerror: %s while running ats_co_SignalMsgSetDB\n", errMsg);
      return -1;
    }

  ILOG("SignalMsgSetDB sucess: signalid=%d signalStatus=%d,signalAutoStatus= %d,lockFlag=%d,signalFault=%d",
          message->signalId,message->signalStatus, message->signalAutoStatus,message->lockFlag,message->signalFault);
  return 0;
}

int
ats_co_SignalMsgGetDB(sqlite3 *db, ats_co_SignalMsg *message)
{

  return 0;
}



// mpuCOSwitch....................................................
int
ats_co_SwitchMsgSetDB(sqlite3 *db, ats_co_SwitchMsg *message)
{
	//ILOG("���������������������������������������������������������������LockFlag =%d\n", message->lockFlag);
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];
  char sqlstr1[MAX_SQL_STR_LEN];
  int oldFreestatus = -1;
  int oldAtpVacancy = -1;
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "select FreeStatus from Switch where SwitchID = '%d'", message->switchId);
  sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);
  if (SQLITE_ROW == rc)
  {
	  oldFreestatus = sqlite3_column_int(stmt, 0);
  }
   sqlite3_finalize(stmt);

   /* if(((oldFreestatus == message->freeStatus) || ((oldFreestatus == -1) &&(message->freeStatus == LOGICAL_FREE)))&&
  	       ((oldAtpVacancy == message->atpVacancy) || ((oldAtpVacancy == -1) &&(message->atpVacancy == 0))))*/

   if(((oldFreestatus == message->freeStatus) || ((oldFreestatus == -1) &&(message->freeStatus == LOGICAL_FREE)))&&
  	       ((oldAtpVacancy == message->atpVacancy) || ((oldAtpVacancy == -1) &&(message->atpVacancy == 0))))
   {

      //prepare for sql statement
      memset(sqlstr, 0, MAX_SQL_STR_LEN);
      if(oldAtpVacancy == message->atpVacancy)
	  {
		  if(PROJECT_MODE == ITC_HEB1)
			{
			  sprintf(
					   sqlstr,
					   "update Switch set BlockAgainstThrowing = '%d',ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d' where SwitchID = '%d'",
					   message->blockAgainstThrowing, message->claimStatus, message->freeStatus,
					   message->lockFlag, message->position, message->switchId);
			}
		  else if(PROJECT_MODE == CBTC)
		  {
			  sprintf(
					   sqlstr,
					   "update Switch set physicalFree= '%d', kickoffFailure= '%d',atpAvailable= '%d',"
					   "atpVacancy= '%d',pointDirection= '%d',TVDFailure= '%d',BlockAgainstThrowing = '%d',"
					   "ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d' where SwitchID = '%d'",
					   message->physicalFree,message->kickoffFailure,message->atpAvailable,message->atpVacancy,
					   message->pointDirection,message->TVDFailure,message->blockAgainstThrowing,message->claimStatus,
					   message->freeStatus,message->lockFlag, message->positionFlag, message->switchId);

			  //modified by quj 20121218
					   //message->freeStatus   message->lockFlag    message->pointDirection

				  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
				  sprintf(sqlstr1, "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
						  "atpVacancy ='%d',direction='%d',TVDFailure='%d' where TrackID = '%d'" ,message->lockFlag,
						  message->freeStatus,message->atpAvailable,message->atpVacancy,message->pointDirection,
						  message->TVDFailure,message->switchId);
				 //modified end
		  }
	   }
      else
      {
		  if(PROJECT_MODE == ITC_HEB1)
			{
			  sprintf(
					   sqlstr,
					   "update Switch set BlockAgainstThrowing = '%d',ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d' where SwitchID = '%d'",
					   message->blockAgainstThrowing, message->claimStatus, message->freeStatus,
					   message->lockFlag, message->position, message->switchId);
			}
		  else if(PROJECT_MODE == CBTC)
		  {
			  sprintf(
					   sqlstr,
					   "update Switch set physicalFree= '%d', kickoffFailure= '%d',atpAvailable= '%d',"
					   "atpVacancy= '%d',pointDirection= '%d',TVDFailure= '%d',BlockAgainstThrowing = '%d',"
					   "ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',OldStatus='%d' where SwitchID = '%d'",
					   message->physicalFree,message->kickoffFailure,message->atpAvailable,message->atpVacancy,
					   message->pointDirection,message->TVDFailure,message->blockAgainstThrowing,message->claimStatus,
					   message->freeStatus,message->lockFlag, message->positionFlag, oldAtpVacancy,message->switchId);

			  //modified by quj 20121218
					   //message->freeStatus   message->lockFlag    message->pointDirection

				  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
				  sprintf(sqlstr1, "update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
						  "atpVacancy ='%d',direction='%d',TVDFailure='%d',OldStatus='%d' where TrackID = '%d'" ,message->lockFlag,
						  message->freeStatus,message->atpAvailable,message->atpVacancy,message->pointDirection,
						  message->TVDFailure,oldAtpVacancy,message->switchId);
				 //modified end
		  }
	   }

   }
   else
   {
	   memset(sqlstr, 0, MAX_SQL_STR_LEN);
	   if(oldAtpVacancy == message->atpVacancy)
	   {
		   if(PROJECT_MODE == ITC_HEB1)
		   {
			   sprintf(
					   sqlstr,
					   "update Switch set BlockAgainstThrowing = '%d',ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',ChangingFlag = '%d' where SwitchID = '%d'",
					   message->blockAgainstThrowing, message->claimStatus, message->freeStatus,
					   message->lockFlag, message->position, 1, message->switchId);
		   }
		   else if(PROJECT_MODE == CBTC)
		   {
			   sprintf(
					   sqlstr,
					   "update Switch set physicalFree= '%d', kickoffFailure= '%d',atpAvailable= '%d',"
					   "atpVacancy= '%d',pointDirection= '%d',TVDFailure= '%d',BlockAgainstThrowing = '%d',"
					   "ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',ChangingFlag = '%d' where SwitchID = '%d'",
					   message->physicalFree,message->kickoffFailure,message->atpAvailable,message->atpVacancy,
					   message->pointDirection,message->TVDFailure,message->blockAgainstThrowing,message->claimStatus,
					   message->freeStatus,message->lockFlag, message->positionFlag, 1,message->switchId);
			   //modified by quj
			  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
			  sprintf(
					sqlstr1,"update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
					  "atpVacancy ='%d',direction='%d',changingFlag='%d',TVDFailure='%d' where TrackID = '%d'" ,message->lockFlag,
					  message->freeStatus,message->atpAvailable,message->atpVacancy,message->pointDirection,
					  1,message->TVDFailure,message->switchId);
				 //modified end
		   }
	   }
	   else
	   {
		   if(PROJECT_MODE == ITC_HEB1)
		   {
			   sprintf(
					   sqlstr,
					   "update Switch set BlockAgainstThrowing = '%d',ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',ChangingFlag = '%d' where SwitchID = '%d'",
					   message->blockAgainstThrowing, message->claimStatus, message->freeStatus,
					   message->lockFlag, message->position, 1, message->switchId);
		   }
		   else if(PROJECT_MODE == CBTC)
		   {
			   sprintf(
					   sqlstr,
					   "update Switch set physicalFree= '%d', kickoffFailure= '%d',atpAvailable= '%d',"
					   "atpVacancy= '%d',pointDirection= '%d',TVDFailure= '%d',BlockAgainstThrowing = '%d',"
					   "ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',ChangingFlag = '%d'"
					   ",OldStatus='%d' where SwitchID = '%d'",
					   message->physicalFree,message->kickoffFailure,message->atpAvailable,message->atpVacancy,
					   message->pointDirection,message->TVDFailure,message->blockAgainstThrowing,message->claimStatus,
					   message->freeStatus,message->lockFlag, message->positionFlag, 1,oldAtpVacancy,message->switchId);
			   //modified by quj
			  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
			  sprintf(
					sqlstr1,"update LogicalTrack set LockFlag = '%d',FreeStatus = '%d', atpAvailable='%d',"
					  "atpVacancy ='%d',direction='%d',changingFlag='%d',TVDFailure='%d' ,OldStatus='%d' where TrackID = '%d'"
					  ,message->lockFlag, message->freeStatus,message->atpAvailable,message->atpVacancy,message->pointDirection,
					  1,message->TVDFailure,oldAtpVacancy,message->switchId);
				 //modified end
		   }
	   }

   }
   //modified by quj
   if(PROJECT_MODE == CBTC)  //modified by lfc 20121218
   {
      rc = sqlite3_exec(db, sqlstr1, 0, 0, &errMsg);
      if (SQLITE_OK != rc)
        {
          ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
          return -1;
        }
      rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
      if (SQLITE_OK != rc)
        {
          ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
          return -1;
        }
   }
   else if(PROJECT_MODE == ITC_HEB1)
   {
      //modified end
      rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

      if (SQLITE_OK != rc)
        {
          ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
          return -1;
        }
   }
    ILOG("update Switch set physicalFree= '%d', kickoffFailure= '%d',atpAvailable= '%d',"
				   "atpVacancy= '%d',pointDirection= '%d',TVDFailure= '%d',BlockAgainstThrowing = '%d',"
				   "ClaimStatus = '%d',FreeStatus = '%d',LockFlag = '%d',PositionFlag = '%d',ChangingFlag = '%d' where SwitchID = '%d'",
				   message->physicalFree,message->kickoffFailure,message->atpAvailable,message->atpVacancy,
				   message->pointDirection,message->TVDFailure,message->blockAgainstThrowing,message->claimStatus,
				   message->freeStatus,message->lockFlag, message->positionFlag, 1,message->switchId);
  return 0;
}

/*******************************************************************
 Function name		ats_co_CrossingMsgSetDB
 description			search configuration of message using message,
                        only for cbtc.added by lfc 20121211
 type and element type
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_CrossingMsgSetDB(sqlite3 *db, ats_co_CrossingMsg *message)
{
	//ILOG("LockFlag =%d\n", message->lockFlag);
	  if ((db == NULL) || message == NULL)
	    {
	      return -1;
	    }

	  int rc;
	  char *errMsg = NULL;
	  sqlite3_stmt *stmt;
	  char sqlstr[MAX_SQL_STR_LEN];
	  char sqlstr1[MAX_SQL_STR_LEN];
	  int oldFreestatus = -1;
	  int oldAtpVacancy = -1;

	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "select FreeStatus ,atpVacancy from Crossing where CrossingID = '%d'", message->crossingIndex);
	  sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	  rc = sqlite3_step(stmt);
	  if (SQLITE_ROW == rc)
	  {
		  oldFreestatus = sqlite3_column_int(stmt, 0);
		  oldAtpVacancy = sqlite3_column_int(stmt, 1);
	  }
	   sqlite3_finalize(stmt);
	   ILOG("oldFreestatus=%d,TrackID=%d",oldFreestatus, message->crossingIndex);

	   if(((oldFreestatus == message->freeStatus) || ((oldFreestatus == -1) &&(message->freeStatus == LOGICAL_FREE)))&&
	  	       ((oldAtpVacancy == message->ATPVacancy) || ((oldAtpVacancy == -1) &&(message->ATPVacancy == 0))))
	   {

	      //prepare for sql statement
	      memset(sqlstr, 0, MAX_SQL_STR_LEN);

	      if(oldAtpVacancy == message->ATPVacancy)
	      {
			  sprintf(
					   sqlstr,
					   "update Crossing set freeStatus = '%d',overlapClaim = '%d', routeClaim = '%d',"
					   "failureIndication= '%d',blockAgainstThrowing = '%d',lockAgainstPassing = '%d',"
					   "position= '%d',direction = '%d',TVDFailure = '%d',ATPVacancy = '%d',"
					   "ChangingFlag = '%d' where CrossingID = '%d'",
					   message->freeStatus, message->overlapClaim, message->routeClaim, message->failureIndication,
					   message->blockAgainstThrowing,message->lockAgainstPassing,message->position,message->direction,
				       message->TVDFailure,message->ATPVacancy,1, message->crossingIndex);
	      }else
	      {
			  sprintf(
					   sqlstr,
					   "update Crossing set freeStatus = '%d',overlapClaim = '%d', routeClaim = '%d',"
					   "failureIndication= '%d',blockAgainstThrowing = '%d',lockAgainstPassing = '%d',"
					   "position= '%d',direction = '%d',TVDFailure = '%d',ATPVacancy = '%d',"
					   "ChangingFlag = '%d',OldStatus='%d' where CrossingID = '%d'",
					   message->freeStatus, message->overlapClaim, message->routeClaim, message->failureIndication,
					   message->blockAgainstThrowing,message->lockAgainstPassing,message->position,message->direction,
				       message->TVDFailure,message->ATPVacancy,1,oldAtpVacancy, message->crossingIndex);
	      }

    	  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
    	  sprintf(sqlstr1, "update LogicalTrack set FreeStatus = '%d', atpAvailable='%d',"
    	  	 "atpVacancy ='%d',direction='%d',TVDFailure='%d' where TrackID = '%d'" ,
    	      message->freeStatus,message->atpAvailable,message->ATPVacancy,message->direction,
    	  	  message->TVDFailure,message->crossingIndex);
          //modified end

	   }
	   else
	   {
		   memset(sqlstr, 0, MAX_SQL_STR_LEN);
		   if(oldAtpVacancy == message->ATPVacancy)
		   {
			   sprintf(
					   sqlstr,
					   "update Crossing set freeStatus = '%d',overlapClaim = '%d', routeClaim = '%d',"
					   "failureIndication= '%d',blockAgainstThrowing = '%d',lockAgainstPassing = '%d',"
					   "position= '%d',direction = '%d',TVDFailure = '%d',ATPVacancy = '%d' where CrossingID = '%d'",
					   message->freeStatus, message->overlapClaim, message->routeClaim, message->failureIndication,
					   message->blockAgainstThrowing,message->lockAgainstPassing,message->position,message->direction,
					   message->TVDFailure,message->ATPVacancy, message->crossingIndex);
			 //modified by quj
			  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
			  sprintf(
					  sqlstr1,"update LogicalTrack set FreeStatus = '%d', atpAvailable='%d',"
					  "atpVacancy ='%d',direction='%d',changingFlag='%d',TVDFailure='%d' where TrackID = '%d'" ,
					  message->freeStatus,message->atpAvailable,message->ATPVacancy,message->direction,
					  1,message->TVDFailure,message->crossingIndex);
		 //modified end
		   }
		   else
		   {
			   sprintf(
					   sqlstr,
					   "update Crossing set freeStatus = '%d',overlapClaim = '%d', routeClaim = '%d',"
					   "failureIndication= '%d',blockAgainstThrowing = '%d',lockAgainstPassing = '%d',"
					   "position= '%d',direction = '%d',TVDFailure = '%d',ATPVacancy = '%d',OldStatus='%d'"
					   " where CrossingID = '%d'",
					   message->freeStatus, message->overlapClaim, message->routeClaim, message->failureIndication,
					   message->blockAgainstThrowing,message->lockAgainstPassing,message->position,message->direction,
					   message->TVDFailure,message->ATPVacancy, oldAtpVacancy,message->crossingIndex);
			 //modified by quj
			  memset(sqlstr1, 0, MAX_SQL_STR_LEN);
			  sprintf(
					  sqlstr1,"update LogicalTrack set FreeStatus = '%d', atpAvailable='%d',"
					  "atpVacancy ='%d',direction='%d',changingFlag='%d',TVDFailure='%d' ,OldStatus='%d' "
					  "where TrackID = '%d'" ,
					  message->freeStatus,message->atpAvailable,message->ATPVacancy,message->direction,
					  1,message->TVDFailure,oldAtpVacancy,message->crossingIndex);
		 //modified end
		   }

	   }
	   //modified by quj
	   if(PROJECT_MODE == CBTC)   //modified by lfc 20121218
	   {
	     rc = sqlite3_exec(db, sqlstr1, 0, 0, &errMsg);

	   	 if (SQLITE_OK != rc)
	   	  {
	   	      ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
	   	      return -1;
	   	  }
	     rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

	   	 if (SQLITE_OK != rc)
	   	  {
	   	      ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
	   	      return -1;
	   	  }
	   }
	   else if(PROJECT_MODE == ITC_HEB1)
	   {
	   	//modified end
	      rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

	      if (SQLITE_OK != rc)
	        {
	          ELOG("SQLerror: %s while running ats_co_SwitchMsgSetDB\n", errMsg);
	          return -1;
	        }
	   }

	    ILOG("update Crossing set freeStatus = '%d',overlapClaim = '%d', routeClaim = '%d',"
				   "failureIndication= '%d',blockAgainstThrowing = '%d',lockAgainstPassing = '%d',"
				   "position= '%d',direction = '%d',TVDFailure = '%d',ATPVacancy = '%d' where CrossingID = '%d'",
				   message->freeStatus, message->overlapClaim, message->routeClaim, message->failureIndication,
				  				   message->blockAgainstThrowing,message->lockAgainstPassing,message->position,message->direction,
				  				   message->TVDFailure,message->ATPVacancy, message->crossingIndex);
	  return 0;

}

int
ats_co_SwitchMsgGetDB(sqlite3 *db, ats_co_SwitchMsg *message)
{
  return 0;
}



// mpuCOTrainPosition   task: TMT logicalProcess....................................................
int
ats_co_TrainPositionMsgSetDB(sqlite3 *db, ats_co_TrainPositionReport *message)
{

	  if ((db == NULL) || message == NULL)
	  {
	      return -1;
	  }
	  sqlite3_stmt *stmt;
	//  char sqlstr[MAX_SQL_STR_LEN];
	  int oldspeed = -1;

	  int rc;
	  char *errMsg = NULL;
	  char sqlstr[MAX_SQL_STR_LEN];

	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(sqlstr, "select carSpeed from TrainStatus where tainindex = '%d'", message->trainIndex);
	  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	  rc = sqlite3_step(stmt);

	  if (SQLITE_ROW == rc)
	  {
		  oldspeed = sqlite3_column_int(stmt, 0);

	  }
	  ILOG("oldFreestatus=%d,TrackID=%d",oldspeed, message->trainIndex);
	  sqlite3_finalize(stmt);


	  //prepare for sql statement
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(
			  sqlstr,
			  "update TrainStatus set LoclsSecured = '%d',OPRValidFlag = '%d',FrontSegID = '%d',"
			  "FrontOffset = '%d',ControlLevel = '%d',OperationMode = '%d',carSpeed = '%d',"
			  "oldspeed = '%d',TrainStandStill = '%d',DriveMode = '%d' where trainindex = '%d'",
			  message->LoclsSecured,message->OPRValidFlag,message->FrontSegID,
			  message->FrontOffset,message->ControlLevel,message->OperationMode,message->carSpeed,
			  oldspeed,message->TrainStandStill,message->trainDriveMode, message->trainIndex);

	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

	  if (SQLITE_OK != rc)
	  {
		  ELOG("SQLerror: %s while running ats_co_TrainStatusMsgSetDB\n", errMsg);
		  return -1;
	  }

	  //modify jsl 20130503
	 	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	 	  sprintf(
	 			  sqlstr,
	 			  "update Train set TrainType = '%d' where trainindex = '%d'",
	 			  message->ControlLevel,message->trainIndex);

	 	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

	 	  if (SQLITE_OK != rc)
	 	  {
	 		  ELOG("SQLerror: %s while running update train type fail\n", errMsg);
	 		  return -1;
	 	  }
	 	  else
	 	  {
	 		  ILOG("%s",sqlstr);
	 		  ILOG("SQLerror: %s while running update train type fail\n", errMsg);
	 	  }
	 	  //end

	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
	  sprintf(
			  sqlstr,
			  "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7) values('%d', '%d', '%d', '%d', '%d','%d','%d')",
			  message->msgType,message->elementType,message->trainIndex,message->carSpeed,message->ControlLevel,
			  message->FrontSegID,message->FrontOffset);
	  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	  if (SQLITE_OK != rc)
	  {
		  ELOG("SQLerror: %s while running ats_co_TrainPositionMsgSetDB", errMsg);
		  return -1;
	  }
	  ILOG("add trainposition sucess: msgtype=%d elementtype=%d trainindex=%d",message->msgType,message->elementType,message->trainIndex);
	  return 0;
}


//int ats_co_TrainPositionMsgSetDB(sqlite3 *db, ats_co_TrainPositionReport *message)
//{
//	 int rc;
//	  sqlite3_stmt *stmt;
//	  char sqlstr[MAX_SQL_STR_LEN];
//	 int position;

	  //prepare for sql statement
//	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
//	  sprintf(
//	      sqlstr,
//	      "select containtrackid from t_segmentcontain t where t.segmentid = '%d' and t.endoffset>'%d' order by t.groupindex where rownum=1",
//	      message->headSegmentID,message->headOffset);
//	  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
//	  if (SQLITE_OK != rc)
//	    {
//	      printf("SQLerror: %s while running ats_co_trainposition\n",
//	          sqlite3_errmsg(db));
//	      return -1;
//	    }
//	  rc = sqlite3_step(stmt);
//	  if (SQLITE_ROW == rc)
//	    {
//	      position = sqlite3_column_int(stmt, 0);
//	      sqlite3_finalize(stmt);
//	    }
//	  else
//	    {
//	      sqlite3_finalize(stmt);
//	      return -2; // not find data in SQL
//	    }
//	TrainIDs trainids;



	//ats_co_tra_GetTrainIDByOccupyTrackId (&trainids, db, 197);
	//ats_co_tra_SetIndex (db, trainids.trainId, message->workAreaIndex);

	//return ats_co_tra_SetPosition (db,trainids.trainId,  197);

//	return true;
//}

int ats_co_TrainPositionMsgGetDB(sqlite3 *db, ats_co_TrainPositionReport *message)
{


	return true;
}



// mpuCOTrainstatus  task: TMT logicalProcess....................................................
int
ats_co_TrainStatusMsgSetDB(sqlite3 *db, ats_co_TrainStatusMsg *message)
{

  if ((db == NULL) || message == NULL)
  {
      return -1;
  }

  int rc;
  char *errMsg = NULL;
  char sqlstr[MAX_SQL_STR_LEN];

  //prepare for sql statement
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(
		  sqlstr,
		  "update TrainStatus set service = '%d',serial = '%d',line = '%d',driver = '%d',"
		  "car1 = '%d',car2 = '%d',ebIsApplied = '%d',ebReason = '%d',"
		  "trainDoorStatus = '%d',controlLevel = '%d',obcuFailure = '%d',"
		  //add by quj 20121213
		  " CommuEstablished= '%d',DstNumber= '%d'"
		  "where trainindex = '%d'",
		  message->service, message->serial, message->line, message->driver,
		  message->car1,0, message->ebIsApplied, message->ebReason,
		  message->trainDoorStatus, message->controlLevel, message->obcuFailure,
		  //add by quj 20121213
		  message->CommuEstablished,message->DstNumber,
		  message->trainIndex);

  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  if (SQLITE_OK != rc)
  {
	  ELOG("SQLerror: %s while running ats_co_TrainStatusMsgSetDB\n", errMsg);
	  return -1;
  }

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(
		  sqlstr,
		  "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7, Field8, Field9, Field10) values('%d', '%d', '%d', '%d', '%d','%d', '%d', '%d', '%d', '%d')",
		  message->msgType,message->elementType,message->trainIndex,message->destination,message->service,message->serial,
  		  message->driver,message->line,message->car,message->driveMode);
  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
  if (SQLITE_OK != rc)
  {
	  ELOG("SQLerror: %s while running ats_co_TrainStatusMsgSetDB", errMsg);
	  return -1;
  }
  ILOG("add trainstatusmsg sucess: msgtype=%d elementtype=%d trainindex=%d",message->msgType,message->elementType,message->trainIndex);
  return 0;
}

int
ats_co_TrainStatusMsgGetDB(sqlite3 *db, ats_co_TrainStatusMsg *message)
{

  return 0;
}

int
ats_co_DirectionMsgSetDB(sqlite3 *db, ats_co_DirectionMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	int direction = 0;/*1:left ,2:right,0:default*/
	char *errMsg = NULL;
	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	if((message->leftDirection == 1) && (message->rightDirection == 0 ))
	{
		direction = 2;
		sprintf(sqlstr, "update SECTIONDIRECTIONFORJS set Direction = '%d' where SECTIONDIRECTIONINDEX = '%d'",direction, message->directionId);
	}
	if((message->leftDirection == 0) && (message->rightDirection == 1 ))
	{
		direction = 1;
		sprintf(sqlstr, "update SECTIONDIRECTIONFORJS set Direction = '%d' where SECTIONDIRECTIONINDEX = '%d'",direction, message->directionId);
	}
	if((message->leftDirection == 0) && (message->rightDirection == 0 ))
	{
		direction = 0;
		sprintf(sqlstr, "update SECTIONDIRECTIONFORJS set Direction = '%d' where SECTIONDIRECTIONINDEX = '%d'",direction, message->directionId);
	}
	if((message->leftDirection == 1) && (message->rightDirection == 1 ))
	{
		direction = 0;
		sprintf(sqlstr, "update SECTIONDIRECTIONFORJS set Direction = '%d' where SECTIONDIRECTIONINDEX = '%d'",direction, message->directionId);
		ELOG("SectionDirection is wrong");
	}

//	sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);//chanded by wangheng 2013.3.9
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_DirectionMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add SectionDirection sucess: msgtype=%d Direction=%d",message->msgType,direction);

	return 0;
}
/*******************************************************************
 Function name			ats_co_PTIMsgSetDB
 description			set the PTI msg to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_PTIMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int
ats_co_PTIMsgSetDB(sqlite3 *db, ats_co_PTIMsg *message)
{
	ILOG("Process PTI message");
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	sqlite3_stmt *stmt;
	char timestr[TIME_SIZE];
	char sqlstr[MAX_SQL_STR_LEN];
	int ptid = 0;
	int trainID = 0;
	int carID = 0;
	int crewID = 0;
	int trainIndex = 0;
	int eventType = 0;
	int oldtrainstatus = 0;

	struct tm *tm_loc;
	time_t t;
	t = time(NULL);
	tm_loc = localtime(&t);
	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S", tm_loc);
	DLOG("create time : %s", timestr);

	// handle the train id  ,car id , crew id
	trainID = message->destnum*10000 + message->tripnumThousand*1000
			+ message->tripnumH*100 + message->tripnumT*10 + message->tripnumS;
	carID = message->activeMSB*256 + message->activeLSB;
	crewID = message->crewnumH*100 + message->crewnumT*10 + message->crewnumS;

	//modified the eventType
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr,"select PTIState, associatePtid from PTITable where PTIindex = '%d'",message->PTIindex);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		ELOG("SQLerror: %s while running ats_co_TrainEventTWCMsgSetDB select PTITable", errMsg);
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW == rc)
	{
		oldtrainstatus = sqlite3_column_int(stmt, 0);
		ptid = sqlite3_column_int(stmt, 1);
		sqlite3_finalize(stmt);
	}
	if((oldtrainstatus == 1 && message->trainstatus == 2 )
			||(oldtrainstatus == 0 && message->trainstatus == 2))
	{
		ILOG("train arrival event");
		eventType = ARRIVAL;
	}else if((oldtrainstatus == 2 && message->trainstatus == 3 )
		||(oldtrainstatus == 0 && message->trainstatus == 3))
	{
		ILOG("train leave event");
		eventType = DEPARTURE;
	}else
	{
		ILOG("train normal working");
		eventType = 0;
	}
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "update PTITable set PTIState = '%d' where PTIindex = '%d'",message->trainstatus,message->PTIindex);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TrainEventPTIMsgSetDB update PTItable", errMsg);
		return -1;
	}

//	DLOG("create time : %s", timestr);
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7, Field8, Stream1) values('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
			434,eventType,message->PTIindex,ptid,trainID,carID,crewID,trainIndex,timestr);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TrainEventPTIMsgSetDB", errMsg);
		return -1;
	}

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7) values('%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			434,eventType,message->PTIindex,ptid,trainID,carID,crewID);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TrainEventPTIMsgSetDB", errMsg);
		return -1;
	}

	return 0;

}
/*******************************************************************
 Function name			ats_co_ALLMsgUpdateSetDB
 description			update all msg to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_ALLMsgUpdate *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int
ats_co_ALLMsgUpdateSetDB(sqlite3 *db, ats_co_ALLMsgUpdate *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6) values('%d', '%d', '%d', '%d', '%d', '%d')",
			message->msgType,message->hmiNumber,message->faultMsg,message->trainMsg,message->statusMsg,message->diagMsg);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_ALLMsgUpdateSetDB", errMsg);
		return -1;
	}
	ILOG("add allmasgupdate sucess: msgtype=%d faultMsg=%d",message->msgType,message->faultMsg);
	return 0;
}

int
ats_co_ALLMsgUpdateGetDB(sqlite3 *db, ats_co_ALLMsgUpdate *message)
{

  return 0;
}

/*******************************************************************
 Function name			ats_co_TrainIDModifySetDB
 description			update train id to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_TrainIDModify *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int
ats_co_TrainIDModifySetDB(sqlite3 *db, ats_co_TrainIDModify *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(
			sqlstr,
			"insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7, Field8, Field9, Field10, Field11, Field12, Field13, Field14,Field15,Field16) values('%d', '%d', '%d', '%d', '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			message->msgType,message->modifyType,message->destination_ID,message->service_ID,message->serial,
			message->car_one,message->car_two,message->logicalSection,message->oldlogicalSection,message->olddestination_ID,
			message->oldservice_ID,message->oldserial,message->trainIDFault,message->trainIndex,message->arsStatus,message->machineNo);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TrainIDModifySetDB", errMsg);
		return -1;
	}
	ILOG("add trainidmodify sucess: msgtype=%d modifytype=%d machineNo=%d",message->msgType,message->modifyType,message->machineNo);
	return 0;
}

int
ats_co_TrainIDModifyGetDB(sqlite3 *db, ats_co_TrainIDModify *message)
{

  return 0;
}

// task(old): HMI-->ATR TrainRegulate....................................................
int
ats_co_TrainEventMsgSetDB(sqlite3 *db, ats_co_TrainEventMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char timestr[TIME_SIZE];
//	struct tm tm_temp;
	char sqlstr[MAX_SQL_STR_LEN];
	struct tm tm_temp;

	tm_temp.tm_year = (message->eventTime.year) - 1900;
	tm_temp.tm_mon = message->eventTime.month - 1;
	tm_temp.tm_mday = message->eventTime.day;
	tm_temp.tm_hour = message->eventTime.hour;
	tm_temp.tm_min = message->eventTime.minute;
	tm_temp.tm_sec = message->eventTime.second;

//	struct tm *tm_loc;
//	time_t t;
//	t = time(NULL);
//	tm_loc = localtime(&t);

	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S",&tm_temp );

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field8, Field4, Stream1) values('%d', '%d', '%d', '%d', '%s')",
			message->msgType,message->eventType,message->trainIndex,message->PTID,timestr);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TrainEventMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add traineventmsg sucess: msgtype=%d trainindex=%d,eventtype = %d",message->msgType,message->trainIndex,message->eventType);
	return 0;
}

int
ats_co_TrainEventMsgGetDB(sqlite3 *db, ats_co_TrainEventMsg *message)
{

  return 0;
}




// task(new): HMI-->ATR TrainRegulate....................................................
int
ats_co_TrainEventTWCMsgSetDB(sqlite3 *db, ats_co_TrainEventTWCMsg *message)
{
	ILOG("Process twc message");
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	sqlite3_stmt *stmt;
	char timestr[TIME_SIZE];
	struct tm tm_temp;
	char sqlstr[MAX_SQL_STR_LEN];
    int twcState = 0;
    int ptid = 0;
    int changRowCount = 0;

//	tm_temp.tm_year = (message->eventTime.year+2000) - 1900;
//	tm_temp.tm_mon = message->eventTime.month - 1;
//	tm_temp.tm_mday = message->eventTime.day;
//	tm_temp.tm_hour = message->eventTime.hour;
//	tm_temp.tm_min = message->eventTime.minute;
//	tm_temp.tm_sec = 0;

    struct tm *tm_loc;
	time_t t;
	t = time(NULL);
	tm_loc = localtime(&t);
	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S", tm_loc);
	DLOG("create time : %s", timestr);

    //when TS send twc msg, we need update/insert carID in TrainStatus Table
    memset(sqlstr, 0, MAX_SQL_STR_LEN);
    sprintf(sqlstr, "select CarID from TrainStatus where CarID = '%d'", message->carID);
    sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
    rc = sqlite3_step(stmt);
    if (SQLITE_ROW == rc)
    {
    	memset(sqlstr, 0, MAX_SQL_STR_LEN);
    	sprintf(sqlstr, "update TrainStatus set TrainIndex = '%d' where CarID = '%d'", message->carID, message->carID);
    }
    else
    {
    	if(message->carID !=0)/*if carid invalid ,mpu will not create car 2013 .3.16 wh*/
    	{
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "insert into TrainStatus(TrainIndex, CarID) values('%d', '%d')",message->carID,message->carID);
    	}
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
    DLOG("Set CarNumber in TrainStatus sucess! TrainIndex = '%d' CarNumber = '%d' updated",message->carID,message->carID);

	//here: msg 400(208) need to convert to 434
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
    ILOG("TWC info: TWCIndex=%d ATPbreakOn=%d",message->TWCIndex,message->ATPbreakOn);
	sprintf(sqlstr,"select twcState, associatePtid from TwcTable where twcNumber = '%d'", message->TWCIndex);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		ELOG("SQLerror: %s while running ats_co_TrainEventTWCMsgSetDB", errMsg);
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW == rc)
	{
		twcState = sqlite3_column_int(stmt, 0);
		ptid = sqlite3_column_int(stmt, 1);
		ILOG("TwcTable : ptid=%d twcState=%d",ptid,twcState);
		sqlite3_finalize(stmt);

		if(twcState != message->ATPbreakOn)
		{
			message->eventType = 0;
			if((twcState == 0) && (message->ATPbreakOn == 1))
				message->eventType = 1; //arrive evevt
			if((twcState == 1) && (message->ATPbreakOn == 0))
				message->eventType = 2; //leave event

			if(twcState != -1)/*-1 means the loading data,if it is loading data ,do not trigger train event*/
			{

				memset(sqlstr, 0, MAX_SQL_STR_LEN);
				DLOG("create time : %s", timestr);
				sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7, Field8, Stream1) values('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
						434,message->eventType,message->TWCIndex,ptid,message->trainID,message->carID,message->crewID,message->trainIndex,timestr);
				rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
				if (SQLITE_OK != rc)
				{
					ELOG("SQLerror: %s while running ats_co_TrainEventTWCMsgSetDB", errMsg);
					return -1;
				}

				memset(sqlstr, 0, MAX_SQL_STR_LEN);
				sprintf(sqlstr, "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7) values('%d', '%d', '%d', '%d', '%d', '%d', '%d')",
						434,message->eventType,message->TWCIndex,ptid,message->trainID,message->carID,message->crewID);
				rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
				if (SQLITE_OK != rc)
				{
					ELOG("SQLerror: %s while running ats_co_TrainEventTWCMsgSetDB", errMsg);
					return -1;
				}

				ILOG("add traineventTWCmsg sucess: msgtype=%d TWCIndex=%d carID=%d crewID=%d",434,message->TWCIndex,message->carID,message->crewID);

			}

			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "update TwcTable set twcState = '%d' where twcNumber = '%d'", message->ATPbreakOn, message->TWCIndex);
			rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
			if (SQLITE_OK != rc)
			{
				return -1;
			}
		}

	}
	else
	{
		ELOG("TWCIndex is not found in TwcTable");
		sqlite3_finalize(stmt);
		return -1;
	}

	return 0;
}

int
ats_co_TrainEventTWCMsgGetDB(sqlite3 *db, ats_co_TrainEventTWCMsg *message)
{

  return 0;
}


// loadrequest from net..................................................
int
ats_co_FepStatusSetDB(sqlite3 *db, ats_co_FepStatusMsg *message)
{
	int rc;
    char *errMsg = NULL;
    sqlite3_stmt *stmt;
    char sqlstr[MAX_SQL_STR_LEN];
//    char sqlstr1[MAX_SQL_STR_LEN];
    CasSwitch2HMIFEPMsg Switch2HMIFEPMSG;
    int s201 = -1;
    int s202 = -1;
    int s203 = -1;
    int s204 = -1;
    char modleColum[4];
    char modleColum1[4];

    if (db == NULL)
	{
    	FLOG("fepstatus fatal error1");
    	return -1;
	}
    ILOG("message->fepID=%d",message->fepID);
    if(message->fepID >= 70 && (message->fepID) <= 73||(message->fepID == 4))
	{
		memset(sqlstr, 0, MAX_SQL_STR_LEN);
		sprintf(sqlstr, "select S201, S202, S203, S204 from FepToNetStatus where ConsoleID = '%d'", message->fepID);
		rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
		rc = sqlite3_step(stmt);
		if (SQLITE_ROW == rc)
		{
			s201 = sqlite3_column_int(stmt, 0);
			s202 = sqlite3_column_int(stmt, 1);
			s203 = sqlite3_column_int(stmt, 2);
			s204 = sqlite3_column_int(stmt, 3);
		}
		else
		{
			FLOG("fepstatus fatal error2, message->fepID: %d", message->fepID);
			sqlite3_finalize(stmt);
			return -1;
		}
		sqlite3_finalize(stmt);
		ILOG(":s201=%d,s202=%d,s203=%d,s204=%d",
				s201, s202, s203, s204);
		memset(sqlstr, 0, MAX_SQL_STR_LEN);
		memset(modleColum, 0, 4);
		sprintf(modleColum,"S%d",message->fepModule);
		//ILOG("modleColum:%s",modleColum);
		sprintf(sqlstr, "update FepToNetStatus set '%s' = '%d' where ConsoleID = '%d'",
				modleColum, message->fepStatus, message->fepID);
		sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
		ILOG("FepStatusSetDB sucess::FepID=%d,FepModel=%d,FepStatus=%d",
					message->fepID, message->fepModule, message->fepStatus);

		if(message->fepModule == 201
				&&((s201 == 1 && message->fepStatus == 0)
						&&s202 == 0 && s203 == 0 && s204 == 0)
		  )
		{
			//fep net port 1 off
			Switch2HMIFEPMSG.switch_flag = 1;
			Switch2HMIFEPMSG.fepid = message->fepID;
			DLOG("fep %d net is off",message->fepID);
		}
		else if(message->fepModule == 202
				&&((s202 == 1 && message->fepStatus == 0)
						&&s201 == 0 && s203 == 0 && s204 == 0)
		  )
		{
			//fep net port 2 off
			Switch2HMIFEPMSG.switch_flag = 1;
			Switch2HMIFEPMSG.fepid = message->fepID;
			DLOG("fep %d net is off",message->fepID);
		}
		else if(message->fepModule == 203
				&&((s203 == 1 && message->fepStatus == 0)
						&&s201 == 0 && s202 == 0 && s204 == 0)
		  )
		{
			//fep net port 3 off
			Switch2HMIFEPMSG.switch_flag = 1;
			Switch2HMIFEPMSG.fepid = message->fepID;
			DLOG("fep %d net is off",message->fepID);
		}
		else if(message->fepModule == 204
				&&((s204 == 1 && message->fepStatus == 0)
						&&s201 == 0 && s202 == 0 && s203 == 0)
		  )
		{
			//fep net port 4 off
			Switch2HMIFEPMSG.switch_flag = 1;
			Switch2HMIFEPMSG.fepid = message->fepID;
			DLOG("fep %d net is off",message->fepID);
		}
		if(Switch2HMIFEPMSG.switch_flag == 1)
		{
			unsigned char szbuf[NET_PACKET_LEN] = { 0 };
			NetHead stNetHead;
			memset(&stNetHead, 0, sizeof(NetHead));
			//the broadcast 213 send the switch msg to fep and hmi
			adapter_net_head_package(&stNetHead, 213, INSIDE,
					sizeof(CasSwitch2HMIFEPMsg)+sizeof(AppHeadIn), PROTOCOL_MULTICAST);
			memset(&Switch2HMIFEPMSG, 0, sizeof(CasSwitch2HMIFEPMsg));

			AppHeadIn appHeadIn;
			memset(&appHeadIn, 0, sizeof(AppHeadIn));
			adapter_app_head_package(&appHeadIn, 656, sizeof(CasSwitch2HMIFEPMsg)); //61

			memcpy(szbuf, &stNetHead, sizeof(NetHead));
			memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

			memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
					&Switch2HMIFEPMSG, sizeof(CasSwitch2HMIFEPMsg) );
			adapter_writen(app2netfd, szbuf,
					sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(CasSwitch2HMIFEPMsg));
			DLOG("send switch msg to HMI and fep");
		}
//		if(s201 == 0 && s202 == 0 && s203 == 0 && s204 == 0)
//		{
//
//			if(message->fepStatus == 1)/*1:connection ;0:disconnection*/
//			{
//	//    		adapter_co_send_loadRequest(message->fepID,message->fepModule,1,1,1,1);
//	//    		ILOG("send loadrequest to fep:FepID=%d,FepModel=%d",
//	//    				message->fepID, message->fepModule);
//	//			time_t t;                                  // time_t type
//	//			struct timeval tv;     // tm struct
//	//			gettimeofday(&tv, NULL);                    // get the second
//	//			t = tv.tv_sec;
//	//		    memset(sqlstr, 0, MAX_SQL_STR_LEN);
//	//		    sprintf(sqlstr, "update Loadrqsstatus set Loadtime = '%d' where loadid = 1",t);
//	//		    sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
//
//			}
//		}

    }

    ats_co_CASNetStatusMsg casnetstatus_message;
    casnetstatus_message.fepID = message->fepID;
    casnetstatus_message.fepModule = message->fepModule;
    casnetstatus_message.fepStatus = message->fepStatus;
    ats_co_CASNetStatusMsgSetDB(db, &casnetstatus_message);
	return 0;
}

//route status
int
ats_co_RouteStatusMsgSetDB(sqlite3 *db, ats_co_RouteStatusMsg *message)
{

  if ((db == NULL) || message == NULL)
  {
      return -1;
  }

  int rc;
  char *errMsg = NULL;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];
  int signalStatus = -1;
  int startSignal = -1;
  int isVirtualSignal = -1;
  int axleCounterID[10];
  int direction[10];
  int temp = 0, i = 0;

  ILOG("routeStatus=%d",message->routeStatus);

  if(message->routeStatus == 1)
  {
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "update RouteToTrack set Status = '%d' where RouteID = '%d'", message->routeStatus, message->routeIndex);
  sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  ILOG("update RouteToTrack sucess: msgtype=%d routeIndex=%d routeStatus=%d",
  			  message->msgType,
  			  message->routeIndex,
  			  message->routeStatus);

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr,"select AxleCounterID, Direction from RouteToTrack where RouteID = '%d' and Status = '%d'",
		  message->routeIndex, message->routeStatus);
  sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);
  while (SQLITE_ROW == rc)
  {
	  axleCounterID[temp] = sqlite3_column_int(stmt, 0);
	  direction[temp] = sqlite3_column_int(stmt, 1);
	  temp++;
	  rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);

  while(i < temp)
  {
	  memset(sqlstr, 0, MAX_SQL_STR_LEN);
//	  if(message->routeStatus == 0)
//		  sprintf(sqlstr, "update LogicalTrack set Direction = '%d' where TrackID = '%d'",
//				  -1, axleCounterID[i]);  //changed by wangheng 2012.7.16
	  if(message->routeStatus == 1)
		  sprintf(sqlstr, "update LogicalTrack set Direction = '%d' where TrackID = '%d'",
				  direction[i], axleCounterID[i]);
	  sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

//	  if(message->routeStatus == 0)
//		  ILOG("update LogicalTrack sucess: msgtype=%d TrackID=%d Direction=%d",
//				  message->msgType,axleCounterID[i],-1);

	  if(message->routeStatus == 1)
		  ILOG("update LogicalTrack sucess: msgtype=%d TrackID=%d Direction=%d",
				  message->msgType,axleCounterID[i],direction[i]);
	  i++;
  }
  }
  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr,"select startSignalID, IsVirtualSignal from RouteAndSignal where RouteIDOfSICAS = '%d'", message->routeIndex);
  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  if (rc != SQLITE_OK)
  {
	  sqlite3_finalize(stmt);
  		return -1;
  }
  rc = sqlite3_step(stmt);
  if (SQLITE_ROW == rc)
  {
	  startSignal = sqlite3_column_int(stmt, 0);
	  isVirtualSignal = sqlite3_column_int(stmt, 1);

	  sqlite3_finalize(stmt);

	  if((startSignal != -1) && (isVirtualSignal == 1))
	  {
		  if(message->routeStatus == 0)
			  signalStatus = 0;
		  if(message->routeStatus == 1)
			  signalStatus = 3;

		  memset(sqlstr, 0, MAX_SQL_STR_LEN);
		  sprintf(sqlstr, "update Signal set SignalStatus = '%d' where SignalID = '%d'", signalStatus, startSignal);
		  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
		  if (SQLITE_OK != rc)
		  {
			  return -1;
		  }

	  }

	  ILOG("update Signal by routestatusmsg sucess: msgtype=%d routeIndex=%d routeStatus=%d signalID=%d signalStatus=%d",
			  message->msgType,
			  message->routeIndex,
			  message->routeStatus,
			  startSignal,
			  signalStatus);
  }
  else
  {
	  sqlite3_finalize(stmt);
	  return -1;
  }

  return 0;
}

int
ats_co_RouteStatusMsgGetDB(sqlite3 *db, ats_co_RouteStatusMsg *message)
{

  return 0;
}





// task: ATR(256)....................................................
int
ats_co_RequireRunningMsgSetDB(sqlite3 *db, ats_co_RequireRunningMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1) values('%d')",message->msgType);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_RequireRunningMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add requireRunningMsgSetDB sucess: msgtype=%d",message->msgType);
	return 0;
}

// task: ATR(256)....................................................
int
ats_co_RequireModifyATRModeMsgSetDB(sqlite3 *db, ats_co_RequireModifyATRModeMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field3, Field4, Field5, Field6, Field7, Field8, Field9, Field10, Field11, Field12, Field13) values('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d','%d', '%d', '%d', '%d', '%d')",
				message->msgType,message->machineNO,message->requireModifyType,message->lineNo,message->trainNO,
				message->mode,message->para1,message->para2,message->destination,message->serviceID,
				message->serial,message->car1,message->car2);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_RequireModifyATRModeMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add requireModifyATRModeMsg sucess: msgtype=%d",message->msgType);
	ILOG("message->machineNO=%d",message->machineNO);
	return 0;
}


// task: ATR(253)....................................................
int
ats_co_LoadTimeTableMsgSetDB(sqlite3 *db, ats_co_LoadTimeTableMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2) values('%d', '%d')",
				message->msgType,message->timerableInnerID);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_LoadTimeTableMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add ats_co_LoadTimeTableMsg sucess: msgtype=%d",message->msgType);
	ILOG("message->timerableInnerID=%d",message->timerableInnerID);
	return 0;
}

// task: ATR(351)....................................................
int
ats_co_TTPLoadTimeTableMsgSetDB(sqlite3 *db, ats_co_InterfaceTTPMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2, Field3) values('%d', '%d','%d')",
				message->msgType,message->innerid,message->versionno);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_TTPLoadTimeTableMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add ats_co_TTPLoadTimeTableMsgSetDB sucess: msgtype=%d",message->msgType);
//	ILOG("message->timerableInnerID=%d",message->timerableInnerID);
	return 0;
}

// task: ATR(205)....................................................
int
ats_co_ControlRightAutoMsgSetDB(sqlite3 *db, ats_co_TsControlRightAutoMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "update controlrightauto set interlockstatus = %d where interlockid=%d",
		message->controlrightauto,message->controlindex);

	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_FepMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add ats_co_FepMsgSetDB sucess: msgtype=%d,fepstatus = %d,fepid=%d",message->msgType,message->controlrightauto,message->controlindex);
//	ILOG("message->timerableInnerID=%d",message->timerableInnerID);
	return 0;
}
/*******************************************************************
 Function name			ats_co_CASBeatMsgSetDB
 description			add set cas heart beat status to db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_CASBeatMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int
ats_co_CASBeatMsgSetDB(sqlite3 *db, ats_co_CASBeatMsg *message)
{
	return 0;
}
/*******************************************************************
 Function name			ats_co_LoadOKMsgSetDB
 description			set load msg status to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_LoadOKmsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int ats_co_LoadOKMsgSetDB(sqlite3 *db, ats_co_LoadOKmsg *message)
{
	ILOG("START ats_co_LoadOKMsgSetDB");
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
//		extern int TRCLoadOKflag;
//		TRCLoadOKflag = 1;
		MpuReceiveLoadNum_i++;
		ILOG("MpuReceiveLoadNum_i = %d",MpuReceiveLoadNum_i);

		/*if 1:TRC restart; 2: TRC finished loading data; 3:CAS is working*/
		if(MpuReceiveLoadNum_i == MPU_FINISH_LOAD_NUM_TRC && MpuStartFlag_i == 0 )
		{
		    DLOG("add ats_co_LoadOKMsgSetDB sucess:%d ");

			MpuStartFlag_i = 1;/*if receive 4 load data ,MPU finished start*/
			DLOG("trc finished start");

		    int rc=0;
		    int heartbeatstatus=0;
		    sqlite3_stmt *stmt;
		    char sqlstr[MAX_SQL_STR_LEN];
		    memset(sqlstr, 0, MAX_SQL_STR_LEN);
		    sprintf(sqlstr, "select heartbeatstatus from CONTROLRIGHTAUTO where FEPid = 70");
		    rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
		    rc = sqlite3_step(stmt);

		    if (SQLITE_ROW == rc)
		    {
		    	heartbeatstatus = sqlite3_column_int(stmt, 0);
		    }
		    else
		    {
		    	FLOG("CONTROLRIGHTAUTO fatal error");
		    	sqlite3_finalize(stmt);
		    	return -1;
		    }
		    sqlite3_finalize(stmt);


		    if(heartbeatstatus == 4 || heartbeatstatus== 2)/*CAS is running*/
		    {
		    	adapter_mpu_send_TrainIDSYCMSG();
		    	ILOG("send 650 msg to CAS");

		    }

		}

	}
	else if(LOCAL_OR_CENTER == CENTER_CAS)
	{
		ILOG("CAS:add ats_co_LoadOKMsgSetDB sucess ");
		MpuReceiveLoadNum_i++;

		/*if 1:TRC restart; 2: TRC finished loading data; 3:CAS is working*/
		if(MpuReceiveLoadNum_i == MPU_FINISH_LOAD_NUM && MpuStartFlag_i == 0 )
		{
			DLOG("add ats_co_LoadOKMsgSetDB sucess:%d ");

			MpuStartFlag_i = 1;/*if receive 4 load data ,MPU finished start*/
			DLOG("trc finished start");

			int rc=0;
			int heartbeatstatus=0;
			sqlite3_stmt *stmt;
			char sqlstr[MAX_SQL_STR_LEN];
			memset(sqlstr, 0, MAX_SQL_STR_LEN);
			sprintf(sqlstr, "select heartbeatstatus from CONTROLRIGHTAUTO where FEPid = 70");
			rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
			rc = sqlite3_step(stmt);

			if (SQLITE_ROW == rc)
			{
				heartbeatstatus = sqlite3_column_int(stmt, 0);
			}
			else
			{
				FLOG("CONTROLRIGHTAUTO fatal error");
				sqlite3_finalize(stmt);
				return -1;
			}
			sqlite3_finalize(stmt);


			if(heartbeatstatus == 4 || heartbeatstatus== 2)/*CAS is running*/
			{
				adapter_mpu_send_TrainIDSYCMSG();
				ILOG("send 650 msg to TRC");

			}

		}

	}
	return 0;
}
/*******************************************************************
 Function name			ats_co_RecieveTrainidSynRequireSetDB
 description			set  receive train id sync to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_RecieveLoadOKMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int ats_co_RecieveTrainidSynRequireSetDB(sqlite3 *db, ats_co_RecieveLoadOKMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into TMTInputTask(Field1) values('%d')",message->msgType);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_RecieveLoadOKMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add ats_co_RecieveLoadOKMsgSetDB sucess: msgtype=%d",message->msgType);
}
/*******************************************************************
 Function name			ats_co_CFepRequireCASDataMsgSetDB
 description			set task: ATR/TMT(608) to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_CFepRequireCASDataMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
// task: ATR/TMT(608)....................................................
int
ats_co_CFepRequireCASDataMsgSetDB(sqlite3 *db, ats_co_CFepRequireCASDataMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into ATRInputTask(Field1, Field2) values('%d', '%d')",
				message->msgType,message->machineNO);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into TMTInputTask(Field1, Field2) values('%d', '%d')",
				message->msgType,message->machineNO);
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_CFepRequireCASDataMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add ats_co_CFepRequireCASDataMsg sucess: msgtype=%d",message->msgType);
	ILOG("message->machineNO=%d",message->machineNO);
	return 0;
}



int
ats_co_AlarmMsgSetDB(sqlite3 *db, ats_co_AlarmMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];
	char timestr[MAX_SQL_STR_LEN];
	char remark[MAX_SQL_STR_LEN];

	struct tm tm_temp;
	tm_temp.tm_year = message->year;
	tm_temp.tm_mon = message->month;
	tm_temp.tm_mday = message->day;
	tm_temp.tm_hour = message->hour;
	tm_temp.tm_min = message->minute;
	tm_temp.tm_sec = 0;
	tm_temp.tm_year -= 1900;
	tm_temp.tm_mon -= 1;
	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S", &tm_temp);

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	if(message->remark == 0)
	{
		sprintf(sqlstr, "insert into AlarmList(ElementType, ElementID, AlarmTime, AlarmID, AlarmClose) values('%d', '%d', '%s', '%d', '%d')",
					message->equipment_Type,message->equipment_ID,timestr,message->alarm_ID,message->isRecover);
	}
	else
	{
		memset(remark, 0, MAX_SQL_STR_LEN);
		sprintf(remark,"%d",message->remark);
		sprintf(sqlstr, "insert into AlarmList(ElementType, ElementID, AlarmTime, AlarmID, AlarmClose, Remark) values('%d', '%d', '%s', '%d', '%d', '%s')",
						message->equipment_Type,message->equipment_ID,timestr,message->alarm_ID,message->isRecover,remark);
	}
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_AlarmMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add alarmMsg sucess: equipment_Type=%d equipment_ID=%d",message->equipment_Type,message->equipment_ID);
	return 0;
}



int
ats_co_AlarmMsgOfOutSetDB(sqlite3 *db, ats_co_ErrIndicationMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];
	char timestr[MAX_SQL_STR_LEN];
	char remark[MAX_SQL_STR_LEN];

	ILOG("year=%d,month=%d,day=%d,hour=%d,min=%d",message->year,message->month,message->day,message->hour,message->minute);


	struct tm tm_temp;
	tm_temp.tm_year = message->year;
	tm_temp.tm_mon = message->month;
	tm_temp.tm_mday = message->day;
	tm_temp.tm_hour = message->hour;
	tm_temp.tm_min = message->minute;
	tm_temp.tm_sec = 0;
	tm_temp.tm_year -= 1900;
	tm_temp.tm_mon -= 1;
	memset(timestr, 0, TIME_SIZE);
	strftime(timestr, TIME_SIZE, "%Y-%m-%d_%H:%M:%S", &tm_temp);

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	if(message->remark == 0)
	{
		sprintf(sqlstr, "insert into AlarmList(ElementType, ElementID, AlarmTime, AlarmID, AlarmClose) values('%d', '%d', '%s', '%d', '%d')",
					message->elementType,message->index,timestr,message->errCode,message->isRecover);
	}
	else
	{
		memset(remark, 0, MAX_SQL_STR_LEN);
		sprintf(remark,"%d",message->remark);
		sprintf(sqlstr, "insert into AlarmList(ElementType, ElementID, AlarmTime, AlarmID, AlarmClose, Remark) values('%d', '%d', '%s', '%d', '%d', '%s')",
						message->elementType,message->index,timestr,message->errCode,message->isRecover,remark);
	}
	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: %s while running ats_co_AlarmMsgSetDB", errMsg);
		return -1;
	}
	ILOG("add alarmMsg sucess: equipment_Type=%d equipment_ID=%d",message->elementType,message->index);
	return 0;
}


/*******************************************************************
 Function name			ats_co_LCfepNetStatusMsgSetDB
 description			set lcfep net status msg to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_LCfepNetStatusMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/

int
ats_co_LCfepNetStatusMsgSetDB(sqlite3 *db, ats_co_LCfepNetStatusMsg *message)
{
	if ((NULL == db)||(NULL == message))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
//	sprintf(
//			sqlstr,
//			"insert into LCfepNetStatus(ElementType, StatusInfo) values('%d', ?)",
//			message->equipment_Type);
	sprintf(sqlstr, "update LCfepNetStatus set StatusInfo = ? where ElementType = '%d'",
			message->equipment_Type);
	sqlite3_prepare(db, sqlstr, -1, &stmt, NULL);
	rc=sqlite3_bind_blob(stmt,1,message->LCfepMsgData, message->length-2,NULL);
	if (rc!=SQLITE_OK)
	{ILOG("bind text error!");}
	rc=sqlite3_step(stmt);
	if (rc!=SQLITE_DONE)
		{ILOG("exec  error!");}
	sqlite3_finalize(stmt);

//	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
//	if (SQLITE_OK != rc)
//	{
//		ELOG("SQLerror: %s while running ats_co_LoadTimeTableMsgSetDB", errMsg);
//		return -1;
//	}
	return 0;
}



int
ats_co_CASNetStatusMsgSetDB(sqlite3 *db, ats_co_CASNetStatusMsg *message)
{
	DLOG("hmi11111");
	int rc;
    char *errMsg = NULL;
    sqlite3_stmt *stmt;
    char sqlstr[MAX_SQL_STR_LEN];

    int connectionid = 0;
    if (db == NULL)
	{
    	return -1;
	}

    memset(sqlstr, 0, MAX_SQL_STR_LEN);
    sprintf(sqlstr, "select ConnectionID from CASNetConfig where DestinationIPNO = '%d' and DestinationComponentNO = '%d'",
    		message->fepID,message->fepModule);
    rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
    rc = sqlite3_step(stmt);

    if (SQLITE_ROW == rc)
    {
    	connectionid = sqlite3_column_int(stmt, 0);
    }
    else
    {
    	FLOG("casnetstatus fatal error2, message->fepID: %d", message->fepID);
    	sqlite3_finalize(stmt);
    	return -1;
    }
    sqlite3_finalize(stmt);

    memset(sqlstr, 0, MAX_SQL_STR_LEN);
    sprintf(sqlstr, "update CASNetStatus set NetStatus = '%d' where ConnectionID = '%d'",
    		message->fepStatus, connectionid);
    sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
    ILOG("casnetstatus sucess::FepID=%d,FepModel=%d,FepStatus=%d",
        		message->fepID, message->fepModule, message->fepStatus);

	return 0;
}



int
ats_co_InterfaceTrackStatusMsgSetDB(sqlite3 *db, ats_co_InterfaceTrackStatusMsg *message)
{
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  char *errMsg = NULL;
  char sqlstr[MAX_SQL_STR_LEN];

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "insert into TMTInputTask(Field1, Field2, Field3, Field4, Field5) values('%d', '%d', '%d', '%d', '%d')",
		  message->msgType,message->trackid,message->freestatus,message->car1,message->car2);
  rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

  return 0;
}



int
ats_co_HmiToATRManualRegulationValueSetDB(sqlite3 *db, ats_co_HmiToATRManualRegulationValue *message)
{
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  char sqlstr[MAX_SQL_STR_LEN];
  sqlite3_stmt *stmt;
  char *tmpid = NULL;
  int len;

  //sqlite3_exec(db , "begin transaction" , 0 , 0 ,NULL);
 tmpid = message->AllStationManualReguData;
 len = 14*message->StationNum;

  memset(sqlstr, 0, MAX_SQL_STR_LEN);
  sprintf(sqlstr, "insert into ATRINPUTTASK(Field1, Field2, Field3, Field4, Field5,Field6, Field7, Field8, Field9, Field10,STREAM1) values('%d', '%d', '%d', '%d', '%d','%d', '%d', '%d', '%d', '%d',?)",
		  message->msgType,message->Type,message->LineNO,message->StationNum,message->TrainIndex,message->Destination,message->ServiceID,message->TripID,message->car1,message->car2);
  sqlite3_prepare(db, sqlstr, -1, &stmt, NULL);
  rc=sqlite3_bind_blob(stmt,1,tmpid, len,NULL);
  if (rc!=SQLITE_OK)
  	{ILOG("bind text error!");}
  rc=sqlite3_step(stmt);
  if (rc!=SQLITE_DONE)
  	{ILOG("exec  error!");}
  sqlite3_finalize(stmt);

  ILOG("add HmiToATRManualRegulationValue sucess: equipment_Type=%d",message->msgType);

  return 0;
}

/*******************************************************************
 Function name			ats_co_RecieveTrainidSynSetDB
 description			set train id sync to the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ats_co_RecieveTrainidSynMsg *         IN/OUT          message
 Return value
 true or false

 *******************************************************************/
int ats_co_RecieveTrainidSynSetDB(sqlite3 *db, ats_co_RecieveTrainidSynMsg *message)
{
	DLOG("ats_co_RecieveTrainidSynSetDB =%d",message->msgType);
	int rc;
	char sqlstr[MAX_SQL_STR_LEN];
	sqlite3_stmt *stmt;
	char *tmpid;
	tmpid = message->AllTrainData;
	sprintf(
				sqlstr,
				"insert into TMTInputTask(Field1,Field2, Stream1) values('%d', '%d',?)",
				message->msgType,message->trainNum);
	sqlite3_prepare(db, sqlstr, -1, &stmt, NULL);
	rc=sqlite3_bind_blob(stmt,1,tmpid, ADB_TRAININFO_UNIT_LEN*(message->trainNum)+1,NULL);
	if (rc!=SQLITE_OK)
	{ILOG("bind text error!");}

	rc=sqlite3_step(stmt);
	if (rc!=SQLITE_DONE)
		{ILOG("exec  error!");}
	sqlite3_finalize(stmt);
	 return 0;
}



//for repaydb-interlock
int
ats_co_Update_Interlock_for_repalydb(sqlite3 *db, ATS_CO_MSG *handle)
{
	if ((NULL == db)||(NULL == handle))
	{
		return -1;
	}
	int rc;
	char *errMsg = NULL;
	char sqlstr[MAX_SQL_STR_LEN];
	sqlite3_stmt *stmt;

//	ILOG("format length=%d",handle->fixed_len);
//	ILOG("elementIndex=%d",handle->format[7]);
//	ILOG("elementIndex=%d",handle->format[8]);
	int elementIndex=bytes2int(&(handle->format[7]),SHT2BYTE);

//	ILOG("start to update interlockdata for replaydb : msgtype=%d elementtype=%d elementindex=%d",handle->config.message_type,handle->config.element_type,elementIndex);

	memset(sqlstr, 0, MAX_SQL_STR_LEN);

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	unsigned long time =(tv.tv_sec+abs(tz.tz_minuteswest*60))*1000+tv.tv_usec/1000;
	sprintf(sqlstr, "insert into T_INTERLOCKDATA(DATATYPE, ELEMENTTYPE,ELEMENTINDEX, TIME,DATA) values('%d', '%d', %d,%lu,?)",
			handle->config.message_type,
			handle->config.element_type,
			elementIndex,time);


	sqlite3_prepare(db, sqlstr, -1, &stmt, NULL);
	//sqlite3_bind_text16(stmt,1,handle->format, handle->fixed_len,NULL);
//	ILOG("handle->fixed_len====%d",handle->fixed_len);
	sqlite3_bind_blob(stmt,1,handle->format, handle->fixed_len-1,NULL);


	rc=sqlite3_step(stmt);

	if (rc!=SQLITE_DONE)
	{
		ILOG("insert into ReplayInterlock exec  error!");
	}

	sqlite3_finalize(stmt);

	ILOG("add interlockdata for replaydb sucess: msgtype=%d elementtype=%d",handle->config.message_type,handle->config.element_type);
	return 0;
}
/*******************************************************************
 Function name			ats_co_log_GetFreeStatus
 description			get track free status from the sqlite3 db
 parameter
 sqlite3 *			IN/OUT 			db
 ATS_CO_MSG *         IN/OUT          message
 const int            IN              trackid
 Return value
 true or false

 *******************************************************************/
int ats_co_log_GetFreeStatus (int *freestatus, sqlite3 *db, const int trackid)
{
	if ((NULL == db)||(NULL == freestatus))
	{
		ELOG("get freestatus failed: value is null");
		return -1;
	}
	int rc;
	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN];
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "select FreeStatus from LogicalTrack where TrackID = '%d'", trackid);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (SQLITE_OK != rc)
	{
		sqlite3_finalize(stmt);
		FLOG("SQLerror: %s while running ats_co_log_GetFreeStatus", sqlite3_errmsg(db));
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW == rc)
	{
		*freestatus = sqlite3_column_int(stmt, 0);
		ILOG("get freestatus sucess: trackid=%d freestatus=%d",trackid,*freestatus);
	}
	else
	{
		sqlite3_finalize(stmt);
		*freestatus = -1;
		ELOG("get freestatus failed: trackid=%d is not found. freestatus=-1",trackid);
		return -1;
	}
	sqlite3_finalize(stmt);
	return 0;
}
