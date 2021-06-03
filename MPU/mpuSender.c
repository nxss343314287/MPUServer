/*
 * mpusender.c
 *
 *  Created on: 2012-6-22
 *      Author: root
 */
#include "include/MPUSendMessage.h"
#include "include/MPUSender.h"
#include "../log.h"
#include "include/MPU.h"


#define SQL_STR_LEN         1024
#define CAS_EQUIPMENTID     81
#define LFEP1_EQUIPMENTTYPE 61
#define LFEP2_EQUIPMENTTYPE 62
#define LFEP3_EQUIPMENTTYPE 63
#define LFEP4_EQUIPMENTTYPE 64
#define CFEP1_EQUIPMENTTYPE 71
#define CFEP2_EQUIPMENTTYPE 72
#define SEND_NETSTATUS_TOHMI_MACHINENO 205
#define CASConnectionNum 46

extern int fepStatusType;

int adapter_init(void)
{
//	log_init();
//	log_set_category("app");
	atexit(onclose);

//	unlink(IO2NET);
//	if (0 != mkfifo(IO2NET, 0660))
//	{
//		ELOG("Failed to mkfifo: %s", strerror(errno));
//	}

	unlink(APP2NET);
	if( (app2netfd = open(APP2NET, O_RDWR)) < 0 )
	{
		if ((app2netfd = mkfifo(APP2NET, 0666)) < 0)
		{
			ELOG("create APP2NET PIPE error,s=%d",app2netfd);
			return -1;
		}
		else
		{
			if ((app2netfd = open(APP2NET, O_RDWR)) < 0)
			{
				ELOG("Open APP2NET PIPE error![%s]", strerror(errno));
				return -1;
			}
		}
	}
	else
	{
		DLOG("open pipe ok, pipename=%s,fp=%d",APP2NET,app2netfd);
	}
	if (app2netfd == -1)
	{
		ELOG("Open APP2NET PIPE error![%s]", strerror(errno));
		return -1;
	}
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
		unlink(APP2TSNET);
		if( (app2TSnetfd = open(APP2TSNET, O_RDWR)) < 0 )
		{
			if ((app2TSnetfd = mkfifo(APP2TSNET, 0666)) < 0)
			{
				ELOG("create APP2TSNET PIPE error,s=%d",app2TSnetfd);
				return -1;
			}
			else
			{
				if ((app2TSnetfd = open(APP2TSNET, O_RDWR)) < 0)
				{
					ELOG("Open APP2TSNET PIPE error![%s]", strerror(errno));
					return -1;
				}
			}
		}
		else
			DLOG("open pipe ok, pipename=%s,fp=%d",APP2TSNET,app2TSnetfd);


		if (app2TSnetfd == -1)
		{
			ELOG("Open APP2TSNET PIPE error![%s]", strerror(errno));
			return -1;
		}
	}

	return 0;
}
void onclose(void)
{
	close(app2netfd);
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
		close(app2TSnetfd);
	}
}
/*******************************************************************
 Function name		mpuSender_SendTask_process
 description		send task status
 parameter
 sqlite *					IN/OUT				db
 Return value
 true or false
 *******************************************************************/
