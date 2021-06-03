 /*
 * mpusendmessage.c
 *
 *  Created on: 2012-6-22
 *      Author: root
 */
#include "include/MPUSendMessage.h"
#include <time.h>
#include <sqlite3.h>
#include "include/MPU.h"
#include "../dllmain.h"

sqlite3 *db_replay_adapter = 0;
#define MAX_SQL_STR_LEN     1024
/*******************************************************************
 Function name		adapter_pl_send_PlCommandDatagram
 description			send the command after convert by pl
 parameter
 sqlite3 *				IN/OUT			db
 Return value
 true or false
 *******************************************************************/
void adapter_pl_send_PlCommandDatagram(sqlite3 *db)
{
	 int trc_equipment_id = 3; //define temp
	 int interlock_equipment_id [] = {60,61,62,63};
	 int DatagramLength =0;
	 unsigned char DatagramBuffer[4000]={0};
	 unsigned char szbuf[5000] = {0};

	 //get cmd send to JNET
	 int i=1;
	 for(;i<=INTERLOCK_REGION_COUNT;i++)
	 {
		  int send_judge =mpusender_trcsend_judge(db,i);
		  //send_judge=0;
		  if(send_judge!=0)
			  return;
		  DLOG("PL GetAtsCommandDatagram InterlockID= %d", i);
		  GetAtsCommandDatagram(i, 1,DatagramBuffer,&DatagramLength,interlock_equipment_id[i-1],trc_equipment_id);//TRC Equipment ID
		  //GetAtsCommandDatagram(2, 1,DatagramBuffer,&DatagramLength,1,1);
		  //DLOG("PL GetAtsCommandDatagram InterlockID");
		  NetHead netHeader;
		  memset(&netHeader, 0, sizeof(netHeader));
		  netHeader.datalen_one=DatagramLength&0xFF;
		  netHeader.datalen_two=(DatagramLength>>8)&0xFF;
		  netHeader.datatype_one=2;//out type
		  netHeader.datatype_two=0;
		  netHeader.destno =interlock_equipment_id[i-1];
		  netHeader.protocoltype=12;// PL send use 12
		  netHeader.srcno=trc_equipment_id;//TRC Terminal ID

		  memcpy(szbuf, &netHeader, sizeof(NetHead));
		  memcpy(szbuf + sizeof(NetHead), DatagramBuffer, DatagramLength);
		  DLOG("PL send length is: %d", DatagramLength);
		  if(LOCAL_OR_CENTER == CENTER_CAS)
		  {
			  adapter_writen(app2netfd,szbuf,sizeof(NetHead) + DatagramLength);
			  ILOG("write 60 61 62 63 from cnet");
		  }else if(LOCAL_OR_CENTER == LOCAL_TRC)
		  {
			  adapter_writen(app2TSnetfd,szbuf,sizeof(NetHead) + DatagramLength);
			  ILOG("write 60 61 62 63 from jnet");
		  }


	 }
}
/*******************************************************************
 Function name		ats_co_Update_TMTcommand_for_repalydb
 description			for repaydb-TMTcommand
 parameter
 sqlite3 *				IN/OUT			db
 int 					IN				trackindex
 int 					IN				status
 char *					IN/OUT			datainfo
 int 					IN				datalen
 Return value
 true or false
 *******************************************************************/
//for repaydb-TMTcommand
int
ats_co_Update_TMTcommand_for_repalydb(sqlite3 *db, int trackindex, int status, char *datainfo, int datalen)
{
	if ((NULL == db)||(NULL == datainfo))
	{
		return -1;
	}
	char sqlstr[MAX_SQL_STR_LEN];
	sqlite3_stmt *stmt;

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	unsigned long time =(tv.tv_sec+abs(tz.tz_minuteswest*60))*1000+tv.tv_usec/1000;

	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	sprintf(sqlstr, "insert into T_TMTDATA(TrackIndex, Status, Time,Data) values('%d', '%d','%lu', ?)",
			trackindex,
			status,
			time);
	sqlite3_prepare(db, sqlstr, -1, &stmt, NULL);
	sqlite3_bind_blob(stmt,1,datainfo, datalen,NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	//ILOG("add tmtdata for replaydb sucess: trackindex=%d status=%d",trackindex,status);
	return 0;
}

void  adapter_net_send_AllNetStatusToHMI(
		int machineNO,
		int componentNO,
		AllNetStatusToHMI message)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(AllNetStatusToHMI), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 61, sizeof(AllNetStatusToHMI)); //61

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			&message, sizeof(AllNetStatusToHMI) );

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(AllNetStatusToHMI));
}


// interface for tmt as follows.............................................

void adapter_tmt_send_loadRequest(
		int machineNO,
		int componentNO,
		char interlockingZone,
		int sd,
		int fi,
		int rc,
		int timeYesOrNo)
{

	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	unsigned char body = 0;
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(LoadRequest), PROTOCOL_TCP);
	LoadRequest loadRequest;
	memset(&loadRequest, 0, sizeof(LoadRequest));
	loadRequest.PDI = 1;
	loadRequest.datatype_low = short_to_byte(518, 1);
	loadRequest.datatype_high = short_to_byte(518, 2);
	loadRequest.datalen = 1;
	if(0 != sd)
	{
		body = body | 0x01;
	}
	if(0 != fi)
	{
		body = body | 0x02;
	}
	if(0 != rc)
	{
		body = body | 0x04;
	}
	if(0 != timeYesOrNo)
	{
		timeYesOrNo = timeYesOrNo | 0x08;
	}
	loadRequest.body = body;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &loadRequest, sizeof(LoadRequest));

	adapter_writen(app2netfd, szbuf, sizeof(NetHead) + sizeof(LoadRequest));
}

void  adapter_tmt_send_TmtFaultInfo(
		int machineNO,
		int componentNO,
		int deviceType,
		int faultType,
		int second,
		int minute,
		int hour,
		int day,
		int month,
		int year,
		int fs,
		int faultNO,
		int operatorNO,
		int orderCounter,
		unsigned char* parameter,
		int parameterLength)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(TmtFaultInfo) + parameterLength, PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 50, sizeof(TmtFaultInfo) + parameterLength); //??????????????????50???

	TmtFaultInfo tmtFaultInfo;
	tmtFaultInfo.deviceType = deviceType;
	tmtFaultInfo.faultType = faultType;
	tmtFaultInfo.second = second;
	tmtFaultInfo.minute = minute;
	tmtFaultInfo.hour = hour;
	tmtFaultInfo.day = day;
	tmtFaultInfo.month = month;
	tmtFaultInfo.year_low = short_to_byte(year, 1);
	tmtFaultInfo.year_high = short_to_byte(year, 2);
	if(fs)
	{
		faultNO += 128;
	}
	tmtFaultInfo.faultNO = faultNO;
	tmtFaultInfo.operatorNO = operatorNO;
	tmtFaultInfo.orderCounter_low = short_to_byte(orderCounter,1);
	tmtFaultInfo.orderCounter_high = short_to_byte(orderCounter,2);
	unsigned char* ptmp = (unsigned char*)malloc(parameterLength);
	int i = 0;
	for(;i < parameterLength;i++)
	{
		ptmp[i] = ((unsigned char*)parameter)[i];
	}

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &tmtFaultInfo,
			sizeof(TmtFaultInfo));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TmtFaultInfo), ptmp,
			parameterLength);

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TmtFaultInfo) + parameterLength );
}

void  adapter_tmt_send_TrainIDModefyAct(
		int machineNO,
		int componentNO,
		int transationNO,
		int ack)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	struct
	{
		unsigned char transationNO;
		unsigned char ack;
	}noModefyError;
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(noModefyError), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 211, sizeof(noModefyError)); //TMT??????????????????????????????????????????????????????????????????211???

	noModefyError.transationNO = transationNO;
	noModefyError.ack = ack;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &noModefyError,
			sizeof(noModefyError));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(noModefyError));
}

/*******************************************************************
 Function name		adapter_tmt_send_CasheartbeatMSG
 description	 send cas heart beat msg
 parameter

 Return value
 true or false
 *******************************************************************/
void adapter_tmt_send_CasheartbeatMSG()
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	if(LOCAL_OR_CENTER == CENTER_CAS )
	{
		adapter_net_head_package(&stNetHead, 207, INSIDE,
			sizeof(CasheartbeatMsg)+sizeof(AppHeadIn), PROTOCOL_MULTICAST);
		ILOG("send to fep from CAS multicast 207");
	}
	else if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
		adapter_net_head_package(&stNetHead, 208, INSIDE,
				sizeof(CasheartbeatMsg)+sizeof(AppHeadIn), PROTOCOL_MULTICAST);
		ILOG("send to fep from TRC multicast 208");
	}
	CasheartbeatMsg CASheartbeat;
	memset(&CASheartbeat, 0, sizeof(CasheartbeatMsg));

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 1000, sizeof(CasheartbeatMsg)); //61

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			&CASheartbeat, sizeof(CasheartbeatMsg) );
	//ILOG("CASheartbeat msg : %s",szbuf);
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(CasheartbeatMsg));

}

