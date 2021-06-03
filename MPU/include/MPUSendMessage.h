/*
 * mpusendmessage.h
 *
 *  Created on: 2012-6-22
 *      Author: root
 */

#ifndef MPUSENDMESSAGE_H_
#define MPUSENDMESSAGE_H_

#include "MPUSendConvert.h"
#include "MPUConvert.h"
#include "sqlite3.h"



typedef struct
{
	unsigned char cas_equipmentID_low;         //1
	unsigned char cas_equipmentID_high;        //2
	unsigned char cas_netstatus_1;             //3
	unsigned char cas_netstatus_2;             //4
	unsigned char cas_netstatus_3;             //5
	unsigned char cas_netstatus_4;             //6
	unsigned char cas_netstatus_5;             //7
	unsigned char cas_netstatus_6;             //8
	unsigned char cas_netstatus_remain_1;      //9
	unsigned char cas_netstatus_remain_2;      //10
	unsigned char cas_netstatus_remain_3;      //11
	unsigned char cas_netstatus_remain_4;      //12
	unsigned char cas_netstatus_remain_5;      //13
	unsigned char cas_netstatus_remain_6;      //14

	unsigned char lfep1_equipmentID_low;       //15
	unsigned char lfep1_equipmentID_high;      //16
	unsigned char lfep1_netstatus_1;           //17
	unsigned char lfep1_netstatus_2;           //18
	unsigned char lfep1_netstatus_3;           //19
	unsigned char lfep1_netstatus_4;           //20
	unsigned char lfep1_netstatus_5;           //21
	unsigned char lfep1_netstatus_remain_1;    //22
	unsigned char lfep1_netstatus_remain_2;    //23
	unsigned char lfep1_netstatus_remain_3;    //24
	unsigned char lfep1_netstatus_remain_4;    //25
	unsigned char lfep1_netstatus_remain_5;    //26

	unsigned char lfep2_equipmentID_low;       //27
	unsigned char lfep2_equipmentID_high;      //28
	unsigned char lfep2_netstatus_1;           //29
	unsigned char lfep2_netstatus_2;           //30
	unsigned char lfep2_netstatus_3;           //31
	unsigned char lfep2_netstatus_4;           //32
	unsigned char lfep2_netstatus_5;           //33
	unsigned char lfep2_netstatus_remain_1;    //34
	unsigned char lfep2_netstatus_remain_2;    //35
	unsigned char lfep2_netstatus_remain_3;    //36
	unsigned char lfep2_netstatus_remain_4;    //37
	unsigned char lfep2_netstatus_remain_5;    //38

	unsigned char lfep3_equipmentID_low;       //39
	unsigned char lfep3_equipmentID_high;      //40
	unsigned char lfep3_netstatus_1;           //41
	unsigned char lfep3_netstatus_2;           //42
	unsigned char lfep3_netstatus_3;           //43
	unsigned char lfep3_netstatus_4;           //44
	unsigned char lfep3_netstatus_5;           //45
	unsigned char lfep3_netstatus_remain_1;    //46
	unsigned char lfep3_netstatus_remain_2;    //47
	unsigned char lfep3_netstatus_remain_3;    //48
	unsigned char lfep3_netstatus_remain_4;    //49
	unsigned char lfep3_netstatus_remain_5;    //50

	unsigned char lfep4_equipmentID_low;       //51
	unsigned char lfep4_equipmentID_high;      //52
	unsigned char lfep4_netstatus_1;           //53
	unsigned char lfep4_netstatus_2;           //54
	unsigned char lfep4_netstatus_3;           //55
	unsigned char lfep4_netstatus_4;           //56
	unsigned char lfep4_netstatus_5;           //57
	unsigned char lfep4_netstatus_remain_1;    //58
	unsigned char lfep4_netstatus_remain_2;    //59
	unsigned char lfep4_netstatus_remain_3;    //60
	unsigned char lfep4_netstatus_remain_4;    //61
	unsigned char lfep4_netstatus_remain_5;    //62

	unsigned char cfep1_equipmentID_low;       //63
	unsigned char cfep1_equipmentID_high;      //64
	unsigned char cfep1_netstatus_1;           //65
	unsigned char cfep1_netstatus_2;           //66

	unsigned char cfep2_equipmentID_low;       //67
	unsigned char cfep2_equipmentID_high;      //68
	unsigned char cfep2_netstatus_1;           //69
	unsigned char cfep2_netstatus_2;           //70
}AllNetStatusToHMI;

