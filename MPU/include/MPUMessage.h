/*******************************************************************
 filename      ats_co_message.h
 author        root
 created date  2012-1-9
 description   CO锟斤拷锟斤拷锟斤拷IO锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#ifndef ATS_CO_MESSAGE_HH_
#define ATS_CO_MESSAGE_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <sqlite3.h>
#include "MPUConvert.h"

#include "public.h"
#include "../../log.h"
//#include "../../CO/ats_co_sql.h"

#ifdef  __cplusplus
extern "C"
  {
#endif

#define   ATS_WORK_DB_FOR_REPLAY_NAME      "/dev/shm/ATSSQLREPLAY.db"
#define   ATS_IMPORTK_DB_FOR_REPLAY_NAME      "/home/ATS/CAS_fifthTest/mpu/MPUServer/ATSSQLREPLAY.db"

//*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�

enum ErrCode
{

  //section -1000 to 0 ,stand for system call error
  Error = -1, //default
  //section 0 to 1000 ,stand for database operation error
  //section 1000 to 2000 ,stand for network communication error
  //section up 5000 ,stand for error happened at user business logical layer
  ERROR_NET = 4000,
  ERROR_MESSAGE_TYPE = 5000,
};

//* 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
//#define PACKET_LEN          1400
//#define MSG_FIELD_LEN       32
#define MSG_HEADER_LEN      24
#define MAX_NAME_LEN        255


//* sqlite3 definition
#define MAX_SQL_STR_LEN     1024
#define TIME_SIZE 20
#define DB_NAME                 "/dev/shm/ATSSQL.db"
#define DUMPSH_PATH             "/usr/lib/dump.sh"
#define SYNC_ORCL               "/usr/lib/sync_orcl.sh"
#define TABLE_NEED_CLEAR_ONE    "ATTForSqlite"
#define TABLE_NEED_CLEAR_SEC    "AlarmList"
#define PLDB_NAME                 "/dev/shm/KyosanPl.db"

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
enum MsgType
{
  STATUS_TYPE = 400,
  EVENT_TYPE = 434,
  COMMAND_TYPE = 500
};

enum ElementType
{
  LOGICALTRACK = 207,
  TRAIN = 151,
  PLATFORM = 26,
  SWITCHS = 200,
  SIGNAL = 201,
  AXLECOUNTER = 213
};
//
//typedef unsigned char u8;
//typedef short u16;
//typedef int u32;
//typedef long long int u64;
//typedef unsigned long long int NetTime;

//typedef struct _message_header
//{
//  NetTime timestamp; /** the time stamp of sending the message */
//  u32 seqno; /** sequence no */
//  u8 srcno; /** source IP no */
//  u8 destno; /** destination IP no */
//  u8 srcmodule; /** source module no */
//  u8 destmodule; /** destination module no */
//  u16 datatype; /** type of the data */
//  u8 protocoltype; /** type of the protocol */
//  u8 mainorbak; /** server flag */
//  u16 remain;
//  u16 datalen; /** length of the data */
//} message_header_t;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟� <锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<锟斤拷=value><锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷=type><锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷=position><锟斤拷锟斤拷锟斤拷锟斤拷=bits>
//typedef struct _ats_co_MsgHead
//{
//  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
//  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=default(int)><bytes=1-2><bits=default(all)>
//  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
//  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
//  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=6><bits=>
//} ats_co_MsgHead;

typedef struct _ats_co_FepStatusMsg
{
	int fepID; //consoleID:70/71/72......
	int fepModule; //201/202/203/204
	int fepStatus;//0 OR 1
}ats_co_FepStatusMsg;
//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
typedef struct _ats_co_TrackMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=207><type=int><bytes=6><bits=>
  int trackid; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=7-8><bits=>
  //int direction; //release by lfc 20121211
  int lockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=6>
  int freeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=0-1>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        =2>
  //int status; //<锟斤拷锟斤拷锟斤拷锟斤拷锟紸TPVacancy锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 3锟斤拷锟斤拷锟斤拷ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 4锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=22><bits=3-4-5>

  //below is added by zs 2012-3-15
   int atpAvailable; //<type=int><bytes=10><bits=0>;
  //int atpVacancy;
   //int physicalFree;  //<type=int><bytes=12><bits=5>;
   int leftClaim;
   int rightClaim;
} ats_co_TrackMsg;
typedef struct _ats_co_logicalTrackMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=207><type=int><bytes=6><bits=>
  int trackid; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=7-8><bits=>
  int direction; //release by lfc 20121211
  int lockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=6>
  int freeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=0-1>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        =2>
  //int status; //<锟斤拷锟斤拷锟斤拷锟斤拷锟紸TPVacancy锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 3锟斤拷锟斤拷锟斤拷ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 4锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=22><bits=3-4-5>

  //below is added by zs 2012-3-15
   int atpAvailable; //<type=int><bytes=10><bits=0>;
   int atpVacancy;  ////release by lfc 20121211
   //int physicalFree;  //<type=int><bytes=12><bits=5>;
   int TVDFailure;
} ats_co_logicalTrackMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
typedef struct _ats_co_TrainStatusMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=151><type=int><bytes=6><bits=>
  int trainIndex; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=7-8><bits=>
  int trainDoorStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=12><bits=0-1>
  int ebIsApplied; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=13><bits=0>
  int ebReason; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=14><bits=0-1-2>
  int controlLevel; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=15><bits=0-1>
  int driveMode; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=16><bits=0-1-2>
  int obcuFailure; //<OBCU锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=22><bits=1>
  int destination; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=36-37><bits=>
  int service; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=38-39><bits=>
  int serial; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=40><bits=>
  int crew; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=41-42><bits=>
  int line; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=43><bits=>
  int car;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷1><value=><type=int><bytes=44-45><bits=>

  int CommuEstablished;//
  int DstNumber;//
  int driver;//
  int car1;//

} ats_co_TrainStatusMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
typedef struct _ats_co_TrainEventMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=434><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=151><type=int><bytes=6><bits=>
  int trainIndex; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  DateTime eventTime; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=tm><bytes=9-13><bits=>
  int eventType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><<type=int><bytes=14-15><bits=>
  int eventCause; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=16-17><bits=>
  int PTID; //
} ats_co_TrainEventMsg;