/*******************************************************************
 Function name		adapter_mpu_send_TrainIDSYCMSG
 description	 send train id sync msg
 parameter

 Return value
 true or false
 *******************************************************************/
void adapter_mpu_send_TrainIDSYCMSG()
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));

	adapter_net_head_package(&stNetHead, 70, INSIDE,
			sizeof(Center_TRC_SYNC)+sizeof(AppHeadIn), PROTOCOL_TCP);
//	ILOG("send to fep from TRC multicast 650");

	Center_TRC_SYNC CASheartbeat;
	memset(&CASheartbeat, 0, sizeof(Center_TRC_SYNC));

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 650, sizeof(Center_TRC_SYNC)); //61

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			&CASheartbeat, sizeof(Center_TRC_SYNC) );
	//ILOG("CASheartbeat msg : %s",szbuf);
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(Center_TRC_SYNC));

}
/*******************************************************************
 Function name		adapter_mpu_send_TrainIDSYCMSG
 description	 send train identify no msg
 parameter
		int 		IN		machineNO,
		int			IN		componentNO,
		int			IN		 consoleNO,
		int 		IN		interlockZone,
		int 		IN		disposalNO,
		int 		IN		functionNO,
		unsigned short 		IN		elementType,
		unsigned short		IN		TWCindex,
		unsigned short 		IN		destination,
		unsigned short 		IN		service,
		unsigned short 		IN		serial
 Return value
 true or false
 *******************************************************************/
void adapter_tmt_send_TrainIdentifyNO(
		int machineNO,
		int componentNO,
		int consoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
		unsigned short elementType,
		unsigned short TWCindex,
		unsigned short destination,
		unsigned short service,
		unsigned short serial)
{
	ILOG("adapter_tmt_send_TrainIdentifyNO:elementType=%d,TWCindex=%d,destination=%d,service=%d,serial=%d",elementType,TWCindex,destination,service,serial);
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(TrainIdentifyNO), PROTOCOL_TCP);
	TrainIdentifyNO trainIdentifyNO;
	memset(&trainIdentifyNO, 0, sizeof(TrainIdentifyNO));
	trainIdentifyNO.PDI = 1;
	trainIdentifyNO.datatype_low =  short_to_byte(512, 1);//
	trainIdentifyNO.datatype_high =  short_to_byte(512, 2);
	trainIdentifyNO.datalen = 23;
	trainIdentifyNO.consoleNO = extract(consoleNO,1);
	trainIdentifyNO.disposalNO = extract(disposalNO,1);
	trainIdentifyNO.functionNO_low = extract(functionNO,1);
	trainIdentifyNO.functionNO_high = extract(functionNO,2);
	trainIdentifyNO.elementType = elementType;//
	trainIdentifyNO.TWCIndex_low = short_to_byte(TWCindex, 1);
	trainIdentifyNO.TWCIndex_high = short_to_byte(TWCindex, 2);


	//BYTE buftemp_des[3];
	//BYTE buftemp_serv[3];
	int destination_ge,destination_shi,destination_bai;
	int service_ge,service_shi,service_bai;
	int serial_ge,serial_shi;

	//memset(buftemp_des, 0 , sizeof(BYTE)*3);
	//	memset(buftemp_serv, 0 , sizeof(BYTE)*3);
	//	if (int2bytes(buftemp_des, THR2BYTE, destination) != 0)
	//	return;
	//	if (int2bytes(buftemp_serv, THR2BYTE, service) != 0)
	//	return;
	//	destination_ge = buftemp_des[0];
	//	destination_shi = buftemp_des[1];
	//	destination_bai = buftemp_des[2];
	//	service_ge = buftemp_serv[0];
	//	service_shi = buftemp_serv[1];
	//	service_bai = buftemp_serv[2];

		destination_bai = destination/100;
		destination_shi = (destination - destination_bai*100)/10;
		destination_ge = destination - destination_bai*100 - destination_shi*10;
		service_bai = service/100;
		service_shi = (service - service_bai*100)/10;
		service_ge = service - service_bai*100 - service_shi*10;
		serial_shi = serial/10;
	    serial_ge = serial%10;

	if (setBitsOfByte(&trainIdentifyNO.destination_ID_low, 0, 4, destination_shi) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.destination_ID_low, 4, 4, destination_bai) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.service_ID_high_and_destination_ID_high, 0, 4, service_bai) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.service_ID_high_and_destination_ID_high, 4, 4, destination_ge) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.service_ID_low, 0, 4, service_ge) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.service_ID_low, 4, 4, service_shi) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.serial, 0, 4, serial_ge) != 0)
	return;
	if (setBitsOfByte(&trainIdentifyNO.serial, 4, 4, serial_shi) != 0)
	return;

	struct tm *tm_loc;
	time_t t;
	t = time(NULL);
	tm_loc = localtime(&t);
	trainIdentifyNO.second = tm_loc->tm_sec;
	trainIdentifyNO.minute = tm_loc->tm_min;
	trainIdentifyNO.hour = tm_loc->tm_hour;
	trainIdentifyNO.day = tm_loc->tm_mday;
	trainIdentifyNO.month = tm_loc->tm_mon;
	trainIdentifyNO.year = tm_loc->tm_year - 2000;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &trainIdentifyNO, sizeof(TrainIdentifyNO));
	printf("send trainID to TS!");//added on 20120314

	if(PL_ACTIVE == 1)
	{
		SetAtsCommandDatagram(interlockZone,(unsigned char *)(szbuf + sizeof(NetHead)),sizeof(TrainIdentifyNO),AUTO_COMMAND);
		return;
	}
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{

	}
	else if(LOCAL_OR_CENTER == CENTER_CAS)
	{
		int iret = adapter_writen(app2netfd,szbuf,sizeof(NetHead) + sizeof(TrainIdentifyNO));
		if  (0 < iret)
		{
			//DLOG("write %d bytes to fifo success!\n",iret);
		}
		else
		{
			DLOG("write fifo err!\n");
		}
	}
	//adapter_writen(app2netfd, szbuf, sizeof(NetHead) + sizeof(TrainIdentifyNO));
}
//add by quj 20121231
void adapter_tmt_send_TrainIdentifyNO_ForCBTC(
		int machineNO,
		int componentNO,
		int consoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
		unsigned short elementType,
//		unsigned short TWCindex,
		unsigned short destination,
		unsigned short service,
		unsigned short serial,
		unsigned short trainindex,
		unsigned short crewid,
		unsigned short lineid,
		unsigned short carid
		)
{
	ILOG("adapter_tmt_send_TrainIdentifyNO_CBTC:elementType=%d,trainindex=%d,destination=%d,"
			"service=%d,serial=%d,crewid=%d,lineid=%d,carid=%d",
			elementType,trainindex,destination,service,serial,crewid,lineid,carid);
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(TrainIdentifyNO_CBTC), PROTOCOL_TCP);
	DLOG("11586 net length = %d",sizeof(TrainIdentifyNO_CBTC));
	TrainIdentifyNO_CBTC trainIdentifyNO;
	memset(&trainIdentifyNO, 0, sizeof(TrainIdentifyNO_CBTC));
	trainIdentifyNO.PDI = 1;
	trainIdentifyNO.datatype_low =  short_to_byte(512, 1);//
	trainIdentifyNO.datatype_high =  short_to_byte(512, 2);
	trainIdentifyNO.datalen = 22;
	trainIdentifyNO.consoleNO = extract(consoleNO,1);
	trainIdentifyNO.disposalNO = extract(disposalNO,1);
	trainIdentifyNO.functionNO_low = extract(functionNO,1);
	trainIdentifyNO.functionNO_high = extract(functionNO,2);
	trainIdentifyNO.elementType = elementType;//
	ILOG("adapter_tmt_send_TrainIdentifyNO_CBTC:trainindex=%d",trainindex);
	trainIdentifyNO.trainindex_low = short_to_byte(trainindex,1);
	trainIdentifyNO.trainindex_high = short_to_byte(trainindex,2);
	ILOG("adapter_tmt_send_TrainIdentifyNO_CBTC:trainindex_low=%d,trainindex_high=%d",
			trainIdentifyNO.trainindex_low,trainIdentifyNO.trainindex_high );
	trainIdentifyNO.destination_low = short_to_byte(destination,1);
	trainIdentifyNO.destination_high = short_to_byte(destination,2);
	trainIdentifyNO.service_low = short_to_byte(service,1);
	trainIdentifyNO.service_high = short_to_byte(service,2);
	trainIdentifyNO.serial_low = extract(serial,1);
	trainIdentifyNO.crewid_low = short_to_byte(crewid,1);
	trainIdentifyNO.crewid_high = short_to_byte(crewid,2);
	trainIdentifyNO.lineid = extract(lineid,1);