// struct for tmt as follows.............................................
typedef struct _All_TrainDataStruct
{
	unsigned char originMachine;
	unsigned char originComponent;
	unsigned short destination;
	unsigned short service;
	unsigned char serial;
	unsigned char line;
	int car1;
	int car2;
	unsigned short driver;
	int warpTime;
	unsigned char trainIDFault;
	unsigned char driveMode;
	unsigned short logicalSection;
	unsigned char trainStatus;
}AllTrainDataStruct;

typedef struct _All_TrainDataByte
{
	unsigned char destination_low;
	unsigned char destination_high;
	unsigned char service_low;
	unsigned char service_high;
	unsigned char serial;
	unsigned char line;
	unsigned char car_one_1;
	unsigned char car_one_2;
	unsigned char car_one_3;
	unsigned char car_two_1;
	unsigned char car_two_2;
	unsigned char car_two_3;
	unsigned char driver_low;
	unsigned char driver_high;
	unsigned char warpTime_low;
	unsigned char warpTime_high;
	unsigned char driveMode;
	unsigned char trainIDFault;
	unsigned char logicalSection_low;
	unsigned char logicalSection_high;
	unsigned char trainStatus;
}AllTrainDataByte;

typedef struct _Train_ID
{
	unsigned char destination_ID_low;					/** destination ID:low unsigned short */
	unsigned char destination_ID_high;					/** destination ID:high byte */
	unsigned char service_ID_low;						/** service ID:low byte */
	unsigned char service_ID_high;						/** service ID:high byte */
	unsigned char serial;								/** serial number */
	unsigned char driver_low;
	unsigned char driver_high;
	unsigned char line;
	unsigned char car_one_1;
	unsigned char car_one_2;
	unsigned char car_one_3;
	unsigned char car_two_1;
	unsigned char car_two_2;
	unsigned char car_two_3;
	unsigned char logicalSection_low;
	unsigned char logicalSection_high;
	unsigned char runWarp_1;
	unsigned char runWarp_2;
	unsigned char runWarp_3;
	unsigned char runWarp_4;
	unsigned char trainStatus;
	unsigned char trainIDFault;
	unsigned char driveMode;
	unsigned char carSpeed_low;
	unsigned char carSpeed_high;
	unsigned char trainNO_low;
	unsigned char trainNO_high;
	unsigned char trainDirection;
	unsigned char arsStatus;
	unsigned char regulationMode;
}TrainID;

typedef struct _Tmt_FaultInfo
{
	unsigned char deviceType;
	unsigned char faultType;
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char year_low;
	unsigned char year_high;
	unsigned char faultNO;
	unsigned char operatorNO;
	unsigned char orderCounter_low;
	unsigned char orderCounter_high;
	unsigned char parameter[0];
}TmtFaultInfo;