typedef struct _ats_co_TrainEventTWCMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=208><type=int><bytes=6><bits=>
  int ATPbreakOn; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><<type=int><bytes=14-15><bits=>
  int TWCIndex; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  int PTID; //
  int destination; //
  int service; //
  int serial; //
  int trainID;
  int eventType;
  int carID;//<bytes=20-21>
  int crewID;//<bytes=18-19>
  int trainIndex;
  DateTime eventTime; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=tm><bytes=9-13><bits=>
} ats_co_TrainEventTWCMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
typedef struct _ats_co_PlatformMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=26><type=int><bytes=6><bits=>
  int platform; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  int enforceSkipOrHold; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=><type=int><bytes=20><bits=4-5> modified to <bits=4-5-6> by zs 2012-10-16
  int autoSkip; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=21><bits=7>
} ats_co_PlatformMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
typedef struct _ats_co_SwitchMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=200><type=int><bytes=6><bits=>
  int switchId; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  int positionFlag; //converted inner value
  int claimStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<type=int><bytes=11><bits=3-4>
  int blockAgainstThrowing; //<锟斤拷锟斤拷锟斤拷锟斤拷锟�锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷><type=int><bytes=12><bits=2>
  int lockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=12><bits=4>
  int freeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=7>
  //below is for counting positionFlag
  int pointTrailed; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=0>;
  int supervision; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=12><bits=1>;
  int position; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=6>;
  int kickoffFailure;

  //below is added by zs 2012-3-15
  int atpAvailable; //<type=int><bytes=10><bits=0>;
  int atpVacancy;  //<type=int><bytes=22><bits=3>;
  int physicalFree;  //<type=int><bytes=12><bits=5>;

  //added by lfc 20121211
  int overlapClaiming;
  int pointDirection;
  int TVDFailure;
  int routeClaim;


} ats_co_SwitchMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
typedef struct _ats_co_SignalMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=201><type=int><bytes=6><bits=>
  int signalId; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  int signalStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=11><bits=4-5-6>
  int signalAutoStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷 1ARS 2ARC 3锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷FLT><type=int><bytes=12><bits=2-3>
  int lockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0=锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1=锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=12><bits=1>
  int signalFault; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=13><bits=4>
} ats_co_SignalMsg;

typedef struct _ats_co_DirectionMsg
{
  int PDI;
  int msgType;
  int length;
  int counter;
  int elementType;
  int directionId;
  int leftDirection;
  int rightDirection;
} ats_co_DirectionMsg;
// PTI msg struct
typedef struct _ats_co_PTIMsg
{
  int PDI;
  int msgType;
  int length;
  int counter;
  int elementType;
  int PTIindex;
  int operatingmode;
  int I1available;
  int fault;
  int commf;
  int ATPavailable;
  int failureinfo;
  int elementavailabe;
  int startframe;
  int channelnum;
  int freshness;
  int crewnumH;
  int crewnumT;
  int crewnumS;
  int tripnumThousand;
  int tripnumH;
  int tripnumT;
  int tripnumS;
  int destnum;
  int trainstatus;
  int direction;
  int activeMSB;
  int activeLSB;


} ats_co_PTIMsg;


//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
typedef struct _ats_co_AxleCounterMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=213><type=int><bytes=6><bits=>
  int axleCounterId; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=7-8><bits=>
  int axleCounterFault; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0 OK,1 ARB锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷, 2 NRB锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=20><bits=0-1>
  int freeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷 2锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><type=int><bytes=11><bits=0-1>
} ats_co_AxleCounterMsg;

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷TMT锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
typedef struct _ats_co_DataForTMT
{
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=207/200>
  int elementid; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=>
  int trackdirection; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int tracklockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int trackfreeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int trackstatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟紸TPVacancy锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 3锟斤拷锟斤拷锟斤拷ATP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 4锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int switchpositionFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷 2锟斤拷锟斤拷锟斤拷锟斤拷 3锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int switchclaimStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 2锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
  int switchblockAgainstThrowing; //<锟斤拷锟斤拷锟斤拷锟斤拷锟�锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷>
  int switchlockFlag; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int switchfreeStatus; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�<value=0锟斤拷锟斤拷锟斤拷锟斤拷 1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷>
  int finish;
} ats_co_DataForTMT;