//	trainIdentifyNO.lineid_high = extract(lineid,2);
	trainIdentifyNO.carid_low = extract(carid,1);
	trainIdentifyNO.carid_high = extract(carid,2);
	trainIdentifyNO.serial_high = extract(serial,2);
//	trainIdentifyNO.TWCIndex_low = short_to_byte(TWCindex, 1);
//	trainIdentifyNO.TWCIndex_high = short_to_byte(TWCindex, 2);


	//BYTE buftemp_des[3];
	//BYTE buftemp_serv[3];
//	int destination_ge,destination_shi,destination_bai;
//	int service_ge,service_shi,service_bai;
//	int serial_ge,serial_shi;
//
//	//memset(buftemp_des, 0 , sizeof(BYTE)*3);
//	//	memset(buftemp_serv, 0 , sizeof(BYTE)*3);
//	//	if (int2bytes(buftemp_des, THR2BYTE, destination) != 0)
//	//	return;
//	//	if (int2bytes(buftemp_serv, THR2BYTE, service) != 0)
//	//	return;
//	//	destination_ge = buftemp_des[0];
//	//	destination_shi = buftemp_des[1];
//	//	destination_bai = buftemp_des[2];
//	//	service_ge = buftemp_serv[0];
//	//	service_shi = buftemp_serv[1];
//	//	service_bai = buftemp_serv[2];
//
//		destination_bai = destination/100;
//		destination_shi = (destination - destination_bai*100)/10;
//		destination_ge = destination - destination_bai*100 - destination_shi*10;
//		service_bai = service/100;
//		service_shi = (service - service_bai*100)/10;
//		service_ge = service - service_bai*100 - service_shi*10;
//		serial_shi = serial/10;
//	    serial_ge = serial%10;
//
//
//		if (setBitsOfByte(&trainIdentifyNO.trainindex_low, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.trainindex_low, 4, 4, service_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.trainindex_high, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.trainindex_high, 4, 4, service_shi) != 0)
//		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.destination_low, 0, 4, destination_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.destination_low, 4, 4, destination_bai) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.destination_high, 0, 4, service_bai) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.destination_high, 4, 4, destination_ge) != 0)
//		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.service_low, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.service_low, 4, 4, service_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.service_high, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.service_high, 4, 4, service_shi) != 0)
//		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.serial_low, 0, 4, serial_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.serial_low, 4, 4, serial_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.serial_high, 0, 4, serial_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.serial_high, 4, 4, serial_shi) != 0)
//		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.crewid_low, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.crewid_low, 4, 4, service_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.crewid_high, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.crewid_high, 4, 4, service_shi) != 0)
//		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.lineid, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.lineid, 4, 4, service_shi) != 0)
//		return;
////		if (setBitsOfByte(&trainIdentifyNO.lineid_high, 0, 4, service_ge) != 0)
////		return;
////		if (setBitsOfByte(&trainIdentifyNO.lineid_high, 4, 4, service_shi) != 0)
////		return;
//
//		if (setBitsOfByte(&trainIdentifyNO.carid_low, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.carid_low, 4, 4, service_shi) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.carid_high, 0, 4, service_ge) != 0)
//		return;
//		if (setBitsOfByte(&trainIdentifyNO.carid_high, 4, 4, service_shi) != 0)
//		return;
//
//
//
//	struct tm *tm_loc;
//	time_t t;
//	t = time(NULL);
//	tm_loc = localtime(&t);
//	trainIdentifyNO.second = tm_loc->tm_sec;
//	trainIdentifyNO.minute = tm_loc->tm_min;
//	trainIdentifyNO.hour = tm_loc->tm_hour;
//	trainIdentifyNO.day = tm_loc->tm_mday;
//	trainIdentifyNO.month = tm_loc->tm_mon;
//	trainIdentifyNO.year = tm_loc->tm_year - 2000;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &trainIdentifyNO, sizeof(TrainIdentifyNO_CBTC));
	printf("send trainID to TS!");//added on 20120314
	ILOG("adapter_tmt_send_TrainIdentifyNO_CBTC:trainindex_low=%d,trainindex_high=%d,"
			"destination_low=%d,destination_high=%d",
			trainIdentifyNO.trainindex_low,trainIdentifyNO.trainindex_high,
			trainIdentifyNO.destination_low,trainIdentifyNO.destination_high);
//	int i = 0;
//	for(i;i < 46;i++)
//	{
//		DLOG("11586--->%d:%d",i,szbuf[i]);
//	}
	adapter_writen(app2netfd, szbuf, sizeof(NetHead) + sizeof(TrainIdentifyNO_CBTC));


}




void adapter_tmt_send_trainID (
		int machineNO,
		int componentNO,
		int destination,
		int service,
		int serial,
        int driver,
        int line,
        int car1,
        int car2,
        int logicalSection,
        int runWarp,
        int trainStatus,
        int trainIDFault,
        int driveMode,
        int carSpeed,
        int trainDirection,
        int trainNO,
        int arsStatus,
        int regulationMode)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	unsigned char blobbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(TrainID), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 203, sizeof(TrainID)); //TMT?????????????????????????????????????????????203???

	TrainID trainID;
	memset(&trainID, 0, sizeof(TrainID));
	trainID.destination_ID_low = short_to_byte(destination, 1);
	trainID.destination_ID_high = short_to_byte(destination, 2);
	trainID.service_ID_low = short_to_byte(service, 1);
	trainID.service_ID_high = short_to_byte(service, 2);
	trainID.serial = serial;
	trainID.driver_low = short_to_byte(driver, 1);
	trainID.driver_high = short_to_byte(driver, 2);
	trainID.line = line;
	trainID.car_one_1 = int_to_byte(car1,1);
	trainID.car_one_2 = int_to_byte(car1,2);
	trainID.car_one_3 = int_to_byte(car1,3);
	trainID.car_two_1 = int_to_byte(car2,1);
	trainID.car_two_2 = int_to_byte(car2,2);
	trainID.car_two_3 = int_to_byte(car2,3);
	trainID.logicalSection_low = short_to_byte(logicalSection,1);
	trainID.logicalSection_high = short_to_byte(logicalSection,2);
	unsigned int runWarpTemp = runWarp;
	trainID.runWarp_1 = int_to_byte(runWarpTemp,1);
	trainID.runWarp_2 = int_to_byte(runWarpTemp,2);
	trainID.runWarp_3 = int_to_byte(runWarpTemp,3);
	trainID.runWarp_4 = int_to_byte(runWarpTemp,4);
	trainID.trainStatus = trainStatus;
	trainID.trainIDFault = trainIDFault;
	trainID.driveMode = driveMode;
	trainID.carSpeed_low = short_to_byte(carSpeed,1);
	trainID.carSpeed_high = short_to_byte(carSpeed,2);
	trainID.trainNO_low= short_to_byte(trainNO,1);
	trainID.trainNO_high=short_to_byte(trainNO,2);
	trainID.trainDirection=trainDirection;
	trainID.arsStatus=arsStatus;
	trainID.regulationMode=regulationMode;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &trainID,
			sizeof(TrainID));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TrainID));


	memcpy(blobbuf, &appHeadIn, sizeof(AppHeadIn));
	memcpy(blobbuf + sizeof(AppHeadIn), &trainID,
			sizeof(TrainID));
    if(db_replay_adapter != NULL)
    {
    	ats_co_Update_TMTcommand_for_repalydb(
			db_replay_adapter,
			logicalSection,
			trainIDFault,
			blobbuf,
			sizeof(AppHeadIn) + sizeof(TrainID));
    }
    else
    {
    	sqlite3_open("/dev/shm/ATSSQLREPLAY.db", &db_replay_adapter);
    	ats_co_Update_TMTcommand_for_repalydb(
    				db_replay_adapter,
    				logicalSection,
    				trainStatus,
    				blobbuf,
    				sizeof(AppHeadIn) + sizeof(TrainID));
    }
}

void  adapter_tmt_send_AllTrainDataStruct(
		int machineNO,
		int componentNO,
		char allTrainDataStruct[])
{
	ILOG("Be about to send alltrainID=%d,",componentNO);

	int train_num = allTrainDataStruct[0];
	int traindata_len = ADB_TRAININFO_UNIT_LEN*train_num+1;
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + traindata_len, PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
//	int tmp = sizeof(AllTrainDataByte);
//	tmp = sizeof(AllTrainDataByte) * iCount;
//	tmp = sizeof(AllTrainDataByte) * iCount + 1;
	adapter_app_head_package(&appHeadIn, 202, traindata_len);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			allTrainDataStruct, traindata_len );

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + traindata_len);
}

