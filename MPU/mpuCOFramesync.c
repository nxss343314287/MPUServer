/*******************************************************************
filename      mpuCOFrameSync.c
author        root
created date  2012-6-20
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/
#include "include/MPUMessage.h"
#include "include/MPURMMessage.h"

int
ats_co_FramesyncMsgSetDB(sqlite3 *db, MPU_RM_FRAME_SYN_IND *message)
{
  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  int rc;
  int i = 0;
  char *errMsg = NULL;
  char sqlstr[MAX_SQL_STR_LEN];

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
  	sprintf(sqlstr, "delete from FrameSYNC");
  	rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
  	if (SQLITE_OK != rc)
  	{
  		ELOG("delete sql error: %s while running ats_co_FramesyncMsgSetDB", errMsg);
  		return -1;
  	}

	while(i < message->listnum)
	{
		//prepare for sql statement
		memset(sqlstr, 0, MAX_SQL_STR_LEN);
		sprintf(sqlstr, "insert into FrameSYNC(HostID, SeqNO, ProcessTime,MainOrBak) values('%d', '%d', '%lld' ,'%d')", message->iHostID[i],
			  message->seqNo[i], message->sMPUSyncTime,message->MainOrBak[i]);

		rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

		ILOG("FramesyncMsgSetDB sucess: HostID=%d SeqNO='%d' MainOrBak='%d'",message->iHostID[i],message->seqNo[i],message->MainOrBak[i]);
		i++;
		if (SQLITE_OK != rc)
		{
			ELOG("insert sql error: %s while running ats_co_FramesyncMsgSetDB\n", errMsg);
		}
	}
	return 0;
}

int
ats_co_FramesyncMsgGetDB(sqlite3 *db, MPU_RM_FRAME_SYN_IND *message)
{

  int rc;
  int listnum = 0;
  sqlite3_stmt *stmt;
  char sqlstr[MAX_SQL_STR_LEN];

  if ((db == NULL) || message == NULL)
    {
      return -1;
    }

  //prepare for sql statement
  memset(sqlstr, 0, MAX_SQL_STR_LEN);

	sprintf(sqlstr,"select HostID, SeqNO,MainOrBak, ProcessTime from FrameSYNC");

  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      ELOG("SQL prepare error: %s\n", sqlite3_errmsg(db));
    }

  rc = sqlite3_step(stmt);
  //int ncols = sqlite3_column_count(stmt);

  if (rc == SQLITE_NOTFOUND)
    {

      return SQLITE_NOTFOUND;
    }

	while (rc == SQLITE_ROW)
	{
		message->iHostID[listnum] = sqlite3_column_int(stmt, 0);
		message->seqNo[listnum] = sqlite3_column_int(stmt, 1);
		message->MainOrBak[listnum] = sqlite3_column_int(stmt, 2);
		message->sMPUSyncTime = sqlite3_column_int(stmt, 3);

		ILOG("get frame : iHostID[%d] = %d,seqNo[%d] = %d ,MainOrBak = %d",listnum,message->iHostID[listnum],listnum,message->seqNo[listnum],message->MainOrBak[listnum]);
		listnum++;
		rc = sqlite3_step(stmt);
	}

	message->listnum = listnum;
	sqlite3_finalize(stmt);

	if(listnum == 0)
	{
		return -1;
	}
	return 0;
}