//to save configure information ,this is a brief structure  .
//though name of function is not using here ,but we also save them.
typedef struct _ConversationConfig
{
  int message_type;
  int element_type;
  int index; //here we use index to identify different of message type,so it can not be set the same
  char *bufferConverToMsgFuncName;
  char *msgConverToBufferFuncName;
  char *ats_co_setDBFuncName;
  char *ats_co_getDBFuncName;
} sConversationConfig;

typedef struct _ats_co_Crossing
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=203><type=int><bytes=6><bits=>
  int crossingIndex; //
  //modified by lfc 20121211
  int freeStatus;
  int overlapClaim;
  int routeClaim;
  int failureIndication;
  int lockAgainstPassing;
  int blockAgainstThrowing;
  int position;
  int direction;
  int TVDFailure;
  int ATPVacancy;
  int atpAvailable;

} ats_co_CrossingMsg;

typedef struct _ats_co_AreaStatusMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
  int areaStatusIndex; //<value=><type=int><bytes=7-8><bits=>
  int operatingMode; //<value=><type=int><bytes=9><bits=0-3>
  int fault; //<value=><type=int><bytes=9><bits=4>
  int CommF; //<value=><type=int><bytes=9><bits=5>
  int SDA; //<value=><type=int><bytes=9><bits=7>
  int ATPflag; //<value=><type=int><bytes=10><bits=0>
  int failureInformation; //<value=><type=int><bytes=10><bits=1-6>
  int interLocking; //<value=><type=int><bytes=10><bits=7>
  int areaBlockFlag; //<value=><type=int><bytes=11><bits=0>
  int reduceNoiseFlag; //<value=><type=int><bytes=11><bits=1>
  int allowNearFlag; //<value=><type=int><bytes=11><bits=2>
} ats_co_AreaStatusMsg;

typedef struct _ats_co_TrainPositionReport
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
  int trainIndex; //<value=><type=int><bytes=7-8><bits=>
  //int operatingMode; //<value=><type=int><bytes=9><bits=0-3>
  int fault; //<value=><type=int><bytes=9><bits=4>
  int CommF; //<value=><type=int><bytes=9><bits=5>
  int SDA; //<value=><type=int><bytes=9><bits=7>
  int ATPflag; //<value=><type=int><bytes=10><bits=0>
  int failureInformation; //<value=><type=int><bytes=10><bits=1-6>
  int interLocking; //<value=><type=int><bytes=10><bits=7>

  int LoclsSecured; //edit by quj <value=><type=int><bytes=11><bits=0>
  int operatingMode;
  int locSecurityFlag;
  int trainPositionFlag; //<value=><type=int><bytes=11><bits=4>
  int locIntegrityIsOK; //<value=><type=int><bytes=14><bits=0>
  int headSegmentID; //<value=><type=int><bytes=15-16><bits=>
  int headOffset; //<value=><type=int><bytes=17-20><bits=>
  int trainDirection; //<value=><type=int><bytes=21><bits=0-1>
  int tailSegmentID; //<value=><type=int><bytes=22-23><bits=>
  int tailOffset; //<value=><type=int><bytes=24-27><bits=>
  int operationCtrLevel; //<value=><type=int><bytes=29><bits=0-1>
  int trainDriveMode; //<value=><type=int><bytes=30><bits=0-2>
  int trainSpeed; //<value=><type=int><bytes=34-35><bits=>
  //add by quj 20121213
  int OPRValidFlag;
  int FrontSegID;
  int FrontOffset;
  int ControlLevel;
  int OperationMode;
  int carSpeed;
  int FrontTravelDir;
  int TrainStandStill;


} ats_co_TrainPositionReport;

typedef struct _ats_co_RouteStatusMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
  int routeIndex; //<value=><type=int><bytes=7-8><bits=>
  int operatingMode; //<value=><type=int><bytes=9><bits=0-3>
  int fault; //<value=><type=int><bytes=9><bits=4>
  int CommF; //<value=><type=int><bytes=9><bits=5>
  int SDA; //<value=><type=int><bytes=9><bits=7>
  int AtpAvailable; //<value=><type=int><bytes=10><bits=0>
  int failureInformation; //<value=><type=int><bytes=10><bits=1-6>
  int interlockAvailable; //<value=><type=int><bytes=10><bits=7>
  int routeCtrLevel; //<value=><type=int><bytes=11><bits=0-1>
  int delayRouteRelease; //<value=><type=int><bytes=11><bits=2>
  int routeStatus; //<value=><type=int><bytes=11><bits=3>
  int AtpCtrLevel; //<value=><type=int><bytes=11><bits=4>
} ats_co_RouteStatusMsg;