void  adapter_tmt_send_AllTrainDataStruct2(
		int machineNO,
		int componentNO,
		AllTrainDataStruct atd[],
		int iCount)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	int i = 0;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(AllTrainDataByte) * iCount + sizeof(unsigned char), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
//	int tmp = sizeof(AllTrainDataByte);
//	tmp = sizeof(AllTrainDataByte) * iCount;
//	tmp = sizeof(AllTrainDataByte) * iCount + 1;
	adapter_app_head_package(&appHeadIn, 202, sizeof(AllTrainDataByte) * iCount + sizeof(unsigned char));

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	unsigned char count = (unsigned char)iCount;

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &count,
			sizeof(unsigned char));

	AllTrainDataByte atdbyte;

	for(i = 0; i < iCount; i++)
	{
		memset(&atdbyte,0,sizeof(AllTrainDataByte));
		atdbyte.destination_low = short_to_byte(atd[i].destination, 1);
		atdbyte.destination_high = short_to_byte(atd[i].destination, 2);
		atdbyte.service_low = short_to_byte(atd[i].service,1);
		atdbyte.service_high = short_to_byte(atd[i].service,2);
		atdbyte.serial = atd[i].serial;
		atdbyte.line = atd[i].line;
		atdbyte.car_one_1 = int_to_byte(atd[i].car1,1);
		atdbyte.car_one_2 = int_to_byte(atd[i].car1,2);
		atdbyte.car_one_3 = int_to_byte(atd[i].car1,3);
		atdbyte.car_two_1 = int_to_byte(atd[i].car2,1);
		atdbyte.car_two_2 = int_to_byte(atd[i].car2,2);
		atdbyte.car_two_3 = int_to_byte(atd[i].car2,3);
		atdbyte.driver_low = short_to_byte(atd[i].driver,1);
		atdbyte.driver_high = short_to_byte(atd[i].driver,2);
		atdbyte.warpTime_low = int_to_byte(atd[i].warpTime,1);
		atdbyte.warpTime_high = int_to_byte(atd[i].warpTime,2);
		atdbyte.driveMode = atd[i].driveMode;
		atdbyte.trainIDFault = atd[i].trainIDFault;
		atdbyte.logicalSection_low = short_to_byte(atd[i].logicalSection,1);
		atdbyte.logicalSection_high = short_to_byte(atd[i].logicalSection,2);
		atdbyte.trainStatus = atd[i].trainStatus;
		memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(unsigned char) + sizeof(AllTrainDataByte) * i, &atdbyte,
				sizeof(AllTrainDataByte) );
	}
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(AllTrainDataByte) * iCount + sizeof(unsigned char));
}


void  adapter_tmt_send_AllTrainDataStructToFep(
		int machineNO,
		int componentNO,
		char allTrainDataStruct[])
{
	int train_num = allTrainDataStruct[0];
    int traindata_len = 16*train_num+1;

	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + traindata_len, PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 601, traindata_len);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			allTrainDataStruct, traindata_len );

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + traindata_len);
}

void adapter_tmt_send_interfacetrackstatus_to_fep (
		int machineNO,
		int componentNO,
        int trackid,
        int freestatus,
        int car1,
        int car2)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(InterfaceTrackStatus_To_Fep), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 702, sizeof(InterfaceTrackStatus_To_Fep));

	InterfaceTrackStatus_To_Fep interfaceTrackStatus_To_Fep;
	memset(&interfaceTrackStatus_To_Fep, 0, sizeof(InterfaceTrackStatus_To_Fep));
	interfaceTrackStatus_To_Fep.trackid_1 = int_to_byte(trackid,1);
	interfaceTrackStatus_To_Fep.trackid_2 = int_to_byte(trackid,2);
	interfaceTrackStatus_To_Fep.trackid_3 = int_to_byte(trackid,3);
	interfaceTrackStatus_To_Fep.trackid_4 = int_to_byte(trackid,4);
	interfaceTrackStatus_To_Fep.freestatus = freestatus;
	interfaceTrackStatus_To_Fep.car1_1 = short_to_byte(car1,1);
	interfaceTrackStatus_To_Fep.car1_2 = short_to_byte(car1,2);
	interfaceTrackStatus_To_Fep.car2_1 = short_to_byte(car2,1);
	interfaceTrackStatus_To_Fep.car2_2 = short_to_byte(car2,2);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &interfaceTrackStatus_To_Fep,
			sizeof(InterfaceTrackStatus_To_Fep));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(InterfaceTrackStatus_To_Fep));
}




// interface for atr as follows.............................................
void adapter_atr_send_loaded_plantimetable_to_tgi(
		unsigned short machine_number,
		unsigned short component_number,
		unsigned char loaded_inner_time_table_number)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machine_number,INSIDE,
			sizeof(AppHeadIn) + sizeof(LoadedPlanTimeTable), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 153, sizeof(LoadedPlanTimeTable)); //????????????153

	LoadedPlanTimeTable loadedPlanTimeTable;
	memset(&loadedPlanTimeTable, 0, sizeof(LoadedPlanTimeTable));
	loadedPlanTimeTable.timetable_innernumber = loaded_inner_time_table_number;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &loadedPlanTimeTable,
			sizeof(LoadedPlanTimeTable));
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(LoadedPlanTimeTable));
}

void adapter_atr_send_timetable_loadresult_to_online(
		int machineNO,
		int componentNO,
		unsigned char success_or_no,
		unsigned char timetable_innernumber,
		int versionNO)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO,INSIDE,
			sizeof(AppHeadIn) + sizeof(TimeTableLoadResult), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 550, sizeof(TimeTableLoadResult)); //????????????550

	TimeTableLoadResult timeTableLoadResult;
	memset(&timeTableLoadResult, 0, sizeof(TimeTableLoadResult));
	timeTableLoadResult.success_or_no = success_or_no;
	timeTableLoadResult.timetable_innernumber = timetable_innernumber;
	timeTableLoadResult.versionno_1 = int_to_byte(versionNO, 1);
	timeTableLoadResult.versionno_2 = int_to_byte(versionNO, 2);
	timeTableLoadResult.versionno_3 = int_to_byte(versionNO, 3);
	timeTableLoadResult.versionno_4 = int_to_byte(versionNO, 4);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &timeTableLoadResult,
			sizeof(TimeTableLoadResult));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TimeTableLoadResult));
}

//354
void adapter_atr_send_timetable_loadresult_to_TTP(
		int machineNO,
		int componentNO,
		unsigned char success_or_no,
		unsigned char timetable_innernumber,
		int versionNO)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO,INSIDE,
			sizeof(AppHeadIn) + sizeof(TimeTableLoadResultToTTP), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 354, sizeof(TimeTableLoadResultToTTP));

	TimeTableLoadResultToTTP timeTableLoadResult;
	memset(&timeTableLoadResult, 0, sizeof(TimeTableLoadResultToTTP));
	timeTableLoadResult.success_or_no = success_or_no;
	timeTableLoadResult.timetable_innernumber = timetable_innernumber;
	timeTableLoadResult.versionno_1 = int_to_byte(versionNO, 1);
	timeTableLoadResult.versionno_2 = int_to_byte(versionNO, 2);
	timeTableLoadResult.versionno_3 = int_to_byte(versionNO, 3);
	timeTableLoadResult.versionno_4 = int_to_byte(versionNO, 4);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &timeTableLoadResult,
			sizeof(TimeTableLoadResultToTTP));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TimeTableLoadResultToTTP));
}

void adapter_atr_send_feedbackrunning(
		int machineNO,
		int componentNO,
		unsigned char timetable_innernumber,
		int activaltimetable_or_no)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO,INSIDE,
			sizeof(AppHeadIn) + sizeof(FeedbackRunning), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 151, sizeof(FeedbackRunning)); //????????????151

	FeedbackRunning feedbackRunning;
	memset(&feedbackRunning, 0, sizeof(FeedbackRunning));
	feedbackRunning.timetable_innernumber = timetable_innernumber;
	feedbackRunning.activaltimetable_or_no = activaltimetable_or_no;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &feedbackRunning,
			sizeof(FeedbackRunning));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(FeedbackRunning));
}

void adapter_atr_send_load_result_to_online(
		unsigned short machine_number,
		unsigned short component_number,
		unsigned short loaded_inner_time_table_number,
		unsigned char error_number,
		unsigned char load_result)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machine_number,INSIDE,
			sizeof(AppHeadIn) + sizeof(ScheduleResult), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 600, sizeof(ScheduleResult)); //????????????600 ??????Online????????????????????????????????????

	ScheduleResult scheduleResult;
	memset(&scheduleResult, 0, sizeof(ScheduleResult));
	scheduleResult.loaded_inner_time_table_number_low = short_to_byte(
			loaded_inner_time_table_number, 1);
	scheduleResult.loaded_inner_time_table_number_high = short_to_byte(
			loaded_inner_time_table_number, 2);
	scheduleResult.error_number = error_number;
	scheduleResult.load_result = load_result;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &scheduleResult,
			sizeof(ScheduleResult));
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(ScheduleResult));
}