//typedef struct _Train_Identify_NO
//{
//	unsigned char PDI;
//	unsigned char datatype_low;
//	unsigned char datatype_high;
//	unsigned char datalen;
//	unsigned char consoleNO;
//	unsigned char disposalNO;
//	unsigned char functionNO_low;
//	unsigned char functionNO_high;
//	unsigned char elementType;
//	unsigned char trainIndex_low;
//	unsigned char trainIndex_high;
//	unsigned char destination_low;
//	unsigned char destination_high;
//	unsigned char service_low;
//	unsigned char service_high;
//	unsigned char serial;
//	unsigned char crew_low;
//	unsigned char crew_high;
//	unsigned char line;
//	unsigned char car_low;
//	unsigned char car_high;
//}TrainIdentifyNO;
typedef struct _Train_Identify_NO
{
	unsigned char PDI;
	unsigned char datatype_low;
	unsigned char datatype_high;
	unsigned char datalen;
	unsigned char consoleNO;
	unsigned char disposalNO;
	unsigned char functionNO_low;
	unsigned char functionNO_high;
	unsigned char elementType;
	unsigned char TWCIndex_low;
	unsigned char TWCIndex_high;

//	//add by quju
//	unsigned char trainindex_low;
//	unsigned char trainindex_high;
//	unsigned char destination_low;
//	unsigned char destination_high;
//	unsigned char service_low;
//	unsigned char service_high;
//	unsigned char serial_low;
//	unsigned char crewid_low;
//	unsigned char crewid_high;
//	unsigned char lineid_low;
//	unsigned char lineid_high;
//	unsigned char carid_low;
//	unsigned char carid_high;
//
//	unsigned char serial_high;
// ///end add by quj

//	unsigned char runClass;
//	unsigned char platform_index_low;
//	unsigned char platform_index_high;

	unsigned char next_platform_index_high;
	unsigned char runClass_and_next_platform_index_low;

	unsigned char destination_ID_low;					  /** destination ID:low byte */
	unsigned char service_ID_high_and_destination_ID_high;
	unsigned char service_ID_low;						  /** service ID:low byte */
	unsigned char serial;								  /** serial number */
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}TrainIdentifyNO;
//add by quj cas heartbeat msg
typedef struct _CasheartbeatMsg
{
	unsigned char datatype_low;
	unsigned char datatype_high;
	unsigned char msg3;
	unsigned char msg4;
	unsigned char msg5;
}CasheartbeatMsg;
typedef struct _CasSwitch2HMIFEPMsg
{
	unsigned char switch_flag;
	unsigned char fepid;
}CasSwitch2HMIFEPMsg;
typedef struct _Center_TRC_SYNC
{
}Center_TRC_SYNC;
//add by quj
typedef struct _Train_Identify_NO_CBTC
{
	unsigned char PDI;
	unsigned char datatype_low;
	unsigned char datatype_high;
	unsigned char datalen;
	unsigned char consoleNO;
	unsigned char disposalNO;
	unsigned char functionNO_low;
	unsigned char functionNO_high;
	unsigned char elementType;
//	unsigned char TWCIndex_low;
	//unsigned char TWCIndex_high;

	//add by quju
	unsigned char trainindex_low;
	unsigned char trainindex_high;
	unsigned char destination_low;
	unsigned char destination_high;
	unsigned char service_low;
	unsigned char service_high;
	unsigned char serial_low;
	unsigned char crewid_low;
	unsigned char crewid_high;
	unsigned char lineid;
	//unsigned char lineid_high;
	unsigned char carid_low;
	unsigned char carid_high;

	unsigned char serial_high;
 ///end add by quj

//	unsigned char runClass;
//	unsigned char platform_index_low;
//	unsigned char platform_index_high;
//
//	unsigned char next_platform_index_high;
//	unsigned char runClass_and_next_platform_index_low;
//
//	unsigned char destination_ID_low;					  /** destination ID:low byte */
//	unsigned char service_ID_high_and_destination_ID_high;
//	unsigned char service_ID_low;						  /** service ID:low byte */
//	unsigned char serial;								  /** serial number */
//	unsigned char year;
//	unsigned char month;
//	unsigned char day;
//	unsigned char hour;
//	unsigned char minute;
//	unsigned char second;
}TrainIdentifyNO_CBTC;
typedef struct _load_Request
{
	unsigned char PDI;
	unsigned char datatype_low;
	unsigned char datatype_high;
	unsigned char datalen;
	unsigned char body;
}LoadRequest;

typedef struct _InterfaceTrackStatus_To_Fep
{
	unsigned char trackid_1;
	unsigned char trackid_2;
	unsigned char trackid_3;
	unsigned char trackid_4;
	unsigned char freestatus;
	unsigned char car1_1;
	unsigned char car1_2;
	unsigned char car2_1;
	unsigned char car2_2;
}InterfaceTrackStatus_To_Fep;