//typedef struct _ats_co_WCUErrMsg
//{
//      int PDI;//<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
//      int msgType;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
//      int length ;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
//      int counter;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
//      int elementType;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
//      int WCUIndex;//<value=><type=int><bytes=7-8><bits=>
//      int minute;//<value=><type=int><bytes=9><bits=>
//      int hour;//<value=><type=int><bytes=10><bits=>
//      int day;//<value=><type=int><bytes=11><bits=>
//      int month;//<value=><type=int><bytes=12><bits=>
//      int year;//<value=><type=int><bytes=13><bits=>
//      int errCode;//<value=><type=int><bytes=14-15><bits=>
//      int moduleCode;//<value=><type=int><bytes=16-17><bits=>
//}ats_co_WCUErrMsg;
//
//typedef struct _ats_co_AxleOrTrackCircuit
//{
//      int PDI;//<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
//      int msgType;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
//      int length ;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
//      int counter;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
//      int elementType;//<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
//      int trainIndex;//<value=><type=int><bytes=7-8><bits=>
//      int minute;//<value=><type=int><bytes=9><bits=>
//      int hour;//<value=><type=int><bytes=10><bits=>
//      int day;//<value=><type=int><bytes=11><bits=>
//      int month;//<value=><type=int><bytes=12><bits=>
//      int year;//<value=><type=int><bytes=13><bits=>
//      int errCode;//<value=><type=int><bytes=14-15><bits=>
//      int moduleCode;//<value=><type=int><bytes=16-17><bits=>
//}ats_co_AxleOrTrackCircuit;

typedef struct _ats_co_WCUErrMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
  int index; //<value=><type=int><bytes=7-8><bits=>
  int minute; //<value=><type=int><bytes=9><bits=>
  int hour; //<value=><type=int><bytes=10><bits=>
  int day; //<value=><type=int><bytes=11><bits=>
  int month; //<value=><type=int><bytes=12><bits=>
  int year; //<value=><type=int><bytes=13><bits=>
  int errCode; //<value=><type=int><bytes=14-15><bits=>
  int isRecover; //<value=><type=int><bytes=16-17><bits=>
  int remark;
} ats_co_ErrIndicationMsg;

typedef struct _ats_co_TrainErrMsg
{
  int PDI; //<PDI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=1><type=default(int)><bytes=0><bits=default(all)>
  int msgType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=400><type=default(int)><bytes=1-2><bits=default(all)>
  int length; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=3><bits=>
  int counter; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=><type=int><bytes=4-5><bits=>
  int elementType; //<锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷><value=158><type=int><bytes=6><bits=>
  int trainIndex; //<value=><type=int><bytes=7-8><bits=>
  int minute; //<value=><type=int><bytes=9><bits=>
  int hour; //<value=><type=int><bytes=10><bits=>
  int day; //<value=><type=int><bytes=11><bits=>
  int month; //<value=><type=int><bytes=12><bits=>
  int year; //<value=><type=int><bytes=13><bits=>
  int errCode; //<value=><type=int><bytes=14-15><bits=>
  int moduleCode; //<value=><type=int><bytes=16-17><bits=>
} ats_co_TrainErrMsg;

//typedef struct _ats_co_NoModefyError
//{
//	int msgType;//<value=><type=default(int)><bytes=0-1><bits=default(all)>
//	int length;//<value=211><type=default(int)><bytes=2-5><bits=default(all)>
//	int transationNO;//<value=211><type=default(int)><bytes=6><bits=default(all)>
//	int ack;//<value=211><type=default(int)><bytes=7><bits=default(all)>
//}ats_co_NoModefyError;

typedef struct _ats_co_ALLMsgUpdate
{
	int msgType;//<value=><type=default(int)><bytes=0-1><bits=default(all)>
	int length;//<value=201><type=default(int)><bytes=2-5><bits=default(all)>
	int hmiNumber;
	int faultMsg;
	int trainMsg;
	int statusMsg;
	int diagMsg;
}ats_co_ALLMsgUpdate;