void adapter_atr_send_load_result_to_hmi(
		unsigned short machine_number,
		unsigned short component_number,
		unsigned short loaded_inner_time_table_number,
		unsigned char error_number,
		unsigned char load_result)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machine_number, INSIDE,
			sizeof(AppHeadIn) + sizeof(ScheduleResult), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 258, sizeof(ScheduleResult)); //????????????258 ATR?????????????????????????????????

	ScheduleResult scheduleResult;
	memset(&scheduleResult, 0, sizeof(ScheduleResult));
	scheduleResult.loaded_inner_time_table_number_low = short_to_byte(
			loaded_inner_time_table_number, 1);
	scheduleResult.loaded_inner_time_table_number_high = short_to_byte(
			loaded_inner_time_table_number, 2);
	scheduleResult.error_number = error_number;
	scheduleResult.load_result = load_result;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &scheduleResult,
			sizeof(ScheduleResult));
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(ScheduleResult));
}

void adapter_atr_send_events(
		int machineNO,
		int componentNO,
		unsigned short destination_ID,
		unsigned short service_ID,
		unsigned short serial,
		unsigned short car_number_1,
		unsigned short car_number_2,
		unsigned short event_type,
		unsigned short station_ID,
		u64 time,
		int deTime)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(TrainEvent), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 152, sizeof(TrainEvent)); //????????????152 ATR????????????????????????????????????

	TrainEvent trainEvent;
	memset(&trainEvent, 0, sizeof(TrainEvent));
	trainEvent.destination_ID_low = short_to_byte(destination_ID, 1);
	trainEvent.destination_ID_high = short_to_byte(destination_ID, 2);
	trainEvent.service_ID_low = short_to_byte(service_ID, 1);
	trainEvent.service_ID_high = short_to_byte(service_ID, 2);
	trainEvent.serial = extract(serial,1);
	trainEvent.car_number_1_low = short_to_byte(car_number_1, 1);
	trainEvent.car_number_1_high = short_to_byte(car_number_1, 2);
	trainEvent.car_number_2_low = short_to_byte(car_number_2, 1);
	trainEvent.car_number_2_high = short_to_byte(car_number_2, 2);
	trainEvent.event_type = extract(event_type,1);
	trainEvent.station_ID_low = short_to_byte(station_ID, 1);
	trainEvent.station_ID_high = short_to_byte(station_ID, 2);
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		trainEvent.time[i] = long_long_to_byte(time, i + 1);
	}
	trainEvent.deTime_1 = int_to_byte(deTime, 1);
	trainEvent.deTime_2 = int_to_byte(deTime, 2);
	trainEvent.deTime_3 = int_to_byte(deTime, 3);
	trainEvent.deTime_4 = int_to_byte(deTime, 4);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &trainEvent,
			sizeof(TrainEvent));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TrainEvent));
}

void adapter_atr_send_operation_over(
		unsigned short OC_machine_number,
		unsigned short OC_component_number)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, OC_machine_number, INSIDE, sizeof(AppHeadIn),
			PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 154, 0); //????????????154 ?????????????????? ATR????????????TGI

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	adapter_writen(app2netfd, szbuf, sizeof(NetHead) + sizeof(AppHeadIn));
}

void adapter_atr_send_mode(
		int machineNO,
		int componentNO,
		unsigned short type,
		unsigned short line_number,
		unsigned short train_index,
		unsigned short mode,
		int para1,
		int para2,
		int destination,
		int serviceID,
		int serial,
		int car1,
		int car2)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(TrainMode), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 257, sizeof(TrainMode)); //????????????257 ATR??????????????????????????????

	TrainMode trainMode;
	memset(&trainMode, 0, sizeof(TrainMode));
	trainMode.type = type;
	trainMode.line_number = line_number;
	trainMode.train_index_low = short_to_byte(train_index, 1);
	trainMode.train_index_high = short_to_byte(train_index, 2);
	trainMode.mode = mode;
	//20121130 modify by sunjh
	trainMode.para1_low = short_to_byte(para1, 1);
	trainMode.para1_high = short_to_byte(para1, 2);
	trainMode.para2_low = short_to_byte(para2, 1);
	trainMode.para2_high = short_to_byte(para2, 2);
	trainMode.destination_low = short_to_byte(destination, 1);
	trainMode.destination_high = short_to_byte(destination, 2);
	trainMode.serviceID_low = short_to_byte(serviceID, 1);
	trainMode.serviceID_high = short_to_byte(serviceID, 2);
	trainMode.serial = serial;
	trainMode.car_one_1 = short_to_byte(car1, 1);
	trainMode.car_one_2 = short_to_byte(car1, 2);
	trainMode.car_two_1 = short_to_byte(car2, 1);
	trainMode.car_two_2 = short_to_byte(car2, 2);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &trainMode,
			sizeof(TrainMode));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TrainMode));
}

void adapter_atr_send_predictline(
		int machineNO,
		int componentNO,
		unsigned short destination_ID,
		unsigned short service_ID,
		unsigned short serial,
		unsigned short car_number_1,
		unsigned short car_number_2,
		unsigned short station_ID,
		u64 time,
		int predictValue)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(PredictLine), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 156, sizeof(PredictLine)); //????????????156

	PredictLine predictLine;
	memset(&predictLine, 0, sizeof(PredictLine));
	predictLine.destination_ID_low = short_to_byte(destination_ID, 1);
	predictLine.destination_ID_high = short_to_byte(destination_ID, 2);
	predictLine.service_ID_low = short_to_byte(service_ID, 1);
	predictLine.service_ID_high = short_to_byte(service_ID, 2);
	predictLine.serial = extract(serial,1);
	predictLine.car_number_1_low = short_to_byte(car_number_1, 1);
	predictLine.car_number_1_high = short_to_byte(car_number_1, 2);
	predictLine.car_number_2_low = short_to_byte(car_number_2, 1);
	predictLine.car_number_2_high = short_to_byte(car_number_2, 2);
	predictLine.station_ID_low = short_to_byte(station_ID, 1);
	predictLine.station_ID_high = short_to_byte(station_ID, 2);
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		predictLine.time[i] = long_long_to_byte(time, i + 1);
	}
	predictLine.predictValue_low = short_to_byte(predictValue, 1);
	predictLine.predictValue_high = short_to_byte(predictValue, 2);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &predictLine,
			sizeof(PredictLine));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(PredictLine));
}

void  adapter_atr_send_AllTrainModeDataStruct(
		int machineNO,
		int componentNO,
		char allTrainModeDataStruct[])
{
	int train_num = allTrainModeDataStruct[1];
	int traindata_len = 11*train_num+7;

	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + traindata_len, PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 259, traindata_len);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			allTrainModeDataStruct, traindata_len );

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + traindata_len);
}

void  adapter_atr_send_AllTrainModeDataStruct2(
		int machineNO,
		int componentNO,
		AllTrainModeData atd[],
		int iCount)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	int i = 0;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(AllTrainModeDataByte) * iCount + sizeof(unsigned char), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 259, sizeof(AllTrainModeDataByte) * iCount + sizeof(unsigned char));

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	unsigned char count = (unsigned char)iCount;

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &count,
			sizeof(unsigned char));

	AllTrainModeDataByte atdbyte;

	for(i = 0; i < iCount; i++)
	{
		memset(&atdbyte,0,sizeof(AllTrainModeDataByte));
		atdbyte.line_number = atd[i].line_number;
		atdbyte.para1_low = short_to_byte(atd[i].para1, 1);
		atdbyte.para1_high = short_to_byte(atd[i].para1, 2);
		atdbyte.para2_low = short_to_byte(atd[i].para2, 1);
	    atdbyte.para2_high = short_to_byte(atd[i].para2, 2);
	    atdbyte.mode = atd[i].mode;
	    atdbyte.train_index_low = short_to_byte(atd[i].train_index, 1);
	    atdbyte.train_index_high = short_to_byte(atd[i].train_index, 2);
	    atdbyte.destination_ID_low = short_to_byte(atd[i].destination_ID, 1);
	    atdbyte.destination_ID_high = short_to_byte(atd[i].destination_ID, 2);
		atdbyte.service_ID_low = short_to_byte(atd[i].service_ID,1);
		atdbyte.service_ID_high = short_to_byte(atd[i].service_ID,2);
		atdbyte.serial = atd[i].serial;
		atdbyte.car_number_1_low = short_to_byte(atd[i].car_number_1, 1);
		atdbyte.car_number_1_high = short_to_byte(atd[i].car_number_1, 2);
		atdbyte.car_number_2_low = short_to_byte(atd[i].car_number_2, 1);
		atdbyte.car_number_2_high = short_to_byte(atd[i].car_number_2, 2);
		memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(unsigned char) + sizeof(AllTrainModeDataByte) * i, &atdbyte,
				sizeof(AllTrainModeDataByte) );
	}
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(AllTrainModeDataByte) * iCount + sizeof(unsigned char));
}