// struct for atr as follows.............................................
typedef struct
{
	unsigned char timetable_innernumber;
} LoadedPlanTimeTable;

typedef struct
{
	unsigned char success_or_no;
	unsigned char timetable_innernumber;
	unsigned char versionno_1;
	unsigned char versionno_2;
	unsigned char versionno_3;
	unsigned char versionno_4;
} TimeTableLoadResult;

typedef struct
{
	//354
	unsigned char success_or_no;
	unsigned char timetable_innernumber;
	unsigned char versionno_1;
	unsigned char versionno_2;
	unsigned char versionno_3;
	unsigned char versionno_4;
} TimeTableLoadResultToTTP;

typedef struct
{
	unsigned char timetable_innernumber;
	unsigned char activaltimetable_or_no;													/** mode */
} FeedbackRunning;

typedef struct
{
	//205
	unsigned char type;
	unsigned char reverse;

} CASToHMI;

typedef struct
{
	unsigned char machine_number_low;					/** machine number:low byte */
	unsigned char machine_number_high;					/** machine number:high byte */
	unsigned char component_number_low;					/** component number:low byte */
	unsigned char component_number_high;				/** component number:high byte */
	unsigned char loaded_inner_time_table_number_low;	/** loaded inner time table number:low byte */
	unsigned char loaded_inner_time_table_number_high;	/** loaded inner time table number:high byte */
	unsigned char error_number;							/** error code */
	unsigned char load_result;							/** load result number */
} ScheduleResult;

typedef struct
{
	unsigned char destination_ID_low;					/** destination ID:low byte */
	unsigned char destination_ID_high;					/** destination ID:high byte */
	unsigned char service_ID_low;						/** service ID:low byte */
	unsigned char service_ID_high;						/** service ID:high byte */
	unsigned char serial;								/** serial number */
	unsigned char car_number_1_low;						/** car number one:low byte */
	unsigned char car_number_1_high;					/** car number one:high byte */
	unsigned char car_number_2_low;						/** car number two:low byte */
	unsigned char car_number_2_high;					/** car number two:high byte */
	unsigned char event_type;							/** event type */
	unsigned char station_ID_low;						/** station ID:low byte */
	unsigned char station_ID_high;						/** station ID:high byte */
	unsigned char time[8];								/** time */
	unsigned char deTime_1;
	unsigned char deTime_2;
	unsigned char deTime_3;
	unsigned char deTime_4;
} TrainEvent;


//Added by lfc 20121107
typedef struct
{
	unsigned char ElementType;
	unsigned char Element_ID1;
	unsigned char Element_ID2;
	unsigned char Alarm_minute;
	unsigned char Alarm_hour;
	unsigned char Alarm_day;
	unsigned char Alarm_month;
	unsigned char Alarm_year;
	unsigned char AlarmID_1;
	unsigned char AlarmID_2;
	unsigned char AlarmClose;
	unsigned char Parameter_1;
	unsigned char Parameter_2;
	unsigned char Parameter_3;
	unsigned char Parameter_4;
}COAlarmMessage;


typedef struct
{
	unsigned char type;									/** type number */
	unsigned char line_number;							/** line number */
	unsigned char train_index_low;						/** train index:low byte */
	unsigned char train_index_high;						/** train index:high byte */
	unsigned char mode;									/** mode */
	unsigned char para1_low;
	unsigned char para1_high;
	unsigned char para2_low;
	unsigned char para2_high;
	unsigned char destination_low;
	unsigned char destination_high;
	unsigned char serviceID_low;
	unsigned char serviceID_high;
	unsigned char serial;
	unsigned char car_one_1;
	unsigned char car_one_2;
	unsigned char car_two_1;
	unsigned char car_two_2;
} TrainMode;