typedef struct _ats_co_TrainIDModify
{
	int msgType;//<value=><type=default(int)><bytes=0-1><bits=default(all)>
	int length;//<value=204><type=default(int)><bytes=2-5><bits=default(all)>
	int destination_ID;//<value=><type=default(int)><bytes=6-7><bits=default(all)>
	int service_ID;//<value=><type=default(int)><bytes=8-9><bits=default(all)>
	int serial;//<value=><type=default(int)><bytes=10><bits=default(all)>
	int modifyType;//<value=><type=default(int)><bytes=11><bits=default(all)>
	int processNO;//<value=><type=default(int)><bytes=12><bits=default(all)>
	int car_one;//<value=><type=default(int)><bytes=13-15><bits=default(all)>
	int car_two;//<value=><type=default(int)><bytes=16-18><bits=default(all)>
	int logicalSection;//<value=><type=default(int)><bytes=19-20><bits=default(all)>
	int oldlogicalSection;//<value=><type=default(int)><bytes=21-22><bits=default(all)>
	int olddestination_ID;//<value=><type=default(int)><bytes=23-24><bits=default(all)>
	int oldservice_ID;//<value=><type=default(int)><bytes=25-26><bits=default(all)>
	int oldserial;//<value=><type=default(int)><bytes=27><bits=default(all)>
	int trainIDFault;//<value=><type=default(int)><bytes=28><bits=default(all)>
	int trainIndex;//<value=><type=default(int)><bytes=31-32><bits=default(all)>
//	int atrStatus;//<value=><type=default(int)><bytes=33><bits=default(all)>
	int arsStatus;//<value=><type=default(int)><bytes=34><bits=default(all)>
	int machineNo;
}ats_co_TrainIDModify;

typedef struct _ats_co_RequireRunningMsg
{
	int msgType;
}ats_co_RequireRunningMsg;

typedef struct _ats_co_RequireModifyATRModeMsg
{
	int msgType; //256
	int length;
	int machineNO;
	int requireModifyType;
	int lineNo;
	int trainIndex;
	int trainNO;
	int mode;
	int para1;
	int para2;
	int destination;
	int serviceID;
	int serial;
	int car1;
	int car2;
}ats_co_RequireModifyATRModeMsg;

typedef struct _ats_co_LoadTimeTableMsg
{
	int msgType; //253
	int length;
	int timerableInnerID;
}ats_co_LoadTimeTableMsg;

typedef struct _ats_co_CFepRequireCASDataMsg
{
	int msgType; //608
	int length;
	int machineNO;
}ats_co_CFepRequireCASDataMsg;

typedef struct _ats_co_AlarmMsg
{
	int msgType; //50
	int length;
	int equipment_Type;
	int equipment_ID;
	int minute;
	int hour;
	int day;
	int month;
	int year;
	int alarm_ID;
	int isRecover;
	int remark;
}ats_co_AlarmMsg;


typedef struct _ats_co_LCfepNetStatusMsg
{
	int msgType; //60
	int length;
	int equipment_Type;
	char LCfepMsgData[4];
}ats_co_LCfepNetStatusMsg;


typedef struct _ats_co_CASNetStatusMsg
{
	int fepID; //consoleID:70/71/72......
	int fepModule; //201/202/203/204
	int fepStatus;//0 OR 1
}ats_co_CASNetStatusMsg;


typedef struct _ats_co_InterfaceTrackStatusMsg
{
	int msgType; //701
	int length;
	int trackid;
	int freestatus;
	int car1;
	int car2;
}ats_co_InterfaceTrackStatusMsg;

typedef struct _ats_co_HmiToATRManualRegulationValue
{
	int msgType; //260
	int length;
	int Type;
	int LineNO;
	int StationNum;
	int TrainIndex;
	int Destination;
	int ServiceID;
	int TripID;
	int car1;
	int car2;
//	char AllStationManualReguData[560];//define 40*14
	char *AllStationManualReguData;
}ats_co_HmiToATRManualRegulationValue;

typedef struct _ats_co_InterfaceTTPMsg
{
	int msgType; //351
	int length;
	int innerid;
	int versionno;
}ats_co_InterfaceTTPMsg;
//add by quj for TS fepstatus
typedef struct _ats_co_TsFepMsg
{
	  int PDI;
	  int msgType;
	  int length;
	  int counter;
	  int elementType;
	  int controlindex;
	  int controlrightauto;
	 // int rightDirection;
}ats_co_TsControlRightAutoMsg;
//add by quj for cas heart beat
typedef struct _ats_co_CASBeatMsg
{
	  int PDI;
	  int msgType;
	  int length;
	  int counter;
	  int elementType;
//	  int controlindex;
//	  int controlrightauto;
	 // int rightDirection;
}ats_co_CASBeatMsg;
typedef struct _ats_co_LoadOKmsg
{
	  int PDI;
	  int msgType;
	  int length;
	  int IndicationStatus;
	  int IndicationError;
	  int IndicationRing;
	  int IndicationCounter;

}ats_co_LoadOKmsg;
//650msg
typedef struct _ats_co_RecieveLoadOKMsg
{
	int msgType;
}ats_co_RecieveLoadOKMsg;


typedef struct _ats_co_RecieveTrainidSynMsg
{
	int msgType; //651
	int trainNum;
    int length;
	char *AllTrainData;
//	int carnumber;
//	int destno;
//	int servno;
//	int lineno;
//	int roadno;
//	int car1;
//	int car2;
//	int driverno;
//	int offtime;
//	int drive_mode;
//	int crewreal;
//	int headno;
//	int direction;
//	int trainno;
}ats_co_RecieveTrainidSynMsg;
//Since we use below structure to receive bytes received from pipe,
//we can get the size of received packet ,then analysis head of packet
//format, get message type and size to initial a message handler
typedef void
(*ats_co_bufferConvertToMsgFunc)(const BYTE *buf, void *msg);
typedef void
(*ats_co_msgConvertToBufferFunc)(const void *msg, BYTE *buf);
typedef void
(*ats_co_msgLogicalFunc)(sqlite3 *db, void *message);
typedef void
(*ats_co_setDBFunc)(sqlite3 *db, void *message);
typedef void
(*ats_co_getDBFunc)(sqlite3 *db, void *message);


