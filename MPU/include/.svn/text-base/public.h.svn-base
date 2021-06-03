/*
 * public.h
 *
 *  Created on: Feb 19, 2012
 *      Author: root
 */

#ifndef PUBLIC_H_
#define PUBLIC_H_

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned long long int NetTime;
//typedef unsigned int u64;
//typedef unsigned int NetTime;

/*-- Global typedefs --*/

typedef struct _DateTime
{
   int day;
   int month;
   int year;
   int second;
   int minute;
   int hour;
}DateTime;

typedef struct _message_header
{
	unsigned long long int timestamp; /** the timestamp of sending the message */
	unsigned int seqno; /** sequence no */
	unsigned char srcno; /** source ip no */
	unsigned char destno; /** destination ip no */
	unsigned char srcmodule; /** source module no */
	unsigned char destmodule; /** destination module no */
	unsigned short datatype; /** type of the data */
	unsigned char protocoltype; /** type of the protocol */
	unsigned char mainorbak; /** server flag */
	unsigned short remain;
	unsigned short datalen; /** length of the data */
} message_header_t;

typedef struct _ats_co_TimeTableLoad
{
	int msgType;//<value=><type=default(int)><bytes=0-1><bits=default(all)>
	int length;//<value=201><type=default(int)><bytes=2-5><bits=default(all)>
	int innerID;
	int originComponent;
	int originMachine;
}ats_co_TimeTableLoad;

typedef struct
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
	//	int trainIndex;//<value=><type=default(int)><bytes=31-32><bits=default(all)>
	//	int atrStatus;//<value=><type=default(int)><bytes=33><bits=default(all)>
	//	int arsStatus;//<value=><type=default(int)><bytes=34><bits=default(all)>
		int faultMsg;
		int trainMsg;
		int statusMsg;
		int diagMsg;
}InnDataForTmt;

//typedef struct
//{
//	int index;
//    int internal;
//    int telegramType;
//    int logicalFree;
//    int atpAvailable;
//    int atpVacancy;
//    int freeStatus;
//    int trackid;
//    char trainID;
//    int carNumber;
//    int trainIndex;
//    int destination;
//    int service;
//    int serial;
//    int crew;
//    int line;
//    int driverMode;
//}CoDataForTmt;

typedef struct
{
  int elementType; //<Ԫ������><value=207/200>
  //int elementid; //<Ԫ������><value=>

  int trackdirection; //<�����·����><value=0û�з���1�����С����� 2�����С�����>
  int tracklockFlag; //<��������־><value=0������ 1����>
  int trackfreeStatus; //<�߼��������״̬><value=0ռ�� 1����>
  int trackstatus; //<���ATPVacancy״̬><value=0δ���� 1���� 2ATP�г�ռ�� 3��ATP�г�ռ�� 4����>
  int switchpositionFlag; //<����λ��><value=0�Ŀ� 1��λ 2��λ 3����>
  int switchclaimStatus; //<��������״̬><value=0δ���� 1��·���� 2�����������>
  int switchblockAgainstThrowing; //<���?��״̬><value=0������ 1��>
  int switchlockFlag; //<��������־><value=0������ 1����>
  int switchfreeStatus; //<������б�־><value=0ռ�� 1����>
  int finish;

      int index;
      int internal;
      int telegramType;
      int logicalFree;
      int atpAvailable;
      int atpVacancy;
//      int freeStatus;
      int physicalFree;
      int trackid;
      char trainID;
      int carNumber;
      int trainIndex;
      int destination;
      int service;
      int serial;
      int crew;
      int line;
      int driverMode;

} CoDataForTmt;
#endif /* PUBLIC_H_ */