int mpuSender_SendTask_process (sqlite3 *db)
{
	if ((NULL == db))
	{
		return -1;
	}
	int rc;
	sqlite3_stmt *stmt;
	int msg_type = 0;
	int function_type = 0;
	char sqlstr[SQL_STR_LEN];

	char *trainDataTemp = NULL;
	int train_count = 0;
	char *stationDataTemp = NULL;
    int station_count = 0;

	memset(sqlstr, 0, SQL_STR_LEN);
	sprintf(sqlstr, "select * from SendTaskList");
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (SQLITE_OK != rc)
	{
		sqlite3_finalize(stmt);
		ELOG("SQLerror: %s while running mpuSender_SendTask_process", sqlite3_errmsg(db));
		return -1;
	}

	rc = sqlite3_step(stmt);
	while ((SQLITE_ROW == rc))
	{
		msg_type = sqlite3_column_int(stmt, 1);
		function_type = sqlite3_column_int(stmt, 7);

		ILOG("msg_type = %d",msg_type);

		switch (msg_type)
		{
		case 512:
		{
			DLOG("fepStatusType = %d",fepStatusType);
			if(fepStatusType == 0)
			{
			
				if(function_type == 4032) //ARS ---autoroutetask
				{
					ILOG("Get Sendtasklist->ARSAutoRouteTask sucess: msgtype=%d,machineNO=%d,"
							"componentNO=%d,consoleNo=%d,interlockZone=%d,disposalNo=%d,"
							"functionNo=%d,element_type=%d,element_index=%d",
							msg_type,
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9));

					if(LOCAL_OR_CENTER == LOCAL_TRC)/*1:center; 0:local*/
					{
						int interlock_number = sqlite3_column_int(stmt, 5);

						int send_judge =mpusender_trcsend_judge(db,interlock_number);

						if(send_judge == 0)
						{
							adapter_ars_auto_route_send(
									sqlite3_column_int(stmt, 2),
									sqlite3_column_int(stmt, 3),
									sqlite3_column_int(stmt, 4),
									sqlite3_column_int(stmt, 5),
									sqlite3_column_int(stmt, 6),
									function_type,
									sqlite3_column_int(stmt, 8),
									sqlite3_column_int(stmt, 9));
							ILOG(" TRC  send the message %d",function_type);
						}
						else
						{
							ILOG(" TRC do not send the message %d",function_type);
						}

					}

					else if(LOCAL_OR_CENTER == CENTER_CAS)
					{

						adapter_ars_auto_route_send(
								sqlite3_column_int(stmt, 2),
								sqlite3_column_int(stmt, 3),
								sqlite3_column_int(stmt, 4),
								sqlite3_column_int(stmt, 5),
								sqlite3_column_int(stmt, 6),
								function_type,
								sqlite3_column_int(stmt, 8),
								sqlite3_column_int(stmt, 9));
					}
				}
				if(function_type == 11587) //TMT ---sendtrainidtotraintask
				{
	//				ILOG("Get Sendtasklist->TMTSendTrainIdentifyNOTask sucess: msgtype=%d,machineNO=%d,"
	//						"componentNO=%d,consoleNo=%d,interlockZone=%d,disposalNo=%d,"
	//						"functionNo=%d,elementType=%d,TWCIndex=%d,destination=%d,service=%d,serial=%d",
	//						msg_type,
	//						sqlite3_column_int(stmt, 2),
	//						sqlite3_column_int(stmt, 3),
	//						sqlite3_column_int(stmt, 4),
	//						sqlite3_column_int(stmt, 5),
	//						sqlite3_column_int(stmt, 6),
	//						function_type,
	//						sqlite3_column_int(stmt, 8),
	//						sqlite3_column_int(stmt, 9),
	//						sqlite3_column_int(stmt, 10),
	//						sqlite3_column_int(stmt, 11),
	//						sqlite3_column_int(stmt, 12));

					if(LOCAL_OR_CENTER == LOCAL_TRC)/*1:center; 0:local*/
					{
						int interlock_number = sqlite3_column_int(stmt, 5);

						int send_judge = mpusender_trcsend_judge(db,interlock_number);

						if(send_judge == 0)
						{

							if(PROJECT_MODE == ITC_HEB1)
							{
								adapter_tmt_send_TrainIdentifyNO(
									sqlite3_column_int(stmt, 2),
									sqlite3_column_int(stmt, 3),
									sqlite3_column_int(stmt, 4),
									sqlite3_column_int(stmt, 5),
									sqlite3_column_int(stmt, 6),
									function_type,
									sqlite3_column_int(stmt, 8),
									sqlite3_column_int(stmt, 9),
									sqlite3_column_int(stmt, 10),
									sqlite3_column_int(stmt, 11),
									sqlite3_column_int(stmt, 12));
							}
							else if(PROJECT_MODE == CBTC)
							{
								adapter_tmt_send_TrainIdentifyNO_ForCBTC(
										sqlite3_column_int(stmt, 2),
										sqlite3_column_int(stmt, 3),
										sqlite3_column_int(stmt, 4),
										sqlite3_column_int(stmt, 5),
										sqlite3_column_int(stmt, 6),
										function_type,
										sqlite3_column_int(stmt, 8),
									//	sqlite3_column_int(stmt, 9),
										sqlite3_column_int(stmt, 10),
										sqlite3_column_int(stmt, 11),
										sqlite3_column_int(stmt, 12),
										sqlite3_column_int(stmt, 13),
										sqlite3_column_int(stmt, 14),
										sqlite3_column_int(stmt, 15),
										sqlite3_column_int(stmt, 16));
							}
							ILOG(" TRC  send the message %d",function_type);
						}
						else
						{
							ILOG(" TRC do not send the message %d",function_type);
						}
					}
					else if(LOCAL_OR_CENTER == CENTER_CAS)
					{
						if(PROJECT_MODE == ITC_HEB1)
						{
							adapter_tmt_send_TrainIdentifyNO(
								sqlite3_column_int(stmt, 2),
								sqlite3_column_int(stmt, 3),
								sqlite3_column_int(stmt, 4),
								sqlite3_column_int(stmt, 5),
								sqlite3_column_int(stmt, 6),
								function_type,
								sqlite3_column_int(stmt, 8),
								sqlite3_column_int(stmt, 9),
								sqlite3_column_int(stmt, 10),
								sqlite3_column_int(stmt, 11),
								sqlite3_column_int(stmt, 12));
						}
						else if(PROJECT_MODE == CBTC)
						{
							adapter_tmt_send_TrainIdentifyNO_ForCBTC(
								sqlite3_column_int(stmt, 2),
								sqlite3_column_int(stmt, 3),
								sqlite3_column_int(stmt, 4),
								sqlite3_column_int(stmt, 5),
								sqlite3_column_int(stmt, 6),
								function_type,
								sqlite3_column_int(stmt, 8),
							//	sqlite3_column_int(stmt, 9),
								sqlite3_column_int(stmt, 10),
								sqlite3_column_int(stmt, 11),
								sqlite3_column_int(stmt, 12),
								sqlite3_column_int(stmt, 13),
								sqlite3_column_int(stmt, 14),
								sqlite3_column_int(stmt, 15),
								sqlite3_column_int(stmt, 16));
						}
					}

				}
				if(function_type == 11586) //ATR ---dwelltriptask
				{
	//				ILOG("Get Sendtasklist->ATRDwellTripTimeTask sucess: msgtype=%d,machineNO=%d,"
	//						"componentNO=%d,consoleNo=%d,interlockZone=%d,disposalNo=%d,"
	//						"functionNo=%d,elementType=%d,TWCIndex=%d,nextPTID=%d,runClass=%d,PTID=%d",
	//						msg_type,
	//						sqlite3_column_int(stmt, 2),
	//						sqlite3_column_int(stmt, 3),
	//						sqlite3_column_int(stmt, 4),
	//						sqlite3_column_int(stmt, 5),
	//						sqlite3_column_int(stmt, 6),
	//						function_type,
	//						sqlite3_column_int(stmt, 8),
	//						sqlite3_column_int(stmt, 9),
	//						sqlite3_column_int(stmt, 10),
	//						sqlite3_column_int(stmt, 11),
	//						sqlite3_column_int(stmt, 12));



					if(LOCAL_OR_CENTER == LOCAL_TRC)/*1:center; 0:local*/
					{
						int interlock_number = sqlite3_column_int(stmt, 5);

						int send_judge = mpusender_trcsend_judge(db,interlock_number);

						if(send_judge == 0)
						{
							if(PROJECT_MODE == ITC_HEB1)
							{
								adapter_atr_dwell_trip_time(
									sqlite3_column_int(stmt, 2),
									sqlite3_column_int(stmt, 3),
									sqlite3_column_int(stmt, 4),
									sqlite3_column_int(stmt, 5),
									sqlite3_column_int(stmt, 6),
									function_type,
									sqlite3_column_int(stmt, 8),
									sqlite3_column_int(stmt, 9),
									sqlite3_column_int(stmt, 10),
									sqlite3_column_int(stmt, 11));
							}
							else if(PROJECT_MODE == CBTC)
							{
								adapter_atr_dwell_trip_time_ForCBTC(
									sqlite3_column_int(stmt, 2),
									sqlite3_column_int(stmt, 3),
									sqlite3_column_int(stmt, 4),
									sqlite3_column_int(stmt, 5),
									sqlite3_column_int(stmt, 6),
									function_type,
									sqlite3_column_int(stmt, 8),
									sqlite3_column_int(stmt, 15),
									sqlite3_column_int(stmt, 10),
									sqlite3_column_int(stmt, 12),
									sqlite3_column_int(stmt, 13),
									sqlite3_column_int(stmt, 14),
									sqlite3_column_int(stmt, 16),
									sqlite3_column_int(stmt, 17));

							}
							ILOG(" TRC  send the message %d",function_type);
						}
						else
						{
							ILOG(" TRC do not send the message %d",function_type);
						}
					}
					else if(LOCAL_OR_CENTER == CENTER_CAS)
					{
						if(PROJECT_MODE == ITC_HEB1)
						{
							adapter_atr_dwell_trip_time(
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9),
							sqlite3_column_int(stmt, 10),
							sqlite3_column_int(stmt, 11));
						}
						else if(PROJECT_MODE == CBTC)
						{
							adapter_atr_dwell_trip_time_ForCBTC(
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 15),
							sqlite3_column_int(stmt, 10),
							sqlite3_column_int(stmt, 12),
							sqlite3_column_int(stmt, 13),
							sqlite3_column_int(stmt, 14),
							sqlite3_column_int(stmt, 16),
							sqlite3_column_int(stmt, 17));

					}
				}
			}
				if(function_type == 11581 || function_type == 11582) //ATR ---reversaltask
				{
					ILOG("Get Sendtasklist->ATRChangeTipOrReversalTask sucess: msgtype=%d,machineNO=%d,"
							"componentNO=%d,consoleNo=%d,interlockZone=%d,disposalNo=%d,"
							"functionNo=%d,elementType=%d,PTID=%d,strategy=%d",
							msg_type,
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9),
							sqlite3_column_int(stmt, 10));
					adapter_atr_change_tip_or_reversal(
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9),
							sqlite3_column_int(stmt, 10));
				}
				if(function_type == 6019 || function_type == 6020 || function_type == 6021) //ATR ---skiptask
				{
					ILOG("Get Sendtasklist->ATRAutoskipOrCancelTask sucess: msgtype=%d,machineNO=%d,"
							"componentNO=%d,consoleNo=%d,interlockZone=%d,disposalNo=%d,"
							"functionNo=%d,elementType=%d,PTID=%d,skipOrCancel=%d",
							msg_type,
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9),
							sqlite3_column_int(stmt, 10));
					adapter_atr_autoskip_or_cancelskip(
							sqlite3_column_int(stmt, 2),
							sqlite3_column_int(stmt, 3),
							sqlite3_column_int(stmt, 4),
							sqlite3_column_int(stmt, 5),
							sqlite3_column_int(stmt, 6),
							function_type,
							sqlite3_column_int(stmt, 8),
							sqlite3_column_int(stmt, 9),
							sqlite3_column_int(stmt, 10));
				}
		
			}
		}
		break;
		case 152:
//			ILOG("Get Sendtasklist->ATRSendTrainEventsTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,destination=%d,service=%d,serial=%d,"
//					"car1=%d,car2=%d,eventType=%d,station=%d,time=%lld,deTime=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7),
//					sqlite3_column_int(stmt, 8),
//					sqlite3_column_int(stmt, 9),
//					sqlite3_column_int(stmt, 10),
//					sqlite3_column_int(stmt, 11),
//					sqlite3_column_int(stmt, 12));
			adapter_atr_send_events(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12));
			break;

		case 202:
			trainDataTemp = (char *)sqlite3_column_blob(stmt, 22);
			train_count = trainDataTemp[0];
			ILOG("Get Sendtasklist->TMTSendAllTrainDataTask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,train_count=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					train_count);
			adapter_tmt_send_AllTrainDataStruct(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					(char *)sqlite3_column_blob(stmt,22));
			break;

		case 203:
//			ILOG("Get Sendtasklist->TMTSendTrainIDTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,destination=%d,service=%d,serial=%d,"
//					"driver=%d,line=%d,car1=%d,car2=%d,logicalSection=%d,"
//					"runWarp=%d,trainStatus=%d,trainIDFault=%d,driveMode=%d,carSpeed=%d,trainDirection=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7),
//					sqlite3_column_int(stmt, 8),
//					sqlite3_column_int(stmt, 9),
//					sqlite3_column_int(stmt, 10),
//					sqlite3_column_int(stmt, 11),
//					sqlite3_column_int(stmt, 12),
//					sqlite3_column_int(stmt, 13),
//					sqlite3_column_int(stmt, 14),
//					sqlite3_column_int(stmt, 15),
//					sqlite3_column_int(stmt, 16),
//					sqlite3_column_int(stmt, 17));
			adapter_tmt_send_trainID (
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14),
					sqlite3_column_int(stmt, 15),
					sqlite3_column_int(stmt, 16),
					sqlite3_column_int(stmt, 17),
					sqlite3_column_int(stmt, 18),
					sqlite3_column_int(stmt, 19),
					sqlite3_column_int(stmt, 20));
			break;

		case 211:
//			ILOG("Get Sendtasklist->TMTSendTrainIDModefyActTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,transationNO=%d,ack=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5));
//			adapter_tmt_send_TrainIDModefyAct(
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5));
			break;

		case 257:
//			ILOG("Get Sendtasklist->ATRSendModeTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,type=%d,lineNo=%d,trainIndex=%d,mode=%d,"
//					"para1=%d,para2=%d,destination=%d,serviceID=%d,serial=%d,car1=%d,car2=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7),
//					sqlite3_column_int(stmt, 8),
//					sqlite3_column_int(stmt, 9),
//					sqlite3_column_int(stmt, 10),
//					sqlite3_column_int(stmt, 11),
//					sqlite3_column_int(stmt, 12),
//					sqlite3_column_int(stmt, 13),
//					sqlite3_column_int(stmt, 14));
			adapter_atr_send_mode(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14));
			break;

		case 254:
//			ILOG("Get Sendtasklist->ATRSendLoadTimetableAct sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,timetableInnerNo=%d,errorNo=%d,successOrNo=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6));
			adapter_atr_send_load_timetable_act(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6));
			break;

		case 153:
//			ILOG("Get Sendtasklist->ATRSendLoadedPlanTimeTableTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,timetableInnernumber=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4));
			adapter_atr_send_loaded_plantimetable_to_tgi(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4));
			break;

		case 550:
//			ILOG("Get Sendtasklist->ATRSendTimeTableLoadResultTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,successorno=%d,timetableInnernumber=%d,versionNO=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6));
			adapter_atr_send_timetable_loadresult_to_online(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6));
			break;

		case 354:
			ILOG("Get Sendtasklist->ATRSendTimeTableLoadResultToTTPTask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,successorno=%d,timetableInnernumber=%d,versionNO=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6));
			adapter_atr_send_timetable_loadresult_to_TTP(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6));
			break;
		case 151:
//			ILOG("Get Sendtasklist->AtrRequireRunningMsgTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,timetableInnernumber=%d,activaltimetable_or_no=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5));
			adapter_atr_send_feedbackrunning(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5));
			break;

		case 156:
//			ILOG("Get Sendtasklist->ATRSendPredictLineTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,destination=%d,service=%d,serial=%d,"
//					"car1=%d,car2=%d,station=%d,time=%lld,predictValue=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7),
//					sqlite3_column_int(stmt, 8),
//					sqlite3_column_int(stmt, 9),
//					sqlite3_column_int(stmt, 10),
//					sqlite3_column_int(stmt, 11));
			adapter_atr_send_predictline(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11));
			break;

		case 259:
			trainDataTemp = (char *)sqlite3_column_text(stmt, 22);
			train_count = trainDataTemp[1];
//			ILOG("Get Sendtasklist->ATRSendAllTrainModeDataTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,train_count=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					train_count);
			adapter_atr_send_AllTrainModeDataStruct(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_text(stmt,22));
			break;

		case 261:
//			ILOG("Get Sendtasklist->ATRSendHMIOperateFeedbackTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,operateResult=%d,lineNo=%d,trainIndex=%d,"
//					"destination=%d,serviceID=%d,serial=%d,car1=%d,car2=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7),
//					sqlite3_column_int(stmt, 8),
//					sqlite3_column_int(stmt, 9),
//					sqlite3_column_int(stmt, 10),
//					sqlite3_column_int(stmt, 11));
			adapter_atr_send_HMIOperateFeedback(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11));
			break;

		case 262:
			ILOG("Get Sendtasklist->ATRSendTrainEventsToHMITask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,eventType=%d,lineNo=%d,ptid=%d,"
					"idle1=%d,idle2=%d,idle3=%d,idle4=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10));
			adapter_atr_send_trainEventToHMI(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10));
			break;


		case 601:
			trainDataTemp = (char *)sqlite3_column_blob(stmt, 22);
			train_count = trainDataTemp[0];
//			ILOG("Get Sendtasklist->TMTSendAllTrainDataToFepTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,train_count=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					train_count);
			adapter_tmt_send_AllTrainDataStructToFep(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					(char *)sqlite3_column_blob(stmt,22));
			break;

		case 602:
			stationDataTemp = (char *)sqlite3_column_blob(stmt, 22);
			station_count = stationDataTemp[0];
//			ILOG("Get Sendtasklist->ATRSendAllStationDataToFepTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,station_count=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					station_count);
//			char *p = NULL;
//			p = stationDataTemp;
//			int i =0;
//			for(i = 0;i< 44*station_count+1;i++)
//			{
//				ILOG("%x ",*((unsigned char *)p));
//				p++;
//			}
//			ILOG("i = %d", i);

			adapter_atr_send_AllStationDataStructToFep(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					(char *)sqlite3_column_blob(stmt,22));
			//DLOG("finish 2");
			break;

		case 702:
//			ILOG("Get Sendtasklist->TMTSendInterfaceTrackStatusToFepTask sucess: msgtype=%d,machineNO=%d,"
//					"componentNO=%d,trackid=%d,freestatus=%d,car1=%d,car2=%d",
//					msg_type,
//					sqlite3_column_int(stmt, 2),
//					sqlite3_column_int(stmt, 3),
//					sqlite3_column_int(stmt, 4),
//					sqlite3_column_int(stmt, 5),
//					sqlite3_column_int(stmt, 6),
//					sqlite3_column_int(stmt, 7));
			adapter_tmt_send_interfacetrackstatus_to_fep(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7));
					break;

		case 620:
			ILOG("Get Sendtasklist->ATRSendDTIStopTimeToFepTask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,innerElementType=%d,PTID=%d,nextPTID=%d,DTI_Index=%d,"
					"StopTime_Seconds=%d,Date_Hour=%d,Date_Minute=%d,Date_Second=%d,"
					"Is_CountDown=%d,Light_or_Dark=%d,CountDown_ModeValue=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14));
			adapter_atr_send_DTIStopTime(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14));
					break;
		case 50:                        //Added by lfc 20121107
			ILOG("Get Sendtasklist->CoAlarmTask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,AlarmClass=%d,ElementType=%d,ElementID=%d,AlarmID=%d,"
					"AlarmClose=%d,Parameter=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14),
					sqlite3_column_int(stmt, 15));
			adapter_co_send_AlarmMessage(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5),
					sqlite3_column_int(stmt, 6),
					sqlite3_column_int(stmt, 7),
					sqlite3_column_int(stmt, 8),
					sqlite3_column_int(stmt, 9),
					sqlite3_column_int(stmt, 10),
					sqlite3_column_int(stmt, 11),
					sqlite3_column_int(stmt, 12),
					sqlite3_column_int(stmt, 13),
					sqlite3_column_int(stmt, 14),
					sqlite3_column_int(stmt, 15));
			break;
		case 651:
			DLOG("PROCESSOR 651");
			trainDataTemp = (char *)sqlite3_column_blob(stmt, 22);
			DLOG("PROCESSOR 651  1");
			train_count = trainDataTemp[0];
			DLOG("PROCESSOR 651  2");
			ILOG("Get Sendtasklist->TMTSendAllTrainDataTask sucess: msgtype=%d,machineNO=%d,"
					"componentNO=%d,train_count=%d",
					msg_type,
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					train_count);
			adapter_tmt_send_SynAllTrainDataStruct(
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					(char *)sqlite3_column_blob(stmt,22));
			break;