typedef union   _MESSAGE_HEADER
{
	message_header_t header;
	//RM_header_t		rm_header;


}MESSAGE_HEADER;

typedef struct _ATS_CO_MSG
{
  //format operation
  message_header_t header;
  BYTE *format;
  int fixed_len;
  sConversationConfig config;

  //structure of message conform to index value
  void *message;

  //functions for message convertion
  ats_co_bufferConvertToMsgFunc bufferConvertToMsgFunc;
  ats_co_msgConvertToBufferFunc msgConvertToBufferFunc;

  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
  ats_co_msgLogicalFunc logicalMsgFunc;

  //functions for database accessing
  ats_co_setDBFunc setDBFunc;
  ats_co_getDBFunc getDBFunc;

} ATS_CO_MSG;

//According to message type and size ,we can implement buffer format
//and setting of member function .we also should parse fixed head of this
//message to get message type ,then to decide to use which processing map.
int
ats_co_initATSCOMessage(ATS_CO_MSG *handle, const BYTE *buf, size_t len);

//free member point of message
int
ats_co_freeATSCOMessage(ATS_CO_MSG *handle);

//log or print ATSCOMSG contents
int
ats_co_unpackATSCOMessage(ATS_CO_MSG *handle);
int
ats_co_refreshDBATSCOMessage(sqlite3 *db, ATS_CO_MSG *handle);
int
ats_co_parseNetHeader(ATS_CO_MSG *handle, const BYTE *buf);
int
ats_co_logATSCOMessage(ATS_CO_MSG *handle);

int
ats_co_MSG_compare(ATS_CO_MSG *source,ATS_CO_MSG *dest);

//implemented in ats_co_message.c
int
ats_co_BufferConvertToFepStatusMsg(const BYTE buf[] ,ats_co_FepStatusMsg *message);
int
ats_co_FepStatusMsgConvertToBuffer(ats_co_FepStatusMsg *message, BYTE buf[]);

//TrackMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToTrackMsg(const BYTE buf[], ats_co_TrackMsg *message);
int
ats_co_TrackMsgConvertToBuffer(ats_co_TrackMsg *message, BYTE buf[]);

//TrainStatusMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToTrainStatusMsg(const BYTE buf[],ats_co_TrainStatusMsg *message);
int
ats_co_TrainStatusMsgConvertToBuffer(ats_co_TrainStatusMsg *message, BYTE buf[]);

//TrainEventMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToTrainEventMsg(const BYTE buf[],ats_co_TrainEventMsg *message);
int
ats_co_TrainEventMsgConvertToBuffer(ats_co_TrainEventMsg *message, BYTE buf[]);

//PlatformMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToPlatformMsg(const BYTE buf[], ats_co_PlatformMsg *message);
int
ats_co_PlatformMsgConvertToBuffer(ats_co_PlatformMsg *message, BYTE buf[]);

//SwitchMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToSwitchMsg(const BYTE buf[], ats_co_SwitchMsg *message);
int
ats_co_SwitchMsgConvertToBuffer(ats_co_SwitchMsg *message, BYTE buf[]);

//SignalMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToSignalMsg(const BYTE buf[], ats_co_SignalMsg *message);
int
ats_co_SignalMsgConvertToBuffer(ats_co_SignalMsg *message, BYTE buf[]);

//AxleCounterMsg锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
int
ats_co_BufferConvertToAxleCounterMsg(const BYTE buf[],ats_co_AxleCounterMsg *message);
int
ats_co_AxleCounterMsgConvertToBuffer(ats_co_AxleCounterMsg *message, BYTE buf[]);

int
ats_co_BufferConvertToALLMsgUpdate(const BYTE buf[] ,ats_co_ALLMsgUpdate  *message);
int
ats_co_ALLMsgUpdateConvertToBuffer(ats_co_ALLMsgUpdate *message, BYTE buf[]);

int
ats_co_BufferConvertToTrainIDModify(const BYTE buf[] ,ats_co_TrainIDModify  *message);
int
ats_co_TrainIDModifyConvertToBuffer(ats_co_TrainIDModify *message, BYTE buf[]);

int
ats_co_BufferConvertToTrainPositionReport(const BYTE buf[], ats_co_TrainPositionReport *message);
int
ats_co_TrainPositionReportConvertToBuffer(ats_co_TrainPositionReport *message, BYTE buf[]);

int
ats_co_BufferConvertToTrainEventTWCMsg(const BYTE buf[], ats_co_TrainEventTWCMsg *message);
int
ats_co_TrainEventTWCMsgConvertToBuffer(ats_co_TrainEventTWCMsg *message, BYTE buf[]);