void adapter_atr_send_HMIOperateFeedback(
		int machineNO,
		int componentNO,
		unsigned short operateResult,
		unsigned short line_number,
		unsigned short train_index,
		int destination,
		int serviceID,
		int serial,
		int car1,
		int car2)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(HMIOperateFeedback), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 261, sizeof(HMIOperateFeedback)); //????????????261

	HMIOperateFeedback hmiOperateFeedback;
	memset(&hmiOperateFeedback, 0, sizeof(HMIOperateFeedback));
	hmiOperateFeedback.operateResult = operateResult;
	hmiOperateFeedback.line_number = line_number;
	hmiOperateFeedback.train_index_low = short_to_byte(train_index, 1);
	hmiOperateFeedback.train_index_high = short_to_byte(train_index, 2);
	hmiOperateFeedback.destination = destination;
	hmiOperateFeedback.serviceID = serviceID;
	hmiOperateFeedback.serial = serial;
	hmiOperateFeedback.car_one_1 = short_to_byte(car1, 1);
	hmiOperateFeedback.car_one_2 = short_to_byte(car1, 2);
	hmiOperateFeedback.car_two_1 = short_to_byte(car2, 1);
	hmiOperateFeedback.car_two_2 = short_to_byte(car2, 2);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &hmiOperateFeedback,
			sizeof(HMIOperateFeedback));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(HMIOperateFeedback));
}

/*******************************************************************
 Function name		adapter_atr_send_trainEventToHMI
 description	 atr  train event to hmi
 parameter

 Return value
 true or false
 *******************************************************************/
//20130513 sunjh
void adapter_atr_send_trainEventToHMI(
		int machineNO,
		int componentNO,
		int eventType,
		int lineNO,
		int PTID,
		int idle1,
		int idle2,
		int idle3,
		int idle4)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
		NetHead stNetHead;
		memset(&stNetHead, 0, sizeof(NetHead));
		adapter_net_head_package(&stNetHead, machineNO, INSIDE,
				sizeof(AppHeadIn) + sizeof(TrainEventToHMI), PROTOCOL_TCP);

		AppHeadIn appHeadIn;
		memset(&appHeadIn, 0, sizeof(AppHeadIn));
		adapter_app_head_package(&appHeadIn, 262, sizeof(TrainEventToHMI));

		TrainEventToHMI trainEventToHMI;
		memset(&trainEventToHMI, 0, sizeof(TrainEventToHMI));
		trainEventToHMI.eventtype = eventType;
		trainEventToHMI.line_number = lineNO;
		trainEventToHMI.ptid_low = short_to_byte(PTID, 1);
		trainEventToHMI.ptid_high = short_to_byte(PTID, 2);
		trainEventToHMI.idle_one = idle1;
		trainEventToHMI.idle_two = idle2;
		trainEventToHMI.idle_three = idle3;
		trainEventToHMI.idle_four = idle4;
		memcpy(szbuf, &stNetHead, sizeof(NetHead));
		memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
		memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &trainEventToHMI,
				sizeof(TrainEventToHMI));

		adapter_writen(app2netfd, szbuf,
				sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(TrainEventToHMI));
}
/*******************************************************************
 Function name		adapter_co_send_AlarmMessage
 description	 atr  alarm message
 parameter

 Return value
 true or false
 *******************************************************************/
void adapter_co_send_AlarmMessage(int machineNO,
		int componentNO,
		int AlarmClass,
		int Alarm_year,
		int Alarm_month,
		int Alarm_day,
		int Alarm_hour,
		int Alarm_minute,
		int Alarm_second,
		int ElementType,
		int ElementID,
		int AlarmID,
		int AlarmClose,
		int Parameter)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO,INSIDE,
				sizeof(AppHeadIn) + sizeof(COAlarmMessage), PROTOCOL_MULTICAST);
	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 50, sizeof(COAlarmMessage));

	COAlarmMessage coAlarmMessage;
	memset(&coAlarmMessage,0,sizeof(COAlarmMessage));

	coAlarmMessage.ElementType=ElementType;
	coAlarmMessage.Element_ID1=short_to_byte(ElementID,1);
	coAlarmMessage.Element_ID2=short_to_byte(ElementID,2);
	coAlarmMessage.Alarm_minute=Alarm_minute;
	coAlarmMessage.Alarm_hour=Alarm_hour;
	coAlarmMessage.Alarm_day=Alarm_day;
	coAlarmMessage.Alarm_month=Alarm_month;
	coAlarmMessage.Alarm_year=Alarm_year;
	coAlarmMessage.AlarmID_1=short_to_byte(AlarmID,1);
	coAlarmMessage.AlarmID_2=short_to_byte(AlarmID,2);
	coAlarmMessage.AlarmClose=AlarmClose;
	coAlarmMessage.Parameter_1=int_to_byte(Parameter,1);
	coAlarmMessage.Parameter_2=int_to_byte(Parameter,2);
	coAlarmMessage.Parameter_3=int_to_byte(Parameter,3);
	coAlarmMessage.Parameter_4=int_to_byte(Parameter,4);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
		memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
		memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &coAlarmMessage,
				sizeof(COAlarmMessage));

		adapter_writen(app2netfd, szbuf,
				sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(COAlarmMessage));
}


//205
void adapter_co_send_CASToHMIMessage(
		int machineNO,
		int componentNO,
		int type)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(CASToHMI), PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 205, sizeof(CASToHMI));
	CASToHMI casTohmi;
	memset(&casTohmi, 0, sizeof(CASToHMI));
	casTohmi.type = type;
	casTohmi.reverse = 0;


	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &casTohmi,
			sizeof(CASToHMI));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(CASToHMI));
}

void adapter_atr_send_load_timetable_act(
		int machineNO,
		int componentNO,
		unsigned short timetable_innernumber,
		unsigned short errornumber,
		unsigned short success_or_no)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO,INSIDE,
			sizeof(AppHeadIn) + sizeof(LoadTimeTableAct), PROTOCOL_MULTICAST);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 254, sizeof(LoadTimeTableAct)); //????????????354

	LoadTimeTableAct loadTimeTableAct;
	memset(&loadTimeTableAct, 0, sizeof(LoadTimeTableAct));
	loadTimeTableAct.timetable_innernumber = timetable_innernumber;
	loadTimeTableAct.errornumber = errornumber;
	loadTimeTableAct.success_or_no = success_or_no;
	//loadTimeTableAct.success_or_no_low = short_to_byte(success_or_no, 1);
	//loadTimeTableAct.success_or_no_high = short_to_byte(success_or_no, 2);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &loadTimeTableAct,
			sizeof(LoadTimeTableAct));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(LoadTimeTableAct));
}

void adapter_atr_dwell_trip_time(
		int machineNO,
		int componentNO,
		int ConsoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
	    unsigned short elementType,
	    unsigned short TWCIndex,
	    unsigned short nextPTID,
	    unsigned short runClass)
{
	ILOG("adapter_atr_dwell_trip_time:elementType=%d,TWCIndex=%d,nextPTID=%d,runClass=%d",elementType,TWCIndex,nextPTID,runClass);
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(AppHeadOut) + sizeof(DwellTripTime), PROTOCOL_TCP);

	AppHeadOut appHeadOut;
	memset(&appHeadOut, 0, sizeof(AppHeadOut));
	appHeadOut.pdi = 1;
	appHeadOut.datatype_low = short_to_byte(512, 1);
	appHeadOut.datatype_high = short_to_byte(512,2);
	appHeadOut.datalen = sizeof(AppHeadOut) + sizeof(DwellTripTime);

	DwellTripTime dwellTripTime;
	memset(&dwellTripTime, 0, sizeof(DwellTripTime));
	dwellTripTime.consoleNO = extract(ConsoleNO,1);

	dwellTripTime.taskNO = extract(disposalNO,1);
	dwellTripTime.funcNO_low = short_to_byte(functionNO,1);
	dwellTripTime.funcNO_high = short_to_byte(functionNO,2);
	dwellTripTime.elementType = elementType;
	dwellTripTime.TWCIndex_low = short_to_byte(TWCIndex,1);
	dwellTripTime.TWCIndex_high = short_to_byte(TWCIndex,2);

	int nextPTID_ge,nextPTID_shi,nextPTID_bai;
	nextPTID_bai = nextPTID/100;
	nextPTID_shi = (nextPTID - nextPTID_bai*100)/10;
	nextPTID_ge = nextPTID - nextPTID_bai*100 - nextPTID_shi*10;

	dwellTripTime.next_platform_index_high = (nextPTID - nextPTID_ge)/10;

	if (setBitsOfByte(&dwellTripTime.runClass_and_next_platform_index_low, 0, 1, runClass) != 0)
	return;
	if (setBitsOfByte(&dwellTripTime.runClass_and_next_platform_index_low, 4, 4, nextPTID_ge) != 0)
	return;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadOut, sizeof(AppHeadOut));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadOut), &dwellTripTime,
			sizeof(DwellTripTime));

	if(PL_ACTIVE == 1)
	{
		SetAtsCommandDatagram(interlockZone,(unsigned char *)(szbuf + sizeof(NetHead)),sizeof(AppHeadOut) + sizeof(DwellTripTime),AUTO_COMMAND);
		return;
	}
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
	}
	else
	{
		int iret = adapter_writen(app2netfd,szbuf,sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(DwellTripTime));
		if  (0 < iret)
		{
			//DLOG("write %d bytes to fifo success!\n",iret);
		}
		else
		{
			DLOG("write fifo err!\n");
		}
	}
}