//		case 205:
//			ILOG("Get Sendtasklist->atrCASToHMITask success: msgtype=%d,machineNO=%d,"
//										"componentNO=%d,type=%d",
//										msg_type,
//										sqlite3_column_int(stmt, 2),
//										sqlite3_column_int(stmt, 3),
//										sqlite3_column_int(stmt, 4));
//			adapter_co_send_CASToHMIMessage(
//										sqlite3_column_int(stmt, 2),
//										sqlite3_column_int(stmt, 3),
//										sqlite3_column_int(stmt, 4));
//			break;
		default:
			break;
		}
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);


	if(PL_ACTIVE == 1)
	{
		adapter_pl_send_PlCommandDatagram(db);
	}
	//DLOG("finish 3");
	memset(sqlstr, 0, SQL_STR_LEN);
	sprintf(sqlstr, "delete from SendTaskList");
	rc = sqlite3_exec(db, sqlstr, 0, 0, NULL);
	if (SQLITE_OK != rc)
	{
		ELOG("finish error 1 rc = %d",rc);
		return -1;
	}
	//DLOG("finish 4");
	return 0;
}



int mpuSender_SendNetStatus_process (sqlite3 *db)
{
	int rc;
    sqlite3_stmt *stmt;
    char sqlstr[SQL_STR_LEN];
    char *tempstr = NULL;
    //int CASConnectionNum = 42;
    int i = 1;
    int netstatus = 0;

    if (db == NULL)
	{
    	return -1;
	}

    AllNetStatusToHMI message;
    memset(&message,0,sizeof(AllNetStatusToHMI));
    BYTE buf = 0;

    message.cas_equipmentID_low =  short_to_byte(CAS_EQUIPMENTID, 1);
    message.cas_equipmentID_high =  short_to_byte(CAS_EQUIPMENTID, 2);

    while(i <= CASConnectionNum)
    {
       memset(sqlstr, 0, SQL_STR_LEN);
       sprintf(sqlstr, "select NetStatus from CASNetStatus where ConnectionID = '%d'",i);
       rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
       rc = sqlite3_step(stmt);

       if (SQLITE_ROW == rc)
       {
    	   netstatus = sqlite3_column_int(stmt, 0);
    	   setBitsOfByte(&buf, (i-1)%8, 1, netstatus);
    	   //DLOG("buf[%d] = %d", i-1,netstatus);
    	   if(i == 8)
    	   {
    		   message.cas_netstatus_1 = countValueofBits(buf, 0, 8);
    		   //message.cas_netstatus_1 = bytes2int(&buf[0], CHAR2BYTE);
    		   DLOG("message.cas_netstatus_1 = %d", message.cas_netstatus_1);
    	   }
    	   if(i == 16)
		   {
			   message.cas_netstatus_2 = countValueofBits(buf, 0, 8);
			   //message.cas_netstatus_1 = bytes2int(&buf[0], CHAR2BYTE);
			   DLOG("message.cas_netstatus_2 = %d", message.cas_netstatus_2);
		   }
    	   if(i == 24)
		   {
			   message.cas_netstatus_3 = countValueofBits(buf, 0, 8);
			   //message.cas_netstatus_1 = bytes2int(&buf[0], CHAR2BYTE);
			   DLOG("message.cas_netstatus_3 = %d", message.cas_netstatus_3);
		   }
    	   if(i == 32)
		   {
			   message.cas_netstatus_4 = countValueofBits(buf, 0, 8);
			   //message.cas_netstatus_1 = bytes2int(&buf[0], CHAR2BYTE);
			   DLOG("message.cas_netstatus_4 = %d", message.cas_netstatus_4);
		   }
    	   if(i == 40)
		   {
			   message.cas_netstatus_5 = countValueofBits(buf, 0, 8);
			   //message.cas_netstatus_1 = bytes2int(&buf[0], CHAR2BYTE);
			   DLOG("message.cas_netstatus_5 = %d", message.cas_netstatus_5);
		   }
    	   if(i == 46)
		   {
			   message.cas_netstatus_6 = countValueofBits(buf, 0, 8);
			   DLOG("message.cas_netstatus_6 = %d", message.cas_netstatus_6);
		   }
       }
       else
       {
    	   sqlite3_finalize(stmt);
    	   return -1;
       }
       sqlite3_finalize(stmt);
       i++;
    }
	DLOG("message.cas_netstatus_7");
	message.lfep1_equipmentID_low =  short_to_byte(LFEP1_EQUIPMENTTYPE, 1);
	message.lfep1_equipmentID_high =  short_to_byte(LFEP1_EQUIPMENTTYPE, 2);
	memset(sqlstr, 0, SQL_STR_LEN);
	sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",LFEP1_EQUIPMENTTYPE);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	rc = sqlite3_step(stmt);
	DLOG("message.cas_netstatus_8");
	if (SQLITE_ROW == rc)
	{
	   tempstr = (char *)sqlite3_column_blob(stmt, 0);
	   if(tempstr == NULL)
	   {
		   message.lfep1_netstatus_1 = 0;
		   message.lfep1_netstatus_2 = 0;
		   message.lfep1_netstatus_3 = 0;
		   message.lfep1_netstatus_4 = 0;
	   }
	   else
	   {
	   message.lfep1_netstatus_1 = tempstr[0];
	   message.lfep1_netstatus_2 = tempstr[1];
	   message.lfep1_netstatus_3 = tempstr[2];
	   message.lfep1_netstatus_4 = tempstr[3];
	   }
   }
   else
   {
      sqlite3_finalize(stmt);
      return -1;
   }
   sqlite3_finalize(stmt);
   DLOG("message.cas_netstatus_9");
  message.lfep2_equipmentID_low =  short_to_byte(LFEP2_EQUIPMENTTYPE, 1);
  message.lfep2_equipmentID_high =  short_to_byte(LFEP2_EQUIPMENTTYPE, 2);
  memset(sqlstr, 0, SQL_STR_LEN);
  sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",LFEP2_EQUIPMENTTYPE);
  rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
  rc = sqlite3_step(stmt);

  if (SQLITE_ROW == rc)
  {
   tempstr = (char *)sqlite3_column_blob(stmt, 0);
   if(tempstr == NULL)
   {
	   message.lfep2_netstatus_1 = 0;
	   message.lfep2_netstatus_2 = 0;
	   message.lfep2_netstatus_3 = 0;
	   message.lfep2_netstatus_4 = 0;
   }
   else
   {
   message.lfep2_netstatus_1 = tempstr[0];
   message.lfep2_netstatus_2 = tempstr[1];
   message.lfep2_netstatus_3 = tempstr[2];
   message.lfep2_netstatus_4 = tempstr[3];
   }
  }
  else
  {
	 sqlite3_finalize(stmt);
	 return -1;
  }
  sqlite3_finalize(stmt);
  DLOG("message.cas_netstatus_10");
  message.lfep3_equipmentID_low =  short_to_byte(LFEP3_EQUIPMENTTYPE, 1);
 message.lfep3_equipmentID_high =  short_to_byte(LFEP3_EQUIPMENTTYPE, 2);
 memset(sqlstr, 0, SQL_STR_LEN);
 sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",LFEP3_EQUIPMENTTYPE);
 rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
 rc = sqlite3_step(stmt);

 if (SQLITE_ROW == rc)
 {
   tempstr = (char *)sqlite3_column_blob(stmt, 0);
   if(tempstr == NULL)
   {
	   message.lfep3_netstatus_1 = 0;
	   message.lfep3_netstatus_2 = 0;
	   message.lfep3_netstatus_3 = 0;
	   message.lfep3_netstatus_4 = 0;
   }
   else
   {
   message.lfep3_netstatus_1 = tempstr[0];
   message.lfep3_netstatus_2 = tempstr[1];
   message.lfep3_netstatus_3 = tempstr[2];
   message.lfep3_netstatus_4 = tempstr[3];
   }
 }
 else
 {
	sqlite3_finalize(stmt);
	return -1;
 }
 sqlite3_finalize(stmt);
 DLOG("message.cas_netstatus_11");
 message.lfep4_equipmentID_low =  short_to_byte(LFEP4_EQUIPMENTTYPE, 1);
message.lfep4_equipmentID_high =  short_to_byte(LFEP4_EQUIPMENTTYPE, 2);
memset(sqlstr, 0, SQL_STR_LEN);
sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",LFEP4_EQUIPMENTTYPE);
rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
rc = sqlite3_step(stmt);

if (SQLITE_ROW == rc)
{
   tempstr = (char *)sqlite3_column_blob(stmt, 0);
   if(tempstr == NULL)
   {
	   message.lfep4_netstatus_1 = 0;
	   message.lfep4_netstatus_2 = 0;
	   message.lfep4_netstatus_3 = 0;
	   message.lfep4_netstatus_4 = 0;
   }
   else
   {
   message.lfep4_netstatus_1 = tempstr[0];
   message.lfep4_netstatus_2 = tempstr[1];
   message.lfep4_netstatus_3 = tempstr[2];
   message.lfep4_netstatus_4 = tempstr[3];
   }
}
else
{
   sqlite3_finalize(stmt);
   return -1;
}
sqlite3_finalize(stmt);
DLOG("message.cas_netstatus_12");
   message.cfep1_equipmentID_low =  short_to_byte(CFEP1_EQUIPMENTTYPE, 1);
   message.cfep1_equipmentID_high =  short_to_byte(CFEP1_EQUIPMENTTYPE, 2);
   memset(sqlstr, 0, SQL_STR_LEN);
   sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",CFEP1_EQUIPMENTTYPE);
   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);

	if (SQLITE_ROW == rc)
	{
		tempstr = (char *)sqlite3_column_blob(stmt, 0);
		if(tempstr == NULL)
		{
		   message.cfep1_netstatus_1 = 0;
		}
		else
		{
		   message.cfep1_netstatus_1 = tempstr[0];
		}
	}
	else
	{
		sqlite3_finalize(stmt);
		return -1;
	}
	sqlite3_finalize(stmt);
	DLOG("message.cas_netstatus_13");
	message.cfep2_equipmentID_low =  short_to_byte(CFEP2_EQUIPMENTTYPE, 1);
	message.cfep2_equipmentID_high =  short_to_byte(CFEP2_EQUIPMENTTYPE, 2);
	memset(sqlstr, 0, SQL_STR_LEN);
	sprintf(sqlstr, "select StatusInfo from LCfepNetStatus where ElementType = '%d'",CFEP2_EQUIPMENTTYPE);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	rc = sqlite3_step(stmt);

	if (SQLITE_ROW == rc)
	{
		tempstr = (char *)sqlite3_column_blob(stmt, 0);
		if(tempstr == NULL)
		{
		  message.cfep2_netstatus_1 = 0;
		}
		else
		{
		  message.cfep2_netstatus_1 = tempstr[0];
		}
	}
	else
	{
		 sqlite3_finalize(stmt);
		 return -1;
	}
	sqlite3_finalize(stmt);

	adapter_net_send_AllNetStatusToHMI(
		  SEND_NETSTATUS_TOHMI_MACHINENO,
		  0,
		  message);
	DLOG("Send netstatus cas_netstatus_1 = %d ,cas_netstatus_2 = %d ,cas_netstatus_3 = %d"
		  ",cas_netstatus_4 = %d ,cas_netstatus_5 = %d ,cas_netstatus_6 = %d",
		  message.cas_netstatus_1,message.cas_netstatus_2,message.cas_netstatus_3,
		  message.cas_netstatus_4,message.cas_netstatus_5,message.cas_netstatus_6);
	DLOG("Send net status message to hmi %d",SEND_NETSTATUS_TOHMI_MACHINENO);
	return 0;
}