int
ats_co_RouteStatusMsgConvertToBuffer(ats_co_RouteStatusMsg *message, BYTE buf[]);
int
ats_co_BufferConvertToRouteStatusMsg(const BYTE buf[], ats_co_RouteStatusMsg *message);
int
ats_co_BufferConvertToLogicalTrackMsg(const BYTE buf[], ats_co_logicalTrackMsg *message);
//
int
ats_co_BufferConvertToCrossingMsg(const BYTE buf[], ats_co_CrossingMsg *message);
int
ats_co_BufferConvertToAreaStatusMsg(const BYTE buf[], ats_co_AreaStatusMsg *message);
int
ats_co_BufferConvertToErrIndicationMsg(const BYTE buf[], ats_co_ErrIndicationMsg *message);

int
ats_co_BufferConvertToDirectionMsg(const BYTE buf[],ats_co_DirectionMsg *message);
int
ats_co_BufferConvertToPTIMsg(const BYTE buf[],ats_co_PTIMsg *message);
int ats_co_BufferConvertToRecieveTrainidSynMsg(const BYTE buf[],ats_co_RecieveTrainidSynMsg *message);
int
ats_co_BufferConvertToTimeTableLoad(const BYTE buf[] ,ats_co_TimeTableLoad  *message);
//int ats_co_BufferConvertToTrainErrMsg(const BYTE buf[] ,ats_co_TrainErrMsg  *message);

int ats_co_BufferConvertToRequireRunningMsg(const BYTE buf[] ,ats_co_RequireRunningMsg  *message);

int ats_co_BufferConvertToRequireModifyATRModeMsg(const BYTE buf[] ,ats_co_RequireModifyATRModeMsg  *message);

int ats_co_BufferConvertToLoadTimeTableMsg(const BYTE buf[] ,ats_co_LoadTimeTableMsg  *message);

int ats_co_BufferConvertToCFepRequireCASDataMsg(const BYTE buf[] ,ats_co_CFepRequireCASDataMsg  *message);

int ats_co_BufferConvertToAlarmMsg(const BYTE buf[] ,ats_co_AlarmMsg  *message);

int ats_co_BufferConvertToLCfepNetStatusMsg(const BYTE buf[] ,ats_co_LCfepNetStatusMsg  *message);

int ats_co_BufferConvertToCASNetStatusMsg(const BYTE buf[] ,ats_co_CASNetStatusMsg *message);

int ats_co_CASNetStatusMsgConvertToBuffer(ats_co_CASNetStatusMsg *message, BYTE buf[]);

int
ats_co_BufferConvertToInterfaceTrackStatusMsg(const BYTE buf[], ats_co_InterfaceTrackStatusMsg *message);











int
ats_co_FepStatusSetDB(sqlite3 *db, ats_co_FepStatusMsg *message);

int
ats_co_TrackMsgSetDB(sqlite3 *db, ats_co_TrackMsg *message);
int
ats_co_TrackMsgGetDB(sqlite3 *db, ats_co_TrackMsg *message);

int
ats_co_LogicalTrackMsgSetDB(sqlite3 *db, ats_co_logicalTrackMsg *message);

int
ats_co_TrainStatusMsgSetDB(sqlite3 *db, ats_co_TrainStatusMsg *message);
int
ats_co_TrainStatusMsgGetDB(sqlite3 *db, ats_co_TrainStatusMsg *message);


int
ats_co_TrainEventMsgSetDB(sqlite3 *db, ats_co_TrainEventMsg *message);
int
ats_co_TrainEventMsgGetDB(sqlite3 *db, ats_co_TrainEventMsg *message);


int
ats_co_PlatformMsgSetDB(sqlite3 *db, ats_co_PlatformMsg *message);
int
ats_co_PlatformMsgGetDB(sqlite3 *db, ats_co_PlatformMsg *message);


int
ats_co_SwitchMsgSetDB(sqlite3 *db, ats_co_SwitchMsg *message);
int
ats_co_CrossingMsgSetDB(sqlite3 *db, ats_co_CrossingMsg *message);
int
ats_co_SwitchMsgGetDB(sqlite3 *db, ats_co_SwitchMsg *message);


int
ats_co_SignalMsgSetDB(sqlite3 *db, ats_co_SignalMsg *message);
int
ats_co_SignalMsgGetDB(sqlite3 *db, ats_co_SignalMsg *message);


int
ats_co_AxleCounterMsgSetDB(sqlite3 *db, ats_co_AxleCounterMsg *message);
int
ats_co_AxleCounterMsgGetDB(sqlite3 *db, ats_co_AxleCounterMsg *message);

int
ats_co_TrainPositionMsgSetDB(sqlite3 *db, ats_co_TrainPositionReport *message) ;
int
ats_co_TrainPositionMsgGetDB(sqlite3 *db, ats_co_TrainPositionReport *message);

int
ats_co_ALLMsgUpdateSetDB(sqlite3 *db, ats_co_ALLMsgUpdate *message);
int
ats_co_ALLMsgUpdateGetDB(sqlite3 *db, ats_co_ALLMsgUpdate *message);