void adapter_atr_dwell_trip_time_ForCBTC(
		int machineNO,
		int componentNO,
		int ConsoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
	    unsigned short elementType,
	    unsigned short trainIndex,
	    unsigned short nextPTID,
	    unsigned short PTID,
	    unsigned short dwelltime,
	    unsigned short traveltime,
	    unsigned short nextStopPtid,
	    unsigned short askipAtNextPTID
	    )
{
	//added by lfc 20121229
	ILOG("adapter_atr_dwell_CBTC_trip_time:elementType=%d,TrainIndex=%d,PTID=%d,nextPTID=%d",elementType,trainIndex,PTID,nextPTID);
		unsigned char szbuf[NET_PACKET_LEN] = { 0 };
		NetHead stNetHead;
		memset(&stNetHead, 0, sizeof(NetHead));
		adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
				sizeof(AppHeadOut) + sizeof(DwellTripTime_CBTC), PROTOCOL_TCP);

		AppHeadOut appHeadOut;
		memset(&appHeadOut, 0, sizeof(AppHeadOut));
		appHeadOut.pdi = 1;
		appHeadOut.datatype_low = short_to_byte(512, 1);
		appHeadOut.datatype_high = short_to_byte(512,2);
		appHeadOut.datalen = sizeof(AppHeadOut) + sizeof(DwellTripTime_CBTC);

		DwellTripTime_CBTC dwellTripTime;
		memset(&dwellTripTime, 0, sizeof(DwellTripTime_CBTC));
		dwellTripTime.consoleNO = extract(ConsoleNO,1);

		dwellTripTime.taskNO = extract(disposalNO,1);
		dwellTripTime.funcNO_low = short_to_byte(functionNO,1);
		dwellTripTime.funcNO_high = short_to_byte(functionNO,2);
		dwellTripTime.elementType = elementType;
//		dwellTripTime.TWCIndex_low = short_to_byte(TWCIndex,1);
//		dwellTripTime.TWCIndex_high = short_to_byte(TWCIndex,2);
		dwellTripTime.trainIndex_low=short_to_byte(trainIndex,1);
		dwellTripTime.trainIndex_high=short_to_byte(trainIndex,2);
		dwellTripTime.currentPTID_low=short_to_byte(PTID,1);
		dwellTripTime.currentPTID_hign=short_to_byte(PTID,2);
		dwellTripTime.nextPTID_low=short_to_byte(nextPTID,1);
		dwellTripTime.nextPTID_high=short_to_byte(nextPTID,2);
		dwellTripTime.nextOSP=0;
		dwellTripTime.dwellTime_low=short_to_byte(dwelltime,1);
		dwellTripTime.dwellTime_high=short_to_byte(dwelltime,2);
		dwellTripTime.travelTime_low=short_to_byte(traveltime,1);
		dwellTripTime.travelTime_high=short_to_byte(traveltime,2);
		dwellTripTime.nextStopPTID_low=short_to_byte(nextStopPtid,1);
		dwellTripTime.nextStopPTID_hign=short_to_byte(nextStopPtid,2);
		dwellTripTime.aSkipAtNextPTID=askipAtNextPTID;


		memcpy(szbuf, &stNetHead, sizeof(NetHead));
		memcpy(szbuf + sizeof(NetHead), &appHeadOut, sizeof(AppHeadOut));
		memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadOut), &dwellTripTime,
				sizeof(DwellTripTime_CBTC));

		adapter_writen(app2netfd, szbuf,
				sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(DwellTripTime_CBTC));

}


void adapter_atr_change_tip_or_reversal(
		int machineNO,
		int componentNO,
		int ConsoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
		unsigned short elementType,
		unsigned short PTID,
		unsigned short parameter)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO ,OUTSIDE,
			sizeof(AppHeadOut) + sizeof(ChangeOrReversal), PROTOCOL_TCP);

	AppHeadOut appHeadOut;
	memset(&appHeadOut, 0, sizeof(AppHeadOut));
	appHeadOut.pdi = 1;
	appHeadOut.datatype_low = short_to_byte(512, 1);
	appHeadOut.datatype_high = short_to_byte(512,2);
	appHeadOut.datalen = sizeof(AppHeadOut) + sizeof(ChangeOrReversal);

	ChangeOrReversal changeOrReversal;
	changeOrReversal.consoleNO = extract(ConsoleNO,1);
	changeOrReversal.taskNO = extract(disposalNO,1);
	changeOrReversal.funcNO_low = short_to_byte(functionNO,1);
	changeOrReversal.funcNO_high = short_to_byte(functionNO,2);
	changeOrReversal.elementType = extract(elementType,1);
	changeOrReversal.platform_index_low = short_to_byte(PTID,1);
	changeOrReversal.platform_index_high = short_to_byte(PTID,2);
	changeOrReversal.parameter = extract(parameter,1);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadOut, sizeof(AppHeadOut));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadOut), &changeOrReversal,
			sizeof(ChangeOrReversal));

	if(PL_ACTIVE == 1)
	{
		SetAtsCommandDatagram(interlockZone,(unsigned char *)(szbuf + sizeof(NetHead)),sizeof(AppHeadOut) + sizeof(ChangeOrReversal),AUTO_COMMAND);
		return;
	}
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
	}
	else
	{
		int iret = adapter_writen(app2netfd,szbuf,sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(ChangeOrReversal));
		if  (0 < iret)
		{
			//DLOG("write %d bytes to fifo success!\n",iret);
		}
		else
		{
			DLOG("write fifo err!\n");
		}
	}

}

void adapter_atr_autoskip_or_cancelskip(
		int machineNO,
		int componentNO,
		int ConsoleNO,
		int interlockZone,
		int disposalNO,
		int functionNO,
		unsigned short elementType,
		unsigned short PTID,
		unsigned short skipOrCancel)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(AppHeadOut) + sizeof(AutoOrCancelSkip), PROTOCOL_TCP);

	AppHeadOut appHeadOut;
	memset(&appHeadOut, 0, sizeof(AppHeadOut));
	appHeadOut.pdi = 1;
	appHeadOut.datatype_low = short_to_byte(512, 1);
	appHeadOut.datatype_high = short_to_byte(512,2);
	appHeadOut.datalen = sizeof(AppHeadOut) + sizeof(AutoOrCancelSkip);

	AutoOrCancelSkip autoOrCancelSkip;
	autoOrCancelSkip.consoleNO = extract(ConsoleNO,1);
	autoOrCancelSkip.taskNO = extract(disposalNO,1);
	autoOrCancelSkip.funcNO_low = short_to_byte(functionNO,1);
	autoOrCancelSkip.funcNO_high = short_to_byte(functionNO,2);
	autoOrCancelSkip.elementType = extract(elementType,1);
	autoOrCancelSkip.platform_index_low = short_to_byte(PTID,1);
	autoOrCancelSkip.platform_index_high = short_to_byte(PTID,2);
	autoOrCancelSkip.skipOrCancel = extract(skipOrCancel,1);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadOut, sizeof(AppHeadOut));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadOut), &autoOrCancelSkip,
			sizeof(AutoOrCancelSkip));

	if(PL_ACTIVE == 1)
	{
		SetAtsCommandDatagram(interlockZone,(unsigned char *)(szbuf + sizeof(NetHead)),sizeof(AppHeadOut) + sizeof(AutoOrCancelSkip),AUTO_COMMAND);
		return;
	}
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(AutoOrCancelSkip));
}