typedef struct
{
	unsigned char destination_ID_low;					/** destination ID:low byte */
	unsigned char destination_ID_high;					/** destination ID:high byte */
	unsigned char service_ID_low;						/** service ID:low byte */
	unsigned char service_ID_high;						/** service ID:high byte */
	unsigned char serial;								/** serial number */
	unsigned char car_number_1_low;						/** car number one:low byte */
	unsigned char car_number_1_high;					/** car number one:high byte */
	unsigned char car_number_2_low;						/** car number two:low byte */
	unsigned char car_number_2_high;					/** car number two:high byte */
	unsigned char station_ID_low;						/** station ID:low byte */
	unsigned char station_ID_high;						/** station ID:high byte */
	unsigned char time[8];								/** time */
	unsigned char predictValue_low;
	unsigned char predictValue_high;
}PredictLine;

typedef struct _All_TrainModeData
{
	unsigned char line_number;
	int para1;
	int para2;
	unsigned char mode;
	int train_index;
	int destination_ID;
	int service_ID_low;
	unsigned char service_ID;
	unsigned char serial;
	int car_number_1;
	int car_number_2;
}AllTrainModeData;

typedef struct _All_TrainModeDataByte
{
	unsigned char line_number;							/** line number */
	unsigned char para1_low;
	unsigned char para1_high;
	unsigned char para2_low;
	unsigned char para2_high;
	unsigned char mode;									/** mode */
	unsigned char train_index_low;						/** train index:low byte */
	unsigned char train_index_high;						/** train index:high byte */
	unsigned char destination_ID_low;					/** destination ID:low byte */
	unsigned char destination_ID_high;					/** destination ID:high byte */
	unsigned char service_ID_low;						/** service ID:low byte */
	unsigned char service_ID_high;						/** service ID:high byte */
	unsigned char serial;								/** serial number */
	unsigned char car_number_1_low;						/** car number one:low byte */
	unsigned char car_number_1_high;					/** car number one:high byte */
	unsigned char car_number_2_low;						/** car number two:low byte */
	unsigned char car_number_2_high;					/** car number two:high byte */
}AllTrainModeDataByte;

typedef struct
{
	unsigned char operateResult;
	unsigned char line_number;							/** line number */
	unsigned char train_index_low;						/** train index:low byte */
	unsigned char train_index_high;						/** train index:high byte */
	unsigned char destination;
	unsigned char serviceID;
	unsigned char serial;
	unsigned char car_one_1;
	unsigned char car_one_2;
	unsigned char car_two_1;
	unsigned char car_two_2;
} HMIOperateFeedback;

//20130513 sunjh
typedef struct
{
	unsigned char eventtype;
	unsigned char line_number;							/** line number */
	unsigned char ptid_low;						/** train index:low byte */
	unsigned char ptid_high;						/** train index:high byte */
	unsigned char idle_one;
	unsigned char idle_two;
	unsigned char idle_three;
	unsigned char idle_four;

} TrainEventToHMI;


typedef struct
{
	unsigned char timetable_innernumber;									/** type number */
	unsigned char errornumber;							/** line number */
	unsigned char success_or_no;						/** train index:low byte */
	//unsigned char success_or_no_high;						/** train index:high byte */									/** mode */
} LoadTimeTableAct;

//typedef struct
//{
//	unsigned char consoleNO;
//	unsigned char taskNO;
//	unsigned char funcNO_low;
//	unsigned char funcNO_high;
//	unsigned char elementType;
//	unsigned char trainIndex_low;
//	unsigned char trainIndex_high;
//	unsigned char platform_index_low;
//	unsigned char platform_index_high;
//	unsigned char next_platform_index_low;
//	unsigned char next_platform_index_high;
//	unsigned char none;
//	unsigned char dwellTime_low;
//	unsigned char dwellTime_high;
//	unsigned char runTime_low;
//	unsigned char runTime_high;
//}DwellTripTime;
typedef struct
{
	unsigned char consoleNO;
	unsigned char taskNO;
	unsigned char funcNO_low;
	unsigned char funcNO_high;
	unsigned char elementType;
	unsigned char TWCIndex_low;
	unsigned char TWCIndex_high;
	unsigned char next_platform_index_high;
	unsigned char runClass_and_next_platform_index_low;
	unsigned char destination_ID_low;					  /** destination ID:low byte */
	unsigned char service_ID_high_and_destination_ID_high;
	unsigned char service_ID_low;						  /** service ID:low byte */
	unsigned char serial;								  /** serial number */
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}DwellTripTime;