int
ats_co_TrainIDModifySetDB(sqlite3 *db, ats_co_TrainIDModify *message);
int
ats_co_TrainIDModifyGetDB(sqlite3 *db, ats_co_TrainIDModify *message);

int
ats_co_TrainEventTWCMsgSetDB(sqlite3 *db, ats_co_TrainEventTWCMsg *message);
int
ats_co_TrainEventTWCMsgGetDB(sqlite3 *db, ats_co_TrainEventTWCMsg *message);

int
ats_co_RouteStatusMsgSetDB(sqlite3 *db, ats_co_RouteStatusMsg *message);
int
ats_co_RouteStatusMsgGetDB(sqlite3 *db, ats_co_RouteStatusMsg *message);

int
ats_co_RequireRunningMsgSetDB(sqlite3 *db, ats_co_RequireRunningMsg *message);

int
ats_co_RequireModifyATRModeMsgSetDB(sqlite3 *db, ats_co_RequireModifyATRModeMsg *message);

int
ats_co_LoadTimeTableMsgSetDB(sqlite3 *db, ats_co_LoadTimeTableMsg *message);

int
ats_co_TTPLoadTimeTableMsgSetDB(sqlite3 *db, ats_co_InterfaceTTPMsg *message);

int
ats_co_HMItoCASMsgSetDB(sqlite3 *db, ats_co_InterfaceHMItoCASMsg *message);

int
ats_co_CFepRequireCASDataMsgSetDB(sqlite3 *db, ats_co_CFepRequireCASDataMsg *message);

int
ats_co_AlarmMsgSetDB(sqlite3 *db, ats_co_AlarmMsg *message);

int
ats_co_AlarmMsgOfOutSetDB(sqlite3 *db, ats_co_ErrIndicationMsg *message);

int
ats_co_LCfepNetStatusMsgSetDB(sqlite3 *db, ats_co_LCfepNetStatusMsg *message);

int
ats_co_CASNetStatusMsgSetDB(sqlite3 *db, ats_co_CASNetStatusMsg *message);

int
ats_co_InterfaceTrackStatusMsgSetDB(sqlite3 *db, ats_co_InterfaceTrackStatusMsg *message);

int
ats_co_HmiToATRManualRegulationValueSetDB(sqlite3 *db, ats_co_HmiToATRManualRegulationValue *message);
int ats_co_RecieveTrainidSynSetDB(sqlite3 *db, ats_co_RecieveTrainidSynMsg *message);
int
ats_co_BufferConvertToHmiToATRManualRegulationValue(const BYTE buf[], ats_co_HmiToATRManualRegulationValue *message);



int
ats_co_Update_Interlock_for_repalydb(sqlite3 *db, ATS_CO_MSG *handle);



int ats_co_BufferConvertToTTPMsg(const BYTE buf[] ,ats_co_InterfaceTTPMsg  *message);
int
ats_co_BufferConvertToControlRightAutoMsg(const BYTE buf[] ,ats_co_TsControlRightAutoMsg  *message);
int
ats_co_BufferConvertToCASBeatMsg(const BYTE buf[] ,ats_co_CASBeatMsg  *message);
int
ats_co_BufferConvertToRecieveTrainidRequireSynMsg(const BYTE buf[] ,ats_co_RecieveLoadOKMsg  *message);
int
ats_co_BufferConvertToLoadOKMsg(const BYTE buf[] ,ats_co_LoadOKmsg  *message);
int
ats_co_TTPLoadTimeTableMsgSetDB(sqlite3 *db, ats_co_InterfaceTTPMsg *message);

int
ats_co_ControlRightAutoMsgSetDB(sqlite3 *db, ats_co_TsControlRightAutoMsg *message);
int
ats_co_CASBeatMsgSetDB(sqlite3 *db, ats_co_CASBeatMsg *message);
int
ats_co_LoadOKMsgSetDB(sqlite3 *db, ats_co_LoadOKmsg *message);
int
ats_co_RecieveTrainidSynRequireSetDB(sqlite3 *db, ats_co_RecieveLoadOKMsg *message);
int
ats_co_DirectionMsgSetDB(sqlite3 *db, ats_co_DirectionMsg *message);
int
ats_co_PTIMsgSetDB(sqlite3 *db, ats_co_PTIMsg *message);



int ats_co_BufferConvertToHMItoCASMsg(const BYTE buf[] ,ats_co_InterfaceHMItoCASMsg  *message);

int ats_co_logicalMsgFunc(sqlite3 *db, ATS_CO_MSG *handle);

int ats_co_log_GetFreeStatus (int *freestatus, sqlite3 *db, const int trackid);

void  ats_mpu_unpackElementStatus(unsigned char * data, int len);
void  ats_mpu_unpackCommandConfirm(unsigned char *data , int len);



#ifdef  __cplusplus
}
#endif

#endif /* ATS_CO_MESSAGE_HH_ */