void  adapter_atr_send_AllStationDataStructToFep(
		int machineNO,
		int componentNO,
		char allStationDataStruct[])
{
	int station_num = allStationDataStruct[0];
	ILOG("station_num = %d", station_num);
	int stationdata_len = 44*station_num+1;
//	char *p = NULL;
//	p = allStationDataStruct;
//	int i =0;
//	for(i = 0;i< stationdata_len;i++)
//	{
//		ILOG("%x ",*((unsigned char *)p));
//		p++;
//	}
//	ILOG("i = %d", i);

	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + stationdata_len, PROTOCOL_TCP);

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 602, stationdata_len);

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));

	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			allStationDataStruct, stationdata_len );

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + stationdata_len);
}


void adapter_atr_send_DTIStopTime(
		int machineNO,
		int componentNO,
		int innerElementType,
		int PTID,
		int nextPTID,
		int DTI_Index,
		unsigned short StopTime_Seconds,
		unsigned short Date_Hour,
		unsigned short Date_Minute,
		unsigned short Date_Second,
		unsigned short Is_CountDown,
		unsigned short Light_or_Dark,
		unsigned short CountDown_ModeValue)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	if(machineNO >= 190)
		adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + sizeof(DTIStopTime), PROTOCOL_MULTICAST);
	else
		adapter_net_head_package(&stNetHead, machineNO, INSIDE,
					sizeof(AppHeadIn) + sizeof(DTIStopTime), PROTOCOL_TCP);
	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 620, sizeof(DTIStopTime));//620

	DTIStopTime dtiStopTime;
	memset(&dtiStopTime, 0, sizeof(DTIStopTime));
	dtiStopTime.innerElementType = innerElementType;
	dtiStopTime.PTID_low = short_to_byte(PTID, 1);
	dtiStopTime.PTID_high = short_to_byte(PTID, 2);
	dtiStopTime.nextPTID_low = short_to_byte(nextPTID, 1);
	dtiStopTime.nextPTID_high = short_to_byte(nextPTID, 2);
	dtiStopTime.DTI_Index_low = short_to_byte(DTI_Index, 1);
	dtiStopTime.DTI_Index_high = short_to_byte(DTI_Index, 2);
	dtiStopTime.StopTime_Seconds_low = short_to_byte(StopTime_Seconds, 1);
	dtiStopTime.StopTime_Seconds_high = short_to_byte(StopTime_Seconds, 2);
	dtiStopTime.Date_Hour = Date_Hour;
	dtiStopTime.Date_Minute = Date_Minute;
	dtiStopTime.Date_Second = Date_Second;
	if (setBitsOfByte(&dtiStopTime.CountDown_Info, 0, 2, Is_CountDown) != 0)
		return;
	if (setBitsOfByte(&dtiStopTime.CountDown_Info, 2, 2, Light_or_Dark) != 0)
		return;
	if (setBitsOfByte(&dtiStopTime.CountDown_Info, 4, 4, CountDown_ModeValue) != 0)
		return;

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn), &dtiStopTime,
			sizeof(DTIStopTime));

	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + sizeof(DTIStopTime));
}









// interface for ars as follows.............................................

void adapter_ars_auto_route_send(
		int machineNO,
		int componentNO,
		int operator_console_number,
		int interlockZone,
		int transaction_number,
		int function_number,
 		int element_type,
		int element_index)
{
	unsigned char buf[NET_PACKET_LEN] = {0};		/** initialize buf for package */
	NetHead nethead;
	memset(&nethead,0,sizeof(NetHead));	/** initialize nethead */
	/**	source : CAS
	 * 	destination :SSI
	 *	data type:outside
	 *	type of the protocol:TCP
	 *	length:11
	 **/
	adapter_net_head_package(&nethead, machineNO, OUTSIDE, sizeof(AppHeadOut) + sizeof(OCT), PROTOCOL_TCP);
	AppHeadOut apphead;
	memset(&apphead, 0, sizeof(AppHeadOut));	/** initialize apphead */
	apphead.pdi = 1;					/** process data interface:1 */
	apphead.datatype_low = extract(512, 1);				/** type of the data:512 */
	apphead.datatype_high = extract(512, 2);
	apphead.datalen = 11;				/** length of the data */
	OCT oct;							/** operator control telegram */
	memset(&oct, 0, sizeof(OCT));		/** initialize oct */
	oct.operator_console_number = extract(operator_console_number,1);
	oct.transaction_number = extract(transaction_number,1);
	oct.function_number_low = extract(function_number,1);
	oct.function_number_high = extract(function_number,2);
	oct.element_type = extract(element_type,1);
	oct.element_index_low = extract(element_index,1);
	oct.element_index_high = extract(element_index,2);

	memcpy(buf, &nethead, sizeof(NetHead));
	memcpy(buf + sizeof(NetHead), &apphead, sizeof(AppHeadOut));
	memcpy(buf + sizeof(NetHead) + sizeof(AppHeadOut), &oct, sizeof(OCT));

	if(PL_ACTIVE == 1)
	{
		DLOG("adapter_ars_auto_route_send datalenght=%d",sizeof(AppHeadOut) + sizeof(OCT));
		SetAtsCommandDatagram(interlockZone,(unsigned char *)(buf + sizeof(NetHead)),sizeof(AppHeadOut) + sizeof(OCT),AUTO_COMMAND);
		return;
	}
	if(LOCAL_OR_CENTER == LOCAL_TRC)
	{
	}
	else
	{
		int iret = adapter_writen(app2netfd,buf,sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(OCT));
		if  (0 < iret)
		{
			//DLOG("write %d bytes to fifo success!\n",iret);
		}
		else
		{
			DLOG("write fifo err!\n");
		}
	}
}



// interface for load request as follows.............................................
void adapter_co_send_loadRequest(int machineNO,
							int componentNO,
						    int status_data,
						    int fault_indications,
							int reminder_collars,
 							int time)
{
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	unsigned char body = 0;
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, OUTSIDE,
			sizeof(AppHeadOut) + sizeof(unsigned char), PROTOCOL_TCP);

	AppHeadOut appHeadOut;
	memset(&appHeadOut, 0, sizeof(AppHeadOut));
	appHeadOut.pdi = 1;
	appHeadOut.datatype_low = short_to_byte(518, 1);//????????????518 ??????????????????
	appHeadOut.datatype_high = short_to_byte(518,2);
	appHeadOut.datalen = 5;

	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 518, sizeof(unsigned char)); //????????????518 ??????????????????

	if(0 != status_data)
	{
		body = body | 0x01;
	}
	if(0 != fault_indications)
	{
		body = body | 0x02;
	}
	if(0 != reminder_collars)
	{
		body = body | 0x04;
	}
	if(0 != time)
	{
		body = body | 0x08;
	}

	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadOut, sizeof(AppHeadOut));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadOut), &body,
				sizeof(unsigned char));
   if(LOCAL_OR_CENTER == LOCAL_TRC)
   {
		adapter_writen(app2TSnetfd, szbuf,
				sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(unsigned char));
		ILOG("Send load rqs from TRC");
   }else if(LOCAL_OR_CENTER == CENTER_CAS)
   {
		adapter_writen(app2netfd, szbuf,
				sizeof(NetHead) + sizeof(AppHeadOut) + sizeof(unsigned char));
		ILOG("Send load rqs from CAS");
   }

}
/*******************************************************************
 Function name		adapter_tmt_send_SynAllTrainDataStruct
 description	 	all train data sync
 parameter

 Return value
 true or false
 *******************************************************************/
void  adapter_tmt_send_SynAllTrainDataStruct(
		int machineNO,
		int componentNO,
		char allTrainDataStruct[])
{
	ILOG("Be about to send alltrainID=%d,",componentNO);
	int train_num = allTrainDataStruct[0];
	int traindata_len = ADB_TRAININFO_UNIT_LEN*train_num+1;
	unsigned char szbuf[NET_PACKET_LEN] = { 0 };
	NetHead stNetHead;
	memset(&stNetHead, 0, sizeof(NetHead));
	adapter_net_head_package(&stNetHead, machineNO, INSIDE,
			sizeof(AppHeadIn) + traindata_len, PROTOCOL_TCP);
	AppHeadIn appHeadIn;
	memset(&appHeadIn, 0, sizeof(AppHeadIn));
	adapter_app_head_package(&appHeadIn, 651, traindata_len);
	memcpy(szbuf, &stNetHead, sizeof(NetHead));
	memcpy(szbuf + sizeof(NetHead), &appHeadIn, sizeof(AppHeadIn));
	memcpy(szbuf + sizeof(NetHead) + sizeof(AppHeadIn),
			allTrainDataStruct, traindata_len );
	adapter_writen(app2netfd, szbuf,
			sizeof(NetHead) + sizeof(AppHeadIn) + traindata_len);
}