typedef struct
{
	unsigned char consoleNO;
	unsigned char taskNO;
	unsigned char funcNO_low;
	unsigned char funcNO_high;
	unsigned char elementType;
    //added by lfc 20121229
	unsigned char trainIndex_low;
	unsigned char trainIndex_high;
	unsigned char currentPTID_low;
	unsigned char currentPTID_hign;

	unsigned char nextPTID_low;
	unsigned char nextPTID_high;
	unsigned char nextOSP;

	unsigned char dwellTime_low;
	unsigned char dwellTime_high;
	unsigned char travelTime_low;
	unsigned char travelTime_high;
	unsigned char nextStopPTID_low;
	unsigned char nextStopPTID_hign;
	unsigned char aSkipAtNextPTID;
	            //no use now
}DwellTripTime_CBTC;



typedef struct
{
	unsigned char consoleNO;
	unsigned char taskNO;
	unsigned char funcNO_low;
	unsigned char funcNO_high;
	unsigned char elementType;
	unsigned char platform_index_low;
	unsigned char platform_index_high;
	unsigned char parameter;
}ChangeOrReversal;

typedef struct
{
	unsigned char consoleNO;
	unsigned char taskNO;
	unsigned char funcNO_low;
	unsigned char funcNO_high;
	unsigned char elementType;
	unsigned char platform_index_low;
	unsigned char platform_index_high;
	unsigned char skipOrCancel;
}AutoOrCancelSkip;

typedef struct
{
	unsigned char innerElementType;
	unsigned char PTID_low;
	unsigned char PTID_high;
	unsigned char nextPTID_low;
	unsigned char nextPTID_high;
	unsigned char DTI_Index_low;
	unsigned char DTI_Index_high;
	unsigned char StopTime_Seconds_low;
	unsigned char StopTime_Seconds_high;
	unsigned char Date_Hour;
	unsigned char Date_Minute;
	unsigned char Date_Second;
	unsigned char CountDown_Info;
}DTIStopTime;







// struct for ars as follows.............................................
typedef struct
{
	unsigned char operator_console_number;				/** number of addressed operator console */
	unsigned char transaction_number;					/** transaction number for identification of operator action */
	unsigned char function_number_low;					/** operator command number:low byte */
	unsigned char function_number_high;				/** operator command number:high byte */
	unsigned char element_type;						/** type of element */
	unsigned char element_index_low;					/** element sequence number within the element type:low byte */
	unsigned char element_index_high;					/** element sequence number within the element type:high byte */
}OCT;






void  adapter_net_send_AllNetStatusToHMI(
		int machineNO,
		int componentNO,
		AllNetStatusToHMI message);



// interface for tmt as follows.............................................
void adapter_tmt_send_loadRequest(int machineNO, int componentNO, char interlockingZone, int sd, int fi, int rc, int timeYesOrNo);

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
		unsigned short serial);

void adapter_tmt_send_trainID (int machineNO, int componentNO, int destination, int service, int serial,
        int driver, int line, int car1, int car2, int logicalSection, int runWarp, int trainStatus,
        int trainIDFault, int driveMode, int carSpeed,int trainDirection,int trainNO,int arsStatus,int regulationMode);

void  adapter_tmt_send_TmtFaultInfo(int machineNO,int componentNO,int deviceType, int faultType, int second, int minute,
		int hour, int day, int month, int year, int fs, int faultNO, int operatorNO, int orderCounter,unsigned char* parameter, int parameterLength);

void  adapter_tmt_send_TrainIDModefyAct(int machineNO, int componentNO, int transationNO,int ack);