/*******************************************************************
 Function name		mpusender_trcsend_judge
 description		cas or trc send the task or not
 parameter
 sqlite3 *				IN/OUT			db
 int 					IN				interlock_num
 Return value
 true or false
 *******************************************************************/
int mpusender_trcsend_judge(sqlite3 *db,int interlock_num)/*0:send;-1:do not send*/
{
	int rc=0;
	int Interlockstatus=0;
	int heartbeatstatus=0;

	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN];
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "select Interlockstatus,heartbeatstatus from CONTROLRIGHTAUTO where Interlockid = '%d'" ,interlock_num);
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	rc = sqlite3_step(stmt);

	if (SQLITE_ROW == rc)
	{
		Interlockstatus = sqlite3_column_int(stmt, 0);
		heartbeatstatus = sqlite3_column_int(stmt, 1);
	}
	else
	{
		FLOG("CONTROLRIGHTAUTO fatal error");
		sqlite3_finalize(stmt);
		return -1;
	}
	sqlite3_finalize(stmt);



	if(Interlockstatus == 70 || Interlockstatus == 71 || Interlockstatus == 72 || Interlockstatus == 73)
	{
		 return -1;/*center control,TRC do not send message*/
	}

	else if(Interlockstatus == 0)
	{
		 return 0;/*TRC send this message*/
	}

	/*******************************************
	 * receive heart beat from CAS
	 * 0-->0:heartbeatstatus:1
	 * 0-->1:heartbeatstatus:2
	 * 1-->0:heartbeatstatus:3
	 * 1-->1:heartbeatstatus:4
	 *********************************************/
	else if(Interlockstatus == 3)
	{
		if(heartbeatstatus == 2 || heartbeatstatus == 4 )
		{
		   return -1;/*TRC take off control*/
		}
		else
		{
			return 0;/*TRC send this message*/
		}
	}
	return 0;

}