void  adapter_tmt_send_AllTrainDataStruct(int machineNO, int componentNO, char allTrainDataStruct[]);

void  adapter_tmt_send_AllTrainDataStruct2(int machineNO, int componentNO, AllTrainDataStruct atd[], int iCount);

void  adapter_tmt_send_AllTrainDataStructToFep(int machineNO, int componentNO, char allTrainDataStruct[]);

void adapter_tmt_send_interfacetrackstatus_to_fep (
		int machineNO,
		int componentNO,
        int trackid,
        int freestatus,
        int car1,
        int car2);

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
		int Parameter);

void adapter_co_send_CASToHMIMessage(
		int machineNO,
		int componentNO,
		int type);





// interface for atr as follows.............................................
void adapter_atr_send_loaded_plantimetable_to_tgi(
		unsigned short machine_number,
		unsigned short component_number,
		unsigned char loaded_inner_time_table_number);

void adapter_atr_send_timetable_loadresult_to_online(
		int machineNO,
		int componentNO,
		unsigned char success_or_no,
		unsigned char timetable_innernumber,
		int versionNO);

void adapter_atr_send_feedbackrunning(
		int machineNO,
		int componentNO,
		unsigned char timetable_innernumber,
		int activaltimetable_or_no);

void adapter_atr_send_load_result_to_online(unsigned short machine_number,
		unsigned short component_number,
		unsigned short loaded_inner_time_table_number,
		unsigned char error_number, unsigned char load_result);

void adapter_atr_send_load_result_to_hmi(unsigned short machine_number,
		unsigned short component_number,
		unsigned short loaded_inner_time_table_number,
		unsigned char error_number, unsigned char load_result);

void adapter_atr_send_events(int machineNO, int componentNO,unsigned short destination_ID,
		unsigned short service_ID, unsigned short serial,
		unsigned short car_number_1, unsigned short car_number_2,
		unsigned short event_type, unsigned short station_ID,
		u64 time, int deTime);

void adapter_atr_send_operation_over(unsigned short OC_machine_number,
		unsigned short OC_component_number);

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
		int car2);

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
		int predictValue);

void  adapter_atr_send_AllTrainModeDataStruct(
		int machineNO,
		int componentNO,
		char allTrainModeDataStruct[]);

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
		int car2);

void adapter_atr_send_trainEventToHMI(
		int machineNO,
		int componentNO,
		int eventType,
		int lineNO,
		int PTID,
		int idle1,
		int idle2,
		int idle3,
		int idle4);

void adapter_atr_send_load_timetable_act(int machineNO, int componentNO,
		unsigned short timetable_innernumber,unsigned short errornumber,
		unsigned short success_or_no);

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
	    unsigned short runClass);
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
	    );
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
		);
void adapter_atr_change_tip_or_reversal(int machineNO, int componentNO, int ConsoleNO, int interlockZone, int disposalNO, int functionNO, unsigned short elementType, unsigned short PTID, unsigned short parameter);

void adapter_atr_autoskip_or_cancelskip(int machineNO, int componentNO, int ConsoleNO, int interlockZone, int disposalNO, int functionNO, unsigned short elementType, unsigned short PTID, unsigned short skipOrCancel);

void  adapter_atr_send_AllStationDataStructToFep(
		int machineNO,
		int componentNO,
		char allStationDataStruct[]);

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
		unsigned short CountDown_ModeValue);


void adapter_tmt_send_CasheartbeatMSG();

void adapter_pl_send_PlCommandDatagram(sqlite3 *db);


// interface for ars as follows.............................................
void adapter_ars_auto_route_send(int machineNO, int componentNO, int operator_console_number,
		                    int interlockZone,
		                    int transaction_number,
							int function_number,
 							int element_type,
							int element_index);

// interface for load request as follows.............................................
void adapter_co_send_loadRequest(int machineNO,
							int componentNO,
						    int status_data,
						    int fault_indications,
							int reminder_collars,
 							int time);
#endif
