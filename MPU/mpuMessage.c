/*******************************************************************
 filename      mpuMessage.c
 author        root
 created date  2012-1-9
 description   specifications of implemented functions
 warning
 modify history
 author        date        modify        description
 *******************************************************************/



#include "include/MPUMessage.h"
#include "include/MPUReceiver.h"
#include "include/MPUtility.h"
#include "include/MPU.h"
#include "include/MPUReceiverPool.h"

#include <inttypes.h>
#include <sys/time.h>
#include "../log.h"
#include "../dllmain.h"


int fepStatusType = 0;
const char *  LOG_NAME_MPUMessage = "MPU.Message";


//
//���������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������
//���������������������������������������������������������������������������������������������������=(������������������������������������������������������������������������������������������ *������������������������������������������������������������������������������)
//total_type = $number(message_type) * $number(element_type)
//format : {
//					index,
//					message_type,	element_type,
//					ats_co_bufferConverToMsgFunc,
//   				ats_co_msgConverToBufferFunc,
//   				ats_co_setDBFunc,
//					ats_co_getDBFunc,
//   		}
#define MESSAGE_TYPE_NUMBER  	39
#define MESSAGE_ATTRIBUTER_NUMBER  		7

const char *ConversationConfig[MESSAGE_TYPE_NUMBER][MESSAGE_ATTRIBUTER_NUMBER] =
  {
    { "1", "400", "213", "ats_co_BufferConvertToTrackMsg",
        "ats_co_trackMsgConvertToBuffer", "ats_co_TrackMsgSetDB",
        "ats_co_TrackMsgGetDB" }, //circuit and cbtc
    { "2", "400", "151", "ats_co_BufferConvertToTrainStatusMsg",
        "ats_co_TrainStatusMsgConvertToBuffer", "ats_co_TrainStatusMsgSetDB",
        "ats_co_TrainStatusMsgGetDB" },//only for cbtc
    { "3", "434", "151", "ats_co_BufferConvertToTrainEventMsg",
        "ats_co_TrainEventMsgConvertToBuffer", "ats_co_TrainEventMsgSetDB",
        "ats_co_TrainEventMsgGetDB" },
    { "4", "400", "26", "ats_co_BufferConvertToPlatformMsg",
        "ats_co_PlatformMsgConvertToBuffer", "ats_co_PlatformMsgSetDB",
        "ats_co_PlatformMsgGetDB" },  //circuit and cbtc
    { "5", "400", "200", "ats_co_BufferConvertToSwitchMsg",
        "ats_co_SwitchMsgConvertToBuffer", "ats_co_SwitchMsgSetDB",
        "ats_co_SwitchMsgGetDB" },            //circuit and cbtc
    { "6", "400", "201", "ats_co_BufferConvertToSignalMsg",
        "ats_co_SignalMsgConvertToBuffer", "ats_co_SignalMsgSetDB",
        "ats_co_SignalMsgGetDB" },              //circuit and cbtc
    { "7", "400", "293", "ats_co_BufferConvertToAxleCounterMsg",
        "ats_co_AxleCounterMsgConvertToBuffer", "ats_co_AxleCounterMsgSetDB",
        "ats_co_AxleCounterMsgGetDB" },
    {"8","400","203","ats_co_BufferConvertToCrossingMsg","","ats_co_CrossingMsgSetDB",""},//only for cbtc
    {"9","400","158","ats_co_BufferConvertToAreaStatusMsg","","",""},
    //
    {"10","400","157","ats_co_BufferConvertToTrainPositionReport","",
    		"ats_co_TrainPostionMsgSetDB",""},//only for cbtc
    //
    {"11","400","204","ats_co_BufferConvertToRouteStatusMsg","","",""},
    {"12","433","237","ats_co_BufferConvertToErrIndicationMsg","","",""},
    {"13","433","213","ats_co_BufferConvertToErrIndicationMsg","","",""},
    {"14","433","154","ats_co_BufferConvertToErrIndicationMsg","","",""},
    {"15","433","151","ats_co_BufferConvertToErrIndicationMsg","","",""},
    {"16", "400", "207", "ats_co_BufferConvertToLogicalTrackMsg",
            "ats_co_LogicalTrackMsgConvertToBuffer", "ats_co_LogicalTrackMsgSetDB",
            "ats_co_LogicalTrackMsgGetDB" },            //only for cbtc
	{"17","400","28","ats_co_BufferConvertToDirectionMsg","","",""},
	//400 Load rsq start and ok
	{"18","430","","","","",""},
	{"19","431","","","","",""},
    {"21","434","","","","",""},
    {"22","400","209","ats_co_BufferConvertToPTIMsg","ats_co_PTIMsgConvertToBuffer",
        		"ats_co_PTIMsgSetDB","ats_co_PTIMsgGetDB"},
    {"100","201","0","ats_co_BufferConvertToALLMsgUpdate","","",""},
    {"101","204","0","ats_co_BufferConvertToTrainIDModify","","",""},
    //{"102","253","0","ats_co_BufferConvertToTimeTableLoad","","",""},
    {"103","400","208","ats_co_BufferConvertToTrainEventTWCMsg","","",""},
    {"104","155","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"105","256","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"106","253","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"107","608","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"108","50","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"109","60","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"110","701","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"111","260","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"112","351","0","ats_co_BufferConvertToRequireRunningMsg","","",""},
    {"113","400","23","ats_co_BufferConvertToFepMsg","","ats_co_FepMsgSetDB",""}, //add by quj handle ts fep status
    {"114","1000","0","ats_co_BufferConvertToCASBeatMsg","","ats_co_CASBeatMsgSetDB",""},//add by quj handle casheartbeat
//    {"115","431","0","ats_co_BufferConvertToLoadOKMsg","","ats_co_LoadOKMsgSetDB",""},
    {"116","650","0","ats_co_BufferConvertToRecieveTraninIDSynRequireMsg","","ats_co_RecieveTrainIDSynRequireMsgSetDB",""},
    {"117","651","0","ats_co_BufferConvertToRecieveTraninIDSynMsg","","ats_co_RecieveTrainIDSynMsgSetDB",""},
	 {"118","205","0","ats_co_BufferConvertToHMItoCASMsg","","ats_co_RecieveTrainIDSynMsgSetDB",""}
	
  };

/*******************************************************************
 Function name		ats_co_travelConversionConfig
 description			search configuration of message using message
 type and element type
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
//int
//ats_co_travelConversationConfig2(ATS_CO_MSG *handle)
//{
////if we have set message_tpye and element_tpye value ,
////then we can travel configuration list to get index value and configuration of functions
//  int i, j, re;
//
//
//  for (i = 0; i < MESSAGE_TYPE_NUMBER; i++)
//    {
//    for (j = 1; j < MESSAGE_ATTRIBUTER_NUMBER; j++)
//      {
//
//      re = atoi(ConversationConfig[i][j]);
//      if (re != 0) {
//
//    	 if(handle->header.datatype==2)
//    	 	 	 {
//        if (((atoi(ConversationConfig[i][j]) == handle->config.message_type))
//            && ((atoi(ConversationConfig[i][j + 1])
//                == handle->config.element_type)))
//          	  	  	  {
////            ILOG( "<%s>packet type:datatype==2,message_tpye=%s,element_tpye=%s",LOG_NAME_MPUMessage,
////                ConversationConfig[i][j], ConversationConfig[i][j+1]);
//            handle->config.index = atoi(ConversationConfig[i][j - 1]);
//            continue;
//          	  	  	   }
//    	 	 	 }
//       else if(handle->header.datatype==1)
//        		{
//    	   if (atoi(ConversationConfig[i][j]) == handle->config.message_type)
//
//        	          {
////        	    ILOG( "<%s>packet type:datatype=1,message_tpye=%s,element_tpye=%s",
////        	    		LOG_NAME_MPUMessage,
////        	        ConversationConfig[i][j], ConversationConfig[i][j+1]);
//        	        handle->config.index = atoi(ConversationConfig[i][j - 1]);
//        	     continue;
//        	          }
//        		}
//       else {
//        			handle->config.index=-1;
//        		}
//    }
//      }
//}
//
// //to check parse is correct
//if (handle->config.index <= 0)
//{
//		ELOG("<%s>pattern type(data_type=%d,message_type=%d,element_type=%d) is not exist",
//				LOG_NAME_MPUMessage,
//      handle->header.datatype,handle->config.message_type, handle->config.element_type);
//  return false;
//}
//return true;
//}

int
ats_co_travelConversationConfig(ATS_CO_MSG *handle)
{
//if we have set message_tpye and element_tpye value ,
//then we can travel configuration list to get index value and configuration of functions
  int i, re;

  for (i = 0; i < MESSAGE_TYPE_NUMBER; i++)
  {
  	re = atoi(ConversationConfig[i][1]);
  	if (re != 0)
  	{
  		if(handle->header.datatype==2)
  		{
  			if (((atoi(ConversationConfig[i][1]) == handle->config.message_type))
  		     && ((atoi(ConversationConfig[i][2])== handle->config.element_type)))
  			{
//  				ILOG( "<%s>packet type:hostid=%d,seqno=%d,datatype==2,message_tpye=%s,element_tpye=%s",
//  						LOG_NAME_MPUMessage,
//  						handle->header.srcno,
//  						handle->header.seqno,
//  						ConversationConfig[i][1], ConversationConfig[i][2]);
  				handle->config.index = atoi(ConversationConfig[i][0]);
  				break;
  			}
  		}
  		else if(handle->header.datatype==1)
  		{
  			if (((atoi(ConversationConfig[i][1]) == handle->config.message_type)))
  			{
//  				ILOG( "<%s>packet type:hostid=%d,seqno=%d,datatype==1,message_tpye=%s",
//  						LOG_NAME_MPUMessage,
//  						handle->header.srcno,
//  						handle->header.seqno,
//  				  		ConversationConfig[i][1]);
  				handle->config.index = atoi(ConversationConfig[i][0]);
  				break;
  			}
  		}
  		else
  		{
  			handle->config.index = -1;
  		}

  	}

  }

  //to check parse is correct
  if (handle->config.index < 0)
  {
	  return false;
  }
  return true;
}
/*******************************************************************
 Function name			ats_co_MSG_compare
 description		    compare the msg head
 parameter
 ATS_CO_MSG *         		 IN/OUT    	 	    source
 ATS_CO_MSG *         		 IN/OUT    	 	    dest
 Return value
 true or false

 *******************************************************************/
int
ats_co_MSG_compare(ATS_CO_MSG *source, ATS_CO_MSG *dest)
{
	int i = 0;
	u_int32 src, des;
	for (i = 0; i < (int) source->fixed_len; i++)
	{
		src = (u_int32) bytes2int(&source->format[i], CHAR2BYTE);
		des = (u_int32) bytes2int(&source->format[i], CHAR2BYTE);

		if ((src - des) != 0)
		{
		  return (int) (src - des);
		}
	}
	return true;

}

/*******************************************************************
 Function name			ats_co_initMessageValue
 description			initialize data structure of message handle
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_initMessageValue(ATS_CO_MSG *handle)
{
	//for packet of ATS business
	//get message type here ,cast for ATS_CO_MSG
	//alter for internal message in 2012-03-01
	if((handle->header.destmodule) <= 200)
	{
	if(handle->header.datatype == 2)
	{
		if((handle->format+1 != NULL) &&(handle->format+6 != NULL))
		{
			handle->config.message_type = bytes2int(handle->format + 1, SHT2BYTE);
			handle->config.element_type = bytes2int(handle->format + 6,CHAR2BYTE);
			ILOG("Msg $$$$$$$$$$$$$$  %d,%d",handle->config.message_type,handle->config.element_type);
		}
	}
	else if (handle->header.datatype == 1)
	{
		handle->config.message_type = bytes2int(handle->format,SHT2BYTE);
	}
	else
	{
		return false ;
	}

	//get index value and names of all functions
	if (ats_co_travelConversationConfig(handle) == false)
		return false;

	}
	// init the message handle
	switch (handle->config.index)
	{
		case 0: //ats_co_FepStatusMsg
		  handle->message = malloc(sizeof(ats_co_FepStatusMsg));
		  memset(handle->message, 0, sizeof(ats_co_FepStatusMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToFepStatusMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_FepStatusMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_FepStatusSetDB;
		  break;
		case 1: //ats_co_TrackMsg
		  handle->message = malloc(sizeof(ats_co_TrackMsg));
		  memset(handle->message, 0, sizeof(ats_co_TrackMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToTrackMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_TrackMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_TrackMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_TrackMsgGetDB;
		  break;
		case 2: //msgType:151 ats_co_TrainStatusMsg : need update inputtask db for tmt
		  handle->message = malloc(sizeof(ats_co_TrainStatusMsg));
		  memset(handle->message, 0, sizeof(ats_co_TrainStatusMsg));
		  handle->bufferConvertToMsgFunc = (void*) ats_co_BufferConvertToTrainStatusMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_TrainStatusMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_TrainStatusMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_TrainStatusMsgGetDB;
		  break;
		case 3: //ats_co_TrainEventMsg : need update inputtask db for atr
		  handle->message = malloc(sizeof(ats_co_TrainEventMsg));
		  memset(handle->message, 0, sizeof(ats_co_TrainEventMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToTrainEventMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_TrainEventMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_TrainEventMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_TrainEventMsgGetDB;
		  break;
		case 4: // ������������������������������������������������������������������������������������
		  handle->message = malloc(sizeof(ats_co_PlatformMsg));
		  memset(handle->message, 0, sizeof(ats_co_PlatformMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToPlatformMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_PlatformMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_PlatformMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_PlatformMsgGetDB;
		  break;
		case 5: // ������������������������������������������������������?������������������
		  handle->message = malloc(sizeof(ats_co_SwitchMsg));
		  memset(handle->message, 0, sizeof(ats_co_SwitchMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToSwitchMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_SwitchMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_SwitchMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_SwitchMsgGetDB;
		  break;
		case 6: // ������������������������������������������������������������������������������������
		  handle->message = malloc(sizeof(ats_co_SignalMsg));
		  memset(handle->message, 0, sizeof(ats_co_SignalMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToSignalMsg;
		  handle->msgConvertToBufferFunc = (void *) ats_co_SignalMsgConvertToBuffer;
		  handle->setDBFunc = (void *) ats_co_SignalMsgSetDB;
		  handle->getDBFunc = (void *) ats_co_SignalMsgGetDB;
		  break;
		case 7: // ���������������������������������������������������������������������������������
		  handle->message = malloc(sizeof(ats_co_AxleCounterMsg));
		  memset(handle->message, 0, sizeof(ats_co_AxleCounterMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToAxleCounterMsg;
		  //handle->msgConvertToBufferFunc = (void *) ats_co_AxleCounterMsgConvertToBuffer;
		  //handle->setDBFunc = (void *) ats_co_AxleCounterMsgSetDB;
		  //handle->getDBFunc = (void *) ats_co_AxleCounterMsgGetDB;
		  break;
		case 8: //������������������
		  handle->message = malloc(sizeof(ats_co_CrossingMsg));
		  memset(handle->message, 0, sizeof(ats_co_CrossingMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToCrossingMsg;
		  handle->setDBFunc = (void *)ats_co_CrossingMsgSetDB;
		  break;
		case 9: //������������������������������������
		  handle->message = malloc(sizeof(ats_co_AreaStatusMsg));
		  memset(handle->message, 0, sizeof(ats_co_AreaStatusMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToAreaStatusMsg;
		  break;
		case 10: //������������������������������������������������������  : msgType:157 need update inputtask db for tmt
		  handle->message = malloc(sizeof(ats_co_TrainPositionReport));
		  memset(handle->message, 0, sizeof(ats_co_TrainPositionReport));
		  handle->bufferConvertToMsgFunc =(void *) ats_co_BufferConvertToTrainPositionReport;
		  handle->setDBFunc = (void *) ats_co_TrainPositionMsgSetDB ;
		  handle->getDBFunc = (void *) ats_co_TrainPositionMsgGetDB;
		  break;
		case 11: //������������������������������������
			handle->message = malloc(sizeof(ats_co_RouteStatusMsg));
			memset(handle->message, 0, sizeof(ats_co_RouteStatusMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToRouteStatusMsg;
			handle->setDBFunc = (void *) ats_co_RouteStatusMsgSetDB ;
			break;
		case 12: //���������������������������������������������������������������WCU������������������������������������������������������������������������
			handle->message = malloc(sizeof(ats_co_ErrIndicationMsg));
			memset(handle->message, 0, sizeof(ats_co_ErrIndicationMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToErrIndicationMsg;
			break;
		case 13: //������������������/���������������������������������������������������������������������������������������������������
			handle->message = malloc(sizeof(ats_co_ErrIndicationMsg));
			memset(handle->message, 0, sizeof(ats_co_ErrIndicationMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToErrIndicationMsg;
			handle->setDBFunc = (void *) ats_co_AlarmMsgOfOutSetDB;

			break;
		case 14: //���������������������������������������������������������������������������������
			handle->message = malloc(sizeof(ats_co_ErrIndicationMsg));
			memset(handle->message, 0, sizeof(ats_co_ErrIndicationMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToErrIndicationMsg;
			break;
		case 15: //������������������������������������������������������������������������
			handle->message = malloc(sizeof(ats_co_ErrIndicationMsg));
			memset(handle->message, 0, sizeof(ats_co_ErrIndicationMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToErrIndicationMsg;
			//handle->logicalMsgFunc=(void*)logical_process;
			break;
		case 16: //ats_co_logicalTrackMsg
			handle->message = malloc(sizeof(ats_co_logicalTrackMsg));
			memset(handle->message, 0, sizeof(ats_co_logicalTrackMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToLogicalTrackMsg;
			handle->setDBFunc = (void *) ats_co_LogicalTrackMsgSetDB;
			break;

		case 17://Driection
			ILOG("set driection msg parse1");
			handle->message = malloc(sizeof(ats_co_DirectionMsg));
			memset(handle->message, 0, sizeof(ats_co_DirectionMsg));
			//ILOG("set driection msg parse2");
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToDirectionMsg;
			//ILOG("set driection msg parse3");
			handle->setDBFunc=(void *)ats_co_DirectionMsgSetDB;
			break;
		case 18:
			ILOG("receive 400 430msg Load rsq start");
			break;
		case 19:
			handle->message = malloc(sizeof(ats_co_LoadOKmsg));
			memset(handle->message, 0, sizeof(ats_co_LoadOKmsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToLoadOKMsg;
			handle->setDBFunc = (void *) ats_co_LoadOKMsgSetDB;
			ILOG("115 msg1");
			ILOG("receive 400 431msg Load rsq end");
			break;
		case 22:
			ILOG("handle PTI msg 400 209");
			handle->message = malloc(sizeof(ats_co_PTIMsg));
			memset(handle->message, 0, sizeof(ats_co_PTIMsg));
			//ILOG("set driection msg parse2");
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToPTIMsg;
			//ILOG("set driection msg parse3");
			handle->setDBFunc=(void *)ats_co_PTIMsgSetDB;
			break;
		case 100: // datatype:1 msgtype :201 : need update inputtask db for tmt
			handle->message = malloc(sizeof(ats_co_ALLMsgUpdate));
			memset(handle->message, 0, sizeof(ats_co_ALLMsgUpdate));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToALLMsgUpdate;
			handle->setDBFunc=(void *)ats_co_ALLMsgUpdateSetDB;
			break;
		case 101: // datatype:1 msgtype :204 : need update inputtask db for tmt
			handle->message = malloc(sizeof(ats_co_TrainIDModify));
			memset(handle->message, 0, sizeof(ats_co_TrainIDModify));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToTrainIDModify;
			handle->setDBFunc=(void *)ats_co_TrainIDModifySetDB;
			break;
		case 103:
			handle->message = malloc(sizeof(ats_co_TrainEventTWCMsg));
			memset(handle->message, 0, sizeof(ats_co_TrainEventTWCMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToTrainEventTWCMsg;
			handle->setDBFunc=(void *)ats_co_TrainEventTWCMsgSetDB;
			break;
		case 104:
			handle->message = malloc(sizeof(ats_co_RequireRunningMsg));
			memset(handle->message, 0, sizeof(ats_co_RequireRunningMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToRequireRunningMsg;
			handle->setDBFunc=(void *)ats_co_RequireRunningMsgSetDB;
			break;
		case 105:
			handle->message = malloc(sizeof(ats_co_RequireModifyATRModeMsg));
			memset(handle->message, 0, sizeof(ats_co_RequireModifyATRModeMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToRequireModifyATRModeMsg;
			handle->setDBFunc=(void *)ats_co_RequireModifyATRModeMsgSetDB;
			break;
		case 106: //ats_co_LoadTimeTableMsg : need update inputtask db for atr
			handle->message = malloc(sizeof(ats_co_LoadTimeTableMsg));
			memset(handle->message, 0, sizeof(ats_co_LoadTimeTableMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToLoadTimeTableMsg;
			handle->setDBFunc = (void *) ats_co_LoadTimeTableMsgSetDB;
			break;
		case 107: //ats_co_CFepRequireCASDataMsg : need update inputtask db for atr and tmt
		  handle->message = malloc(sizeof(ats_co_CFepRequireCASDataMsg));
		  memset(handle->message, 0, sizeof(ats_co_CFepRequireCASDataMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToCFepRequireCASDataMsg;
		  handle->setDBFunc = (void *) ats_co_CFepRequireCASDataMsgSetDB;
		  break;
		case 108: //ats_co_AlarmMsg : alarm msg
		  handle->message = malloc(sizeof(ats_co_AlarmMsg));
		  memset(handle->message, 0, sizeof(ats_co_AlarmMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToAlarmMsg;
		  handle->setDBFunc = (void *) ats_co_AlarmMsgSetDB;
		  break;
		case 109: //ats_co_LCfepNetStatusMsg
		  handle->message = malloc(sizeof(ats_co_LCfepNetStatusMsg));
		  memset(handle->message, 0, sizeof(ats_co_LCfepNetStatusMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToLCfepNetStatusMsg;
		  handle->setDBFunc = (void *) ats_co_LCfepNetStatusMsgSetDB;
		  break;
		case 110: //ats_co_Interface_TrackStatusMsg(701)
		  handle->message = malloc(sizeof(ats_co_InterfaceTrackStatusMsg));
		  memset(handle->message, 0, sizeof(ats_co_InterfaceTrackStatusMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToInterfaceTrackStatusMsg;
		  handle->setDBFunc = (void *) ats_co_InterfaceTrackStatusMsgSetDB;
		  break;

		case 111: //ats_co_Interface_TrackStatusMsg(260)wh
		  handle->message = malloc(sizeof(ats_co_HmiToATRManualRegulationValue));
		  memset(handle->message, 0, sizeof(ats_co_HmiToATRManualRegulationValue));
		  handle->setDBFunc = (void *) ats_co_HmiToATRManualRegulationValueSetDB;
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToHmiToATRManualRegulationValue;
		break;
		case 112: //351
		  handle->message = malloc(sizeof(ats_co_InterfaceTTPMsg));
		  memset(handle->message, 0, sizeof(ats_co_InterfaceTTPMsg));
		  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToTTPMsg;
		  handle->setDBFunc = (void *) ats_co_TTPLoadTimeTableMsgSetDB;
		  break;
		case 113://23
			//	ILOG("113 msg");
			handle->message = malloc(sizeof(ats_co_TsControlRightAutoMsg));
			memset(handle->message, 0, sizeof(ats_co_TsControlRightAutoMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToControlRightAutoMsg;
			handle->setDBFunc = (void *) ats_co_ControlRightAutoMsgSetDB;
			//  ILOG("113 msg1");
			break;
		case 114:
			handle->message = malloc(sizeof(ats_co_CASBeatMsg));
			memset(handle->message, 0, sizeof(ats_co_CASBeatMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToCASBeatMsg;
			handle->setDBFunc = (void *) ats_co_CASBeatMsgSetDB;

			//  ILOG("114 msg1");
			break;
//		case 115:
//			handle->message = malloc(sizeof(ats_co_LoadOKmsg));
//			memset(handle->message, 0, sizeof(ats_co_LoadOKmsg));
//			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToLoadOKMsg;
//			handle->setDBFunc = (void *) ats_co_LoadOKMsgSetDB;
//			ILOG("115 msg1");
//			break;
		case 116:
			handle->message = malloc(sizeof(ats_co_RecieveLoadOKMsg));
			memset(handle->message, 0, sizeof(ats_co_RecieveLoadOKMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToRecieveTrainidRequireSynMsg;
			handle->setDBFunc = (void *) ats_co_RecieveTrainidSynRequireSetDB;
			break;
		case 117:
			handle->message = malloc(sizeof(ats_co_RecieveTrainidSynMsg));
			memset(handle->message, 0, sizeof(ats_co_RecieveTrainidSynMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToRecieveTrainidSynMsg;
			handle->setDBFunc = (void *) ats_co_RecieveTrainidSynSetDB;
			break;
		case 118: //205
			handle->message = malloc(sizeof(ats_co_InterfaceHMItoCASMsg));
			memset(handle->message, 0, sizeof(ats_co_InterfaceHMItoCASMsg));
			handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToHMItoCASMsg;
			handle->setDBFunc = (void *) ats_co_HMItoCASMsgSetDB;
			break;
		default:
			return false;
			break;
	}

if((handle->header.datatype==2)&&(handle->config.message_type==400))
{
//	ILOG("logicalfun:2,400");
	handle->logicalMsgFunc = (void *)ats_co_logicalMsgFunc;
}
return true;

}

/*******************************************************************
 Function name		ats_co_packNetHeader
 description			convert message header of net layer to bytes buffer
 parameter
 message_header_t *		IN	 		msg
 BYTE		*			OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_packNetHeader(message_header_t *msg, BYTE *buf)
{
if ((int2bytes(&buf[0], LONGLONG2BYTE, msg->timestamp)) != 0)
return -1;
if ((int2bytes(&buf[8], INT2BYTE, msg->seqno)) != 0)
return -1;
if ((int2bytes(&buf[12], CHAR2BYTE, msg->srcno)) != 0)
return -1;
if ((int2bytes(&buf[13], CHAR2BYTE, msg->destno)) != 0)
return -1;
if ((int2bytes(&buf[14], CHAR2BYTE, msg->srcmodule)) != 0)
return -1;
if ((int2bytes(&buf[15], CHAR2BYTE, msg->destmodule)) != 0)
return -1;
if ((int2bytes(&buf[16], SHT2BYTE, msg->datatype)) != 0)
return -1;
if ((int2bytes(&buf[18], CHAR2BYTE, msg->protocoltype)) != 0)
return -1;
if ((int2bytes(&buf[19], CHAR2BYTE, msg->mainorbak)) != 0)
return -1;
if ((int2bytes(&buf[20], SHT2BYTE, msg->remain)) != 0)
return -1;
if ((int2bytes(&buf[22], SHT2BYTE, msg->datalen)) != 0)
return -1;
return true;
}

/*******************************************************************
 Function name		ats_co_parseNetHeader
 description			get message data from bytes buffer
 type and element type
 parameter
 ATS_CO_MSG *		IN	 		handle
 BYTE	*			OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_parseNetHeader(ATS_CO_MSG *handle, const BYTE *buf)
{
const BYTE *po;

po = buf;
handle->header.timestamp = (u_int64) bytes2int(po, LONGLONG2BYTE);
po = po + 8;
handle->header.seqno = (u_int32) bytes2int(po, INT2BYTE);
po = po + 4;
handle->header.srcno = (unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.destno =(unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.srcmodule = (unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.destmodule = (unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.datatype = (u_int16) bytes2int(po, SHT2BYTE);
po = po + 2;
handle->header.protocoltype = (unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.mainorbak = (unsigned char) bytes2int(po, CHAR2BYTE);
po = po + 1;
handle->header.remain = (u_int16) bytes2int(po, SHT2BYTE);
po = po + 2;
handle->header.datalen = (u_int16) bytes2int(po, SHT2BYTE);

return true;
}


void byprint(const BYTE *bytes , int len)
{

	int i = 0;

	char *stream,line[80];

	stream=malloc(len*100);
	memset(stream,0,len*100);
	strcat(stream,"\n");

		while (i < len)
		{

		if(i%16==0)
		{
			strcat(stream,"\n");
			sprintf(line,"<%d>",i);
			strcat(stream,line);

			}

		if(i%8==0)
		{
			strcat(stream,"	");
		}
		sprintf(line,"[%02x]",bytes[i]);
		strcat(stream,line);

		    i++;

		}
		DLOG("<%s>%s",(char *)LOG_NAME_MPUMessage,(char *)stream);
		free(stream);

		return;

}

/*******************************************************************
 Function name		ats_mpu_unpackElementStatus
 description			unpack Element Status,call by PL
 parameter
 unsigned char * data
 int len
 Return value
 void
 *******************************************************************/
BYTE *pl_format=NULL;
int pl_len=0;
void  ats_mpu_unpackElementStatus(unsigned char * data, int len)
{
	DLOG("PL unpackElementStatus len=%d",len);
	pl_len = len;
	pl_format=NULL;
	pl_format = malloc(len);
	memset(pl_format, 0, len);
	memcpy(pl_format, data,len);
	ATS_CO_MSG *handle;
	handle = malloc(sizeof(ATS_CO_MSG));
	memset(handle,0,sizeof(ATS_CO_MSG));

	handle->fixed_len += len;
    handle->format = malloc(handle->fixed_len);
    memset(handle->format, 0, handle->fixed_len);
    memcpy(handle->format, data,handle->fixed_len);

    handle->bufferConvertToMsgFunc = NULL;
    handle->msgConvertToBufferFunc = NULL;
    handle->logicalMsgFunc = NULL;
    handle->setDBFunc = NULL;
    handle->getDBFunc = NULL;

     //allocate config struct

    if (handle->config.bufferConverToMsgFuncName == NULL)
    	handle->config.bufferConverToMsgFuncName = malloc(MAX_NAME_LEN);
    if (handle->config.msgConverToBufferFuncName == NULL )
    	handle->config.msgConverToBufferFuncName = malloc(MAX_NAME_LEN);
    if (handle->config.ats_co_setDBFuncName == NULL)
    	handle->config.ats_co_setDBFuncName = malloc(MAX_NAME_LEN);
    if (handle->config.ats_co_getDBFuncName == NULL)
    	handle->config.ats_co_getDBFuncName = malloc(MAX_NAME_LEN);
    //init header value
    handle->header.datatype=2;
    if (ats_co_initMessageValue(handle) == false)
    {
    	ELOG("unpackElementStatus error ");
    	return;
    }

    MPU* mpu=mpu_getMPUHandle();

    if(handle!=NULL&&mpu!=NULL) //netheader is ok
    {
	  //to lock RecevierPool mutex
	  mpuThreadSyncMutex_lock(&mpu->pMPUCenter->stReceiverPoolMutex);

	  //check pool is full
	  if(mpu->pMPUCenter->pReceiverPool->count>=POOL_SIZE)
	  {
		  ELOG("MPUReceiverPool is full,out of pool size,count=%d",
				 mpu->pMPUCenter->pReceiverPool->count);
		  mpuThreadSyncMutex_unlock(&mpu->log_mutex);
		  mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
	  }

	  if(handle->header.seqno != -1)//mean: if this msg is invalid,no need to put it into receivepool
	  {
		  mpuReceiverPool_MSG_put(mpu->pMPUCenter->pReceiverPool,handle);
		  //DLOG("receivepool count =%d",mpu->pMPUCenter->pReceiverPool->count);
	  }
	  else
	  {
		  ats_co_freeATSCOMessage(handle);
	  }
	  mpuThreadSyncMutex_unlock(&mpu->pMPUCenter->stReceiverPoolMutex);
	  DLOG("PL unpackElementStatus end");
	  //msglenCount += hdl->header.datalen + MSG_HEADER_LEN;

  }
}
/*******************************************************************
 Function name		ats_mpu_unpackCommandConfirm
 description			do nothing,call by PL
 parameter
 unsigned char * data
 int len
 Return value
 void
 *******************************************************************/
void  ats_mpu_unpackCommandConfirm(unsigned char *data , int len)
{
	return;
}



/*******************************************************************
 Function name		ats_co_initATSCOMessage
 description			initialize message handle totally
 parameter
 ATS_CO_MSG *		IN	 		handle
 const BYTE *		IN 			buf
 size_t 			IN 			len
 Return value
 true or false
 *******************************************************************/
int
ats_co_initATSCOMessage(ATS_CO_MSG *handle,  const BYTE *buf, size_t len)
{
 //buffer contains net header ,length = net header size + packet size		
 //to parse net header firstly	

 //allocate memory for format bytes
//	byprint(buf,len);
int result = ats_co_parseNetHeader(handle, buf);

//DLOG("<%s>pattern type(destno=%d,destmodule=%d,protocoltype=%d,result=%d)",
//		LOG_NAME_MPUMessage,handle->header.destno,handle->header.destmodule, handle->header.protocoltype,result);
if(result != 0)
	return ERROR_NET;

//20130325 sunjh  cas:150(source) - 1(destno)
if(handle->header.destno == 1)
{
	if(handle->header.protocoltype == 6)
	{
		DLOG("fepStatusType is close");
		fepStatusType = 0;
	}
	if(handle->header.protocoltype == 7)
	{
		DLOG("fepStatusType is open");
		fepStatusType = 1;
	}


	//20130328 sunjh
	if(fepStatusType == 0)
	{
		//2
		adapter_co_send_CASToHMIMessage(99,0,2);
	}
	else if(fepStatusType == 1)
	{
		//4
		adapter_co_send_CASToHMIMessage(99,0,4);
	}

}





if((handle->header.destmodule) > 200)
{
//	if((handle->header.protocoltype) == 7)
//    {
//		//printf("mpu recevie loadrequest from net!");
//		//DLOG("<%s>recevie loadrequest from net!",LOG_NAME_MPUMessage);
//		handle->header.datalen = 0;
//		adapter_co_send_loadRequest(handle->header.destno,handle->header.destmodule,1,1,1,1);
//    }
//    //DLOG("<%s>recevie loadrequest from net,send to TS sucess!",LOG_NAME_MPUMessage);
//	DLOG("<%s>recevie loadrequest from net,send to Fep sucess!",LOG_NAME_MPUMessage);
//	return false;
	ILOG("handle->header.destno:%d",handle->header.destno);
	//if(handle->header.destno >= 70 && (handle->header.destno) <= 73)
	if(1)
	{
	ats_co_FepStatusMsg message;
	memset(&message,0,sizeof(ats_co_FepStatusMsg));
	message.fepID = handle->header.destno;
	message.fepModule = handle->header.destmodule;
	message.fepStatus = 0;
	if(handle->header.protocoltype == 6)
	{
		message.fepStatus = 0;


//		ILOG("RECEIVE NET STATE IS :NO CONNECTION");
//		return false;//added by 2012.0815
	}
	if(handle->header.protocoltype == 7)
	{
		message.fepStatus = 1;


	}


//	ILOG("recevie loadrequest from net:FepID=%d,FepModel=%d,FepStatus=%d",
//	    		message.fepID, message.fepModule, message.fepStatus);
	handle->fixed_len = 6;
	BYTE fepbuf[handle->fixed_len];
	memset(fepbuf,0,handle->fixed_len);
	ats_co_FepStatusMsgConvertToBuffer(&message, fepbuf);
	handle->format = malloc(handle->fixed_len);
	memset(handle->format, 0, handle->fixed_len);
	memcpy(handle->format, fepbuf,handle->fixed_len);
	//ILOG("FepStatusMsg length:%d",handle->fixed_len);
	handle->config.index = 0;
//	adapter_co_send_loadRequest(message.fepID,message.fepModule,1,1,1,1);
//	ILOG("send loadrequest to fep:FepID=%d,FepModel=%d",
//	    				message.fepID, message.fepModule);
	}
//	else
//	{
//		ats_co_CASNetStatusMsg message;
//			message.fepID = handle->header.destno;
//			message.fepModule = handle->header.destmodule;
//			message.fepStatus = 0;
//			if(handle->header.protocoltype == 6)
//			{
//				message.fepStatus = 0;
//		//		ILOG("RECEIVE NET STATE IS :NO CONNECTION");
//		//		return false;//added by 2012.0815
//			}
//			if(handle->header.protocoltype == 7)
//				message.fepStatus = 1;
//		//	ILOG("recevie loadrequest from net:FepID=%d,FepModel=%d,FepStatus=%d",
//		//	    		message.fepID, message.fepModule, message.fepStatus);
//			handle->fixed_len = 6;
//			BYTE fepbuf[handle->fixed_len];
//			ats_co_CASNetStatusMsgConvertToBuffer(&message, fepbuf);
//			handle->format = malloc(handle->fixed_len);
//			memset(handle->format, 0, handle->fixed_len);
//			memcpy(handle->format, fepbuf,handle->fixed_len);
//			handle->config.index = -1;
//	}
}

if(handle->header.datalen == 0 && (handle->header.destmodule) <= 200)
	return false;
//if(handle->header.datatype==2)
//{
//    handle->fixed_len = (int) bytes2int(buf + 3 + MSG_HEADER_LEN,CHAR2BYTE);
//    if(handle->fixed_len>=BUFFER_SIZE)
//    {
//    	ELOG("<%s>here error 1",LOG_NAME_MPUMessage);
//		return false;
//    }
//
//}
//else if(handle->header.datatype==1)
//{
//	handle->fixed_len = (int) bytes2int(buf + 2 + MSG_HEADER_LEN,INT2BYTE)+6;
//	if(handle->fixed_len>=BUFFER_SIZE)
//	{
//		ELOG("<%s>here error 2",LOG_NAME_MPUMessage);
//		return false;
//	}
//}
//else
//{
//	ELOG("<%s>here error 3, datatype=%d",LOG_NAME_MPUMessage,handle->header.datatype);
//	return false;
//}
if((handle->header.destmodule) <= 200)
{
handle->fixed_len = (int) handle->header.datalen;
if(handle->fixed_len > 0)
{
//ILOG("handle->fixed_len=%d",handle->fixed_len);
int old_fixed_len = handle->fixed_len;
handle->fixed_len += sizeof(BYTE);
handle->format = malloc(handle->fixed_len);
memset(handle->format, 0, handle->fixed_len);
memcpy(handle->format, buf + MSG_HEADER_LEN,old_fixed_len);
memcpy(handle->format + old_fixed_len, &(handle->header.srcno),sizeof(BYTE));
//ILOG("handle->fixed_len=%d",handle->fixed_len);
//ILOG("handle->header.srcno=%d",handle->header.srcno);
}
}
 //set all functions pointer to NULL
handle->bufferConvertToMsgFunc = NULL;
handle->msgConvertToBufferFunc = NULL;
handle->logicalMsgFunc = NULL;
handle->setDBFunc = NULL;
handle->getDBFunc = NULL;

 //allocate config struct

if (handle->config.bufferConverToMsgFuncName == NULL
)
handle->config.bufferConverToMsgFuncName = malloc(MAX_NAME_LEN);
if (handle->config.msgConverToBufferFuncName == NULL
)
handle->config.msgConverToBufferFuncName = malloc(MAX_NAME_LEN);
if (handle->config.ats_co_setDBFuncName == NULL
)
handle->config.ats_co_setDBFuncName = malloc(MAX_NAME_LEN);
if (handle->config.ats_co_getDBFuncName == NULL
)
handle->config.ats_co_getDBFuncName = malloc(MAX_NAME_LEN);

 //According  to identifier (here type and element) of packet ,
 //we must decide to how to set name and structure of conversion function.
 //on the other hand, also to allocate memory for message handle which used type
if(handle->fixed_len > 0)
{
	if (ats_co_initMessageValue(handle) == false)
    {
		//ELOG("<%s>here error 4",LOG_NAME_MPUMessage);
		return false;
    }
//	else
//	{
//		//if init pass,then call logicalFunc(update replaydb)
//		if((handle->header.datatype==2)&&(handle->config.message_type==400))
//		{
//			ILOG("logicalfun:2,400");
//			handle->logicalMsgFunc = (void *)ats_co_logicalMsgFunc;
//		}
//	}
}
 //here we have not allocated memory for handle->message 		
 //so when we do it ? it's a void pointer really.
return true;
}

/*******************************************************************
 Function name		ats_co_freeATSCOMessage
 description			free message handle
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_freeATSCOMessage(ATS_CO_MSG *handle)
{
 //free member's memory
if (handle->format != NULL
)
free(handle->format);


 //free message contain	
if (handle->message != NULL)
free(handle->message);

 //free handle of calling functions 									
 //free handle
if (handle->config.bufferConverToMsgFuncName != NULL)
free(handle->config.bufferConverToMsgFuncName);
if (handle->config.msgConverToBufferFuncName != NULL)
free(handle->config.msgConverToBufferFuncName);
if (handle->config.ats_co_setDBFuncName != NULL)
free(handle->config.ats_co_setDBFuncName);
if (handle->config.ats_co_getDBFuncName != NULL)
free(handle->config.ats_co_getDBFuncName);


if (handle != NULL) free(handle);


return true;
}

int ats_co_logMsgHeader(ATS_CO_MSG *handle)
{
	DLOG("<%s>dataLen=%d,dataType=%d,sequence=%d,protocolType=%d,timestamp=%lld",LOG_NAME_MPUMessage,
	handle->header.datalen,handle->header.datatype,handle->header.seqno,handle->header.protocoltype,handle->header.timestamp);

	return true;
}


/*******************************************************************
 Function name		ats_co_logATSCOMessage
 description
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_logATSCOMessage(ATS_CO_MSG *handle)
{
 //ats_co_TrackMsg *msg;
 //msg=malloc(sizeof(ats_co_TrackMsg));	

	ats_co_logMsgHeader(handle);


switch (handle->config.index)
{
case 1: //ats_co_TrackMsg
//ats_co_logTrackMsg(handle->message);
break;
case 2:
break;
case 3:
break;
case 4:
break;
case 5:
break;
case 6:
break;
case 7:
break;
default:
break;
}
return true;
}




/*******************************************************************
}
 Function name		ats_co_unpackATSCOMessage
 description			get message data from formated buffer contained by handle
 parameter
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_unpackATSCOMessage(ATS_CO_MSG *handle)
{
 //the task is to parse format message to message structure by calling parse function
 //we also know that message have allocated memory at that time .it's important.
if(handle!=NULL && handle->bufferConvertToMsgFunc!=NULL)
handle->bufferConvertToMsgFunc(handle->format, handle->message);

//modified by qujun 2012-12-26
if(handle->config.index == 101 ) //internal MSG 204
{
	((ats_co_TrainIDModify*)handle->message)->machineNo = handle->header.srcno;
    DLOG("204msg machineNO : %d,%d,%d",((ats_co_TrainIDModify*)handle->message)->machineNo,handle->header.srcno,handle->header.destno);
}
else if(handle->config.index == 100 ) //internal MSG 201
{
	((ats_co_ALLMsgUpdate*)handle->message)->hmiNumber = handle->header.srcno;
}


//error test  ????
return true;
}

/*******************************************************************
 Function name		ats_co_refreshDBATSCOMessage
 description			refresh data of message contained by handle to db
 parameter
 sqlite3			IN			db
 ATS_CO_MSG *		IN	 		handle
 Return value
 true or false
 *******************************************************************/
int
ats_co_refreshDBATSCOMessage(sqlite3 *db, ATS_CO_MSG *handle)
{
 //functions for database accessing 
handle->setDBFunc(db, handle->message);
return true;
}

int ats_co_logicalMsgFunc(sqlite3 *db, ATS_CO_MSG *handle)
{
//	InnDataForTmt *x;
//	ats_co_TimeTableLoad *y;
//
//	ats_co_TrainEventMsg *train_event;
//	ats_co_TrainPositionReport *train_posii_event;
//
//	switch (handle->config.index)
//	{
//
//
//	//case 201 :
//	case 100:
//		//
//		x=(InnDataForTmt *)handle->message;
//		tmt_io_inn_UpdateProcess(x);
//		break;
//	//case 204:
//	case 101:
//		x=(InnDataForTmt *)handle->message;
//		tmt_io_inn_UpdateProcess(x);
//		break;
//	//case 253:
//	case 102:
//			y=(ats_co_TimeTableLoad *)handle->message;
//			DLOG("���������������������������������������������������������������");
//			ats_atr_FromHMIProcess(y);
//			//DLOG("���������������������������������������������������������������2");
//			break;
//
//
//	case 3:
//	//case 434:
//
//			train_event=(ats_co_TrainEventMsg *)handle->message;
//			//tmt_io_inn_UpdateProcess(x);
//			DateTime *x;
//
//			x=&(train_event->eventTime);
//			datetime_conversion(x);
//			trainEventToAtr(train_event);
//
//			break;
//
//	case 10:
//
//		train_posii_event=(ats_co_TrainPositionReport *)handle->message;
//					//tmt_io_inn_UpdateProcess(x);
//		//trainPositionEventToHmi(train_posii_event);
//
//			break;
//
//	}

//	ILOG("logicalfun1");
	if((handle->header.datatype==2)&&(handle->config.message_type==400))
	{
		ats_co_Update_Interlock_for_repalydb(db, handle);
	}

	return true;
}





int ats_co_BufferConvertToFepStatusMsg(const BYTE buf[] ,ats_co_FepStatusMsg *message)
{
	message->fepID = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	if(message->fepID<=0 || message->fepID >=1000)
		return -1;
	message->fepModule = (u_int32)bytes2int(&buf[2],SHT2BYTE);
	message->fepStatus = (u_int32)bytes2int(&buf[4],SHT2BYTE);
//	ILOG("buffer to fepID=%d",message->fepID);
//	ILOG("buffer to fepModule=%d",message->fepModule);
//	ILOG("buffer to fepModule=%d",message->fepModule);
	return 0;
}

int ats_co_FepStatusMsgConvertToBuffer(ats_co_FepStatusMsg *message, BYTE buf[])
{
	if (int2bytes(&buf[0], SHT2BYTE, message->fepID) != 0)
	{
		ILOG("fepID to buffer error1");
		return -1;
	}
	if (int2bytes(&buf[2], SHT2BYTE, message->fepModule) != 0)
	{
		ILOG("fepID to buffer error2");
	return -1;
	}
	if (int2bytes(&buf[4], SHT2BYTE, message->fepStatus) != 0)
	{
		ILOG("fepID to buffer error3");
	return -1;
	}
	return 0;
}

/*******************************************************************
 Function name		ats_co_BufferConvertToTrackMsg
 description         convert bytes buffer to track message. for circuit, track
                     is circuit track; for cbtc,track is axle.
 parameter
 BYTE	[]						IN				buf
 ats_co_TrackMsg *				OUT 			message
 Return value
 true or false
 *******************************************************************/
//TrackMsg������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToTrackMsg(const BYTE buf[], ats_co_TrackMsg *message)
{
    //now, have know the type of message structure, we can allocate memory for this pointer.
	if(PROJECT_MODE == ITC_HEB1)
	{
		message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
		message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
		message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
		message->counter = bytes2int(&buf[4], SHT2BYTE);
		message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
		message->trackid = (u_int32) bytes2int(&buf[7], SHT2BYTE);
		//message->direction = countValueofBits(buf[12], 6, 2);
		message->lockFlag = countValueofBits(buf[11], 6, 1);
		message->freeStatus = countValueofBits(buf[11], 0, 2);
		//message->status = countValueofBits(buf[22], 3, 3);
	    //message->physicalFree = countValueofBits(buf[11], 4, 1);
		message->leftClaim = countValueofBits(buf[11], 2, 1);
		message->rightClaim = countValueofBits(buf[11], 3, 1);
		return 0;
	}
	else if(PROJECT_MODE == CBTC)
	{
		//

	}
	else
	{
		return -1;
	}

}
/*******************************************************************
 Function name		ats_co_BufferConvertToLogicalTrackMsg
 description         convert bytes buffer to logical track message,
                     added by lfc 20121211
 parameter
 BYTE	[]						IN				buf
 ats_co_TrackMsg *				OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToLogicalTrackMsg(const BYTE buf[], ats_co_logicalTrackMsg *message)
{
 //now, have know the type of message structure, we can allocate memory for this pointer.
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->trackid = (u_int32) bytes2int(&buf[7], SHT2BYTE);
message->direction = countValueofBits(buf[12], 6, 2);      //modified by lfc 201201211
message->lockFlag = countValueofBits(buf[11], 3, 1);       //modified by lfc 201201211
message->freeStatus = countValueofBits(buf[11], 2, 1);     //modified by lfc 201201211
message->atpVacancy = countValueofBits(buf[22], 3, 3);     //modified by lfc 201201211

//modified by lfc 201201211 10 0 1-->10 7 1
message->atpAvailable = countValueofBits(buf[10], 7, 1);
message->TVDFailure=countValueofBits(buf[22], 0, 2);
return 0;
}


/*******************************************************************
 Function name			ats_co_TrackMsgConvertToBuffer
 description            convert logical track message to bytes buffer
 parameter
 ats_co_TrackMsg *		IN	 		message
 BYTE	[]					OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_TrackMsgConvertToBuffer(ats_co_TrackMsg *message, BYTE buf[])
{
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->trackid) != 0)
return -1;
//if (setBitsOfByte(&buf[12], 6, 2, message->direction) != 0)
//return -1;
if (setBitsOfByte(&buf[11], 6, 1, message->lockFlag) != 0)
return -1;
if (setBitsOfByte(&buf[11], 0, 2, message->freeStatus) != 0)
return -1;
//if (setBitsOfByte(&buf[22], 3, 3, message->status) != 0)
//return -1;
if (setBitsOfByte(&buf[10], 0, 1, message->atpAvailable) != 0)
return -1;
return 0;
}



/*******************************************************************
 Function name			ats_co_BufferConvertToTrainStatusMsg
 description            convert bytes buffer to train status message
 parameter
 BYTE	[]						IN				buf
 ats_co_TrainStatusMsg *		OUT 			message
 Return value
 true or false
 *******************************************************************/
//TrainStatusMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToTrainStatusMsg(const BYTE buf[],
ats_co_TrainStatusMsg *message)
{

	BYTE buftemp[2];
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->trainIndex = (u_int32) bytes2int(&buf[7], SHT2BYTE);
message->trainDoorStatus = countValueofBits(buf[12], 0, 2);
message->ebIsApplied = countValueofBits(buf[13], 0, 1);//
//message->ebReason = countValueofBits(buf[14], 0, 3);
message->controlLevel = countValueofBits(buf[15], 0, 2);//
message->driveMode = countValueofBits(buf[16], 0, 3);
//message->obcuFailure = countValueofBits(buf[22], 1, 1);
message->destination = (u_int32) bytes2int(&buf[36], SHT2BYTE);
message->service = (u_int32) bytes2int(&buf[38], SHT2BYTE);//
buftemp[0] = buf[40];
buftemp[1] = buf[46];
message->serial = (u_int32) bytes2int(buftemp, SHT2BYTE); //
//message->crew = (u_int32) bytes2int(&buf[41], SHT2BYTE);
message->line = (u_int32) bytes2int(&buf[43], CHAR2BYTE);
message->car = (u_int32) bytes2int(&buf[44], SHT2BYTE);

//add by quj 2012-1213
message->CommuEstablished=countValueofBits(buf[11], 0, 1);
message->DstNumber=(u_int32) bytes2int(&buf[36], SHT2BYTE);
message->driver=(u_int32) bytes2int(&buf[41], SHT2BYTE);
message->car1=(u_int32) bytes2int(&buf[44], SHT2BYTE);
message->ebReason=(u_int32) bytes2int(&buf[14], CHAR2BYTE);
message->obcuFailure = countValueofBits(buf[22], 0, 1);
ILOG("STATUS MESSAGE:message->car1=%d,message->driver=%d",message->car1,message->driver);

return 0;
}

/*******************************************************************
 Function name			ats_co_TrainStatusMsgConvertToBuffer
 description            convert train status  message to bytes buffer
 parameter
 ats_co_TrainStatusMsg *		IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_TrainStatusMsgConvertToBuffer(ats_co_TrainStatusMsg *message, BYTE buf[])
{
	BYTE buftemp[2];
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->trainIndex) != 0)
return -1;
if (setBitsOfByte(&buf[12], 0, 2, message->trainDoorStatus) != 0)
return -1;
if (setBitsOfByte(&buf[13], 0, 1, message->ebIsApplied) != 0)
return -1;
if (setBitsOfByte(&buf[14], 0, 3, message->ebReason) != 0)
return -1;
if (setBitsOfByte(&buf[15], 0, 2, message->controlLevel) != 0)
return -1;
if (setBitsOfByte(&buf[16], 0, 3, message->driveMode) != 0)
return -1;
if (setBitsOfByte(&buf[22], 1, 1, message->obcuFailure) != 0)
return -1;
if (int2bytes(&buf[36], SHT2BYTE, message->destination) != 0)
return -1;
if (int2bytes(&buf[38], SHT2BYTE, message->service) != 0)
return -1;
if (int2bytes(buftemp, SHT2BYTE, message->serial) != 0)
return -1;
buf[40] = buftemp[0];
buf[46] = buftemp[1];
if (int2bytes(&buf[41], SHT2BYTE, message->crew) != 0)
return -1;
if (int2bytes(&buf[43], CHAR2BYTE, message->line) != 0)
return -1;
if (int2bytes(&buf[44], SHT2BYTE, message->car) != 0)
return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToTrainEventMsg
 description            convert bytes buffer to train event message
 parameter
 BYTE	[]						IN				buf
 ats_co_TrainEventMsg *			OUT 			message
 Return value
 true or false
 *******************************************************************/
//TrainEventMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToTrainEventMsg(const BYTE buf[],ats_co_TrainEventMsg *message)
{
uint64_t devtime;
struct tm t;
time_t mytime ;
struct tm* now = NULL;
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->trainIndex = (u_int32) bytes2int(&buf[7], SHT2BYTE);
message->eventTime.minute = (u_int32) bytes2int(&buf[9], CHAR2BYTE);
message->eventTime.hour = (u_int32) bytes2int(&buf[10], CHAR2BYTE);
message->eventTime.day = (u_int32) bytes2int(&buf[11], CHAR2BYTE);
message->eventTime.month = (u_int32) bytes2int(&buf[12], CHAR2BYTE);
message->eventTime.year = (u_int32) bytes2int(&buf[13], CHAR2BYTE);
message->eventType = (u_int32) bytes2int(&buf[14], SHT2BYTE);
message->eventCause = (u_int32) bytes2int(&buf[16], SHT2BYTE);
message->PTID = (u_int32) bytes2int(&buf[18], SHT2BYTE);
//
if(message->eventTime.minute==255)
{
	// time span
	//message->eventTime=
	devtime=message->eventTime.hour+message->eventTime.day*256+message->eventTime.month*256*256+message->eventTime.year*256*256*256;

		t.tm_year = 2000 - 1900;
		t.tm_mon = 1 - 1;
		t.tm_mday = 1;
		t.tm_hour = 0;
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_isdst = -1;

		mytime = mktime(&t) + devtime;

		if(mytime != -1)
		{
			now = localtime(&mytime);
			message->eventTime.second = now -> tm_sec;
			message->eventTime.minute = now -> tm_min;
			message->eventTime.hour = now -> tm_hour;
			message->eventTime.day = now -> tm_mday;
			message->eventTime.month = now -> tm_mon + 1;
			message->eventTime.year = now -> tm_year + 1900;

		}
}
else
{
	//BCD convert, max year=99  add by qjun
	//message->eventTime=
	message->eventTime.minute = ((char)message->eventTime.minute & 15)+((char)message->eventTime.minute & 240)*10/16;
	message->eventTime.hour = ((char)message->eventTime.hour & 15)+((char)message->eventTime.hour & 240)*10/16;
	message->eventTime.day = ((char)message->eventTime.day & 15)+((char)message->eventTime.day & 240)*10/16;
	message->eventTime.month = ((char)message->eventTime.month & 15)+((char)message->eventTime.month & 240)*10/16;
	message->eventTime.year = ((char)message->eventTime.year & 15)+((char)message->eventTime.year & 240)*10/16;

}


return 0;
}

/*
void ats_co_convert_event_time(DateTime eventTime, int devtime)
{
	char time[20]={'\0'};
	struct tm t;
	time_t mytime ;
	struct tm* now = NULL;

	t.tm_year = eventTime.year - 1900;
	t.tm_mon = eventTime.month - 1;
	t.tm_mday = eventTime.day;
	t.tm_hour = eventTime.hour;
	t.tm_min = eventTime.minute;
	t.tm_sec = eventTime.second;
	t.tm_isdst = -1;

	mytime = mktime(&t) + devtime;

	if(mytime != -1)
	{
		now = localtime(&mytime);

		sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d"
		             , now -> tm_year + 1900
		             , now -> tm_mon + 1
		             , now -> tm_mday
		             , now -> tm_hour
		             , now -> tm_min
		             , now -> tm_sec);

		printf("%s\n",time);

	}
}
*/
/*******************************************************************
 Function name			ats_co_TrainEventMsgConvertToBuffer
 description            convert train event message to bytes buffer
 parameter
 ats_co_TrainEventMsg *			IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_TrainEventMsgConvertToBuffer(ats_co_TrainEventMsg *message, BYTE buf[])
{

if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->trainIndex) != 0)
return -1;
if (int2bytes(&buf[9], CHAR2BYTE, message->eventTime.minute) != 0)
return -1;
if (int2bytes(&buf[10], CHAR2BYTE, message->eventTime.hour) != 0)
return -1;
if (int2bytes(&buf[11], CHAR2BYTE, message->eventTime.day) != 0)
return -1;
if (int2bytes(&buf[12], CHAR2BYTE, message->eventTime.month) != 0)
return -1;
if (int2bytes(&buf[13], CHAR2BYTE, message->eventTime.year) != 0)
return -1;
if (int2bytes(&buf[14], SHT2BYTE, message->eventType) != 0)
return -1;
if (int2bytes(&buf[16], SHT2BYTE, message->eventCause) != 0)
return -1;
if (int2bytes(&buf[18], SHT2BYTE, message->PTID) != 0)
return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToPlatformMsg
 description            convert bytes buffer to platform message
 parameter
 BYTE	[]						IN		case 1000: //ats_co_CASNetStatusMsg
  DLOG("ats_co_CASNetStatusMsg");
  handle->message = malloc(sizeof(ats_co_CASNetStatusMsg));
  memset(handle->message, 0, sizeof(ats_co_CASNetStatusMsg));
  handle->bufferConvertToMsgFunc = (void *) ats_co_BufferConvertToCASNetStatusMsg;
  DLOG("ats_co_CASNetStatusMsg2");
  handle->msgConvertToBufferFunc = (void *) ats_co_CASNetStatusMsgConvertToBuffer;
  DLOG("ats_co_CASNetStatusMsg3");
  handle->setDBFunc = (void *) ats_co_CASNetStatusMsgSetDB;
  DLOG("ats_co_CASNetStatusMsg4");		buf
 ats_co_PlatformMsg *			OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToPlatformMsg(const BYTE buf[], ats_co_PlatformMsg *message)
{

message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->platform = (u_int32) bytes2int(&buf[7], SHT2BYTE);
message->enforceSkipOrHold = countValueofBits(buf[20], 4, 3);
message->autoSkip = countValueofBits(buf[21], 7, 1);
return 0;
}

/*******************************************************************
 Function name			ats_co_PlatformMsgConvertToBuffer
 description            convert platform message to bytes buffer
 parameter
 ats_co_PlatformMsg *			IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_PlatformMsgConvertToBuffer(ats_co_PlatformMsg *message, BYTE buf[])
{
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->platform) != 0)
return -1;
if (setBitsOfByte(&buf[20], 4, 2, message->enforceSkipOrHold) != 0)
return -1;
if (setBitsOfByte(&buf[21], 7, 1, message->autoSkip) != 0)
return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToSwitchMsg
 description            convert bytes buffer to switch message
 parameter
 BYTE	[]						IN				buf
 ats_co_SwitchMsg *				OUT 			message
 Return value
 true or false
 *******************************************************************/
//SwitchMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToSwitchMsg(const BYTE buf[], ats_co_SwitchMsg*message)
{
    message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->switchId = (u_int32) bytes2int(&buf[7], SHT2BYTE);

	message->pointTrailed = countValueofBits(buf[11], 0, 1);           //
	message->blockAgainstThrowing = countValueofBits(buf[12], 2, 1);
	message->lockFlag = countValueofBits(buf[12], 4, 1);
	message->physicalFree = countValueofBits(buf[12], 5, 1);
	message->freeStatus = countValueofBits(buf[11], 7, 1);
	message->supervision = countValueofBits(buf[12], 1, 1);           //
	message->position = countValueofBits(buf[11], 6, 1);          //
	if(PROJECT_MODE == ITC_HEB1)
	{
		//message->positionFlag=countValueofBits(buf[11],2,1);
		message->claimStatus = countValueofBits(buf[11], 3, 2);
		return 0;

	}
	else if(PROJECT_MODE == CBTC)
	{

			message->kickoffFailure= countValueofBits(buf[11], 5, 1);
			//add by zs 2012-3-15
			message->atpAvailable = countValueofBits(buf[10], 7, 1);
			message->atpVacancy = countValueofBits(buf[22], 3, 3);
			message->overlapClaiming=countValueofBits(buf[11], 1, 2);    //
			message->pointDirection=countValueofBits(buf[14], 3, 2);
			message->TVDFailure=countValueofBits(buf[22], 0, 2);
			message->routeClaim = countValueofBits(buf[11], 3, 2);    //

			//added by lfc 20121211,tranfer out telegram attributes to inner attributes
            //Convert positon+pointTrailed+supervision to positionflag
			if (message->pointTrailed == 1)
			{
				message->positionFlag = 3;
			}
			else if (message->pointTrailed == 0)
			{
				if (message->supervision == 1)
				{
					if (message->position == 1)
					{
						message->positionFlag = 1;
					}
					else if(message->position == 1)
					{
						message->positionFlag = 2;
					}
					else
					{
					}
				}
				else
		        {
					message->positionFlag = 0;
		        }
			}
			else
			{
			}

			//Convert route Claim+ overlap claim to claimStatus
			if(message->routeClaim==1 || message->routeClaim==2)
			{
				message->claimStatus=1;
			}
			else if(message->routeClaim==0)
			{
				if(message->overlapClaiming==1 || message->overlapClaiming==2)
				{
					message->claimStatus=2;
				}
				else if(message->overlapClaiming==0)
				{
					message->claimStatus=0;
				}

			}
			return 0;
	}

	else
	{
		return -1;
	}

}
/*******************************************************************
 Function name			ats_co_SwitchMsgConvertToBuffer
 description            convert switch message to bytes buffer
 parameter
 ats_co_SwitchMsg *		IN	 		message
 BYTE	[]					OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_SwitchMsgConvertToBuffer(ats_co_SwitchMsg *message, BYTE buf[])
{

if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->switchId) != 0)
return -1;
if (setBitsOfByte(&buf[11], 3, 2, message->claimStatus) != 0)
return -1;
if (setBitsOfByte(&buf[12], 2, 1, message->blockAgainstThrowing) != 0)
return -1;
if (setBitsOfByte(&buf[12], 4, 1, message->lockFlag) != 0)
return -1;
if (setBitsOfByte(&buf[11], 7, 1, message->freeStatus) != 0)
return -1;
if (setBitsOfByte(&buf[11], 0, 1, message->pointTrailed) != 0)
return -1;
if (setBitsOfByte(&buf[12], 1, 1, message->supervision) != 0)
return -1;
if (setBitsOfByte(&buf[11], 6, 1, message->position) != 0)
return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToSignalMsg
 description            convert bytes buffer to signal message
 parameter
 BYTE	[]						IN				buf
 ats_co_SignalMsg *				OUT 			message
 Return value
 true or false
 *******************************************************************/
//SignalMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToSignalMsg(const BYTE buf[], ats_co_SignalMsg *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->signalId = (u_int32) bytes2int(&buf[7], SHT2BYTE); //

	message->signalStatus = countValueofBits(buf[11], 4, 3);//
	message->signalAutoStatus = countValueofBits(buf[12], 2, 2);///
	message->lockFlag = countValueofBits(buf[12], 1, 1);//
	if(PROJECT_MODE == ITC_HEB1)
	{

	}
	else if(PROJECT_MODE == CBTC)
	{
		message->signalFault=countValueofBits(buf[13], 4, 1);
	}
	else
	{}
	return 0;
}

/*******************************************************************
 Function name			ats_co_SignalMsgConvertToBuffer
 description            convert signal message to bytes buffer
 parameter
 ats_co_SignalMsg *				IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_SignalMsgConvertToBuffer(ats_co_SignalMsg *message, BYTE buf[])
{
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->signalId) != 0)
return -1;
if (setBitsOfByte(&buf[11], 4, 3, message->signalStatus) != 0)
return -1;
if (setBitsOfByte(&buf[12], 2, 2, message->signalAutoStatus) != 0)
return -1;
if (setBitsOfByte(&buf[12], 1, 1, message->lockFlag) != 0)
return -1;
//if (setBitsOfByte(&buf[13], 4, 1, message->signalFault) != 0)
//return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToAxleCounterMsg
 description            convert bytes buffer to axle counter message
 parameter
 BYTE	[]						IN				buf
 ats_co_AxleCounterMsg *	OUT 			message
 Return value
 true or false
 *******************************************************************/
//AxleCounterMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToAxleCounterMsg(const BYTE buf[],
ats_co_AxleCounterMsg *message)
{
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->axleCounterId = bytes2int(&buf[7], SHT2BYTE);
message->axleCounterFault = countValueofBits(buf[20], 0, 2);
message->freeStatus = countValueofBits(buf[11], 0, 2);
return 0;
}

/*******************************************************************
 Function name			ats_co_AxleCounterMsgConvertToBuffer
 description            convert axle counter message to bytes buffer
 parameter
 ats_co_AxleCounterMsg *	IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_AxleCounterMsgConvertToBuffer(ats_co_AxleCounterMsg *message, BYTE buf[])
{
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->axleCounterId) != 0)
return -1;
if (setBitsOfByte(&buf[20], 0, 2, message->axleCounterFault) != 0)
return -1;
if (setBitsOfByte(&buf[11], 0, 2, message->freeStatus) != 0)
return -1;
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToCrossingMsg
 description            convert bytes buffer to crossing message,only for cbtc
 parameter
 BYTE	[]						IN				buf
 ats_co_CrossingMsg *		OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToCrossingMsg(const BYTE buf[], ats_co_CrossingMsg *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->crossingIndex = bytes2int(&buf[7], SHT2BYTE);

	message->freeStatus = countValueofBits(buf[11], 5, 1);
	message->overlapClaim = countValueofBits(buf[11], 0, 2);
	message->routeClaim = countValueofBits(buf[11], 2, 2);
	message->failureIndication = countValueofBits(buf[11], 5, 1);
	message->lockAgainstPassing = countValueofBits(buf[11], 6, 1);
	message->blockAgainstThrowing=countValueofBits(buf[12], 2, 1);
	message->position = countValueofBits(buf[12], 5, 1);
	message->direction = countValueofBits(buf[14], 2, 2);
	message->TVDFailure=countValueofBits(buf[22], 0, 2);
	message->ATPVacancy = countValueofBits(buf[22], 3, 3);
	message->atpAvailable=countValueofBits(buf[10], 7, 1);

return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToAreaStatusMsg
 description            convert bytes buffer to area status message
 parameter
 BYTE	[]						IN				buf
 ats_co_AreaStatusMsg *		OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToAreaStatusMsg(const BYTE buf[],
ats_co_AreaStatusMsg *message)
{
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->areaStatusIndex = bytes2int(&buf[7], SHT2BYTE);
message->operatingMode = countValueofBits(buf[9], 0, 3);
message->fault = countValueofBits(buf[9], 4, 1);
message->CommF = countValueofBits(buf[9], 5, 1);
message->SDA = countValueofBits(buf[9], 7, 1);
message->ATPflag = countValueofBits(buf[10], 0, 1);
message->failureInformation = countValueofBits(buf[10], 1, 6);
message->interLocking = countValueofBits(buf[10], 7, 1);
message->areaBlockFlag = countValueofBits(buf[11], 0, 1);
message->reduceNoiseFlag = countValueofBits(buf[11], 1, 1);
message->allowNearFlag = countValueofBits(buf[11], 2, 1);
return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToTrainPositionReport
 description            convert train position report to bytes buffer
 parameter
 ats_co_TrainPositionReport *		IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToTrainPositionReport(const BYTE buf[],
ats_co_TrainPositionReport *message)
{
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->trainIndex = bytes2int(&buf[7], SHT2BYTE);
message->operatingMode = countValueofBits(buf[9], 0, 3);
message->fault = countValueofBits(buf[9], 4, 1);
message->CommF = countValueofBits(buf[9], 5, 1);
message->SDA = countValueofBits(buf[9], 7, 1);
message->ATPflag = countValueofBits(buf[10], 0, 1);
message->failureInformation = countValueofBits(buf[10], 1, 6);
message->interLocking = countValueofBits(buf[10], 7, 1);

message->LoclsSecured = countValueofBits(buf[11], 0, 1);

message->trainPositionFlag = countValueofBits(buf[11], 4, 1);
message->locIntegrityIsOK = (u_int32) bytes2int(&buf[14], CHAR2BYTE);
message->headSegmentID = bytes2int(&buf[15], SHT2BYTE);
message->headOffset = bytes2int(&buf[17], INT2BYTE);
message->trainDirection = countValueofBits(buf[21], 0, 2);
message->tailSegmentID = bytes2int(&buf[22], SHT2BYTE);
message->tailOffset = bytes2int(&buf[24], INT2BYTE);
message->operationCtrLevel = countValueofBits(buf[29], 0, 2);
message->trainDriveMode = countValueofBits(buf[30], 0, 3);
message->trainSpeed = bytes2int(&buf[34], SHT2BYTE);
//add by quj 20121213
message->OPRValidFlag=countValueofBits(buf[11], 4, 1);
message->FrontSegID=bytes2int(&buf[15], SHT2BYTE);
message->FrontOffset=bytes2int(&buf[17], INT2BYTE);
message->ControlLevel=countValueofBits(buf[29], 0, 2);
message->OperationMode=countValueofBits(buf[30], 0, 3);
message->carSpeed=bytes2int(&buf[34], SHT2BYTE);
message->FrontTravelDir=countValueofBits(buf[21], 0, 2);
message->TrainStandStill=countValueofBits(buf[36], 0, 2);

return 0;
}

int
ats_co_TrainPositionReportConvertToBuffer(ats_co_TrainPositionReport *message, BYTE buf[])
{
	if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
	return -1;
	if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
	return -1;
	if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
	return -1;
	if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
	return -1;
	if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
	return -1;
	if (int2bytes(&buf[7], SHT2BYTE, message->trainIndex) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 0, 3, message->operatingMode) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 4, 1, message->fault) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 5, 1, message->CommF) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 7, 1, message->SDA) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 0, 1, message->ATPflag) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 1, 6, message->failureInformation) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 7, 1, message->interLocking) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 0, 1, message->locSecurityFlag) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 4, 1, message->trainPositionFlag) != 0)
	return -1;
	if (int2bytes(&buf[14], CHAR2BYTE, message->locIntegrityIsOK) != 0)
	return -1;
	if (int2bytes(&buf[15], SHT2BYTE, message->headSegmentID) != 0)
	return -1;
	if (int2bytes(&buf[17], INT2BYTE, message->headOffset) != 0)
	return -1;
	if (setBitsOfByte(&buf[21], 0, 2, message->trainDirection) != 0)
	return -1;
	if (int2bytes(&buf[22], SHT2BYTE, message->tailSegmentID) != 0)
	return -1;
	if (int2bytes(&buf[24], INT2BYTE, message->tailOffset) != 0)
	return -1;
	if (setBitsOfByte(&buf[29], 0, 2, message->operationCtrLevel) != 0)
	return -1;
	if (setBitsOfByte(&buf[30], 0, 3, message->trainDriveMode) != 0)
	return -1;
	if (int2bytes(&buf[34], SHT2BYTE, message->trainSpeed) != 0)
	return -1;
	return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToRouteStatusMsg
 description            convert bytes buffer to route status message
 parameter
 BYTE	[]						IN				buf
 ats_co_RouteStatusMsg *		OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToRouteStatusMsg(const BYTE buf[], ats_co_RouteStatusMsg *message)
{
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->routeIndex = bytes2int(&buf[7], SHT2BYTE);
message->operatingMode = countValueofBits(buf[9], 0, 3);
message->fault = countValueofBits(buf[9], 4, 1);
message->CommF = countValueofBits(buf[9], 5, 1);
message->SDA = countValueofBits(buf[9], 7, 1);
message->AtpAvailable = countValueofBits(buf[10], 0, 1);
message->failureInformation = countValueofBits(buf[10], 1, 6);
message->interlockAvailable = countValueofBits(buf[10], 7, 1);
message->routeCtrLevel = countValueofBits(buf[11], 0, 2);
message->delayRouteRelease = countValueofBits(buf[11], 2, 1);
message->routeStatus = countValueofBits(buf[11], 3, 1);
message->AtpCtrLevel = countValueofBits(buf[11], 4, 1);
return 0;
}

int ats_co_RouteStatusMsgConvertToBuffer(ats_co_RouteStatusMsg *message, BYTE buf[])
{
	if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
	return -1;
	if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
	return -1;
	if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
	return -1;
	if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
	return -1;
	if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
	return -1;
	if (int2bytes(&buf[7], SHT2BYTE, message->routeIndex) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 0, 3, message->operatingMode) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 4, 1, message->fault) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 5, 1, message->CommF) != 0)
	return -1;
	if (setBitsOfByte(&buf[9], 7, 1, message->SDA) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 0, 1, message->AtpAvailable) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 1, 6, message->failureInformation) != 0)
	return -1;
	if (setBitsOfByte(&buf[10], 7, 1, message->interlockAvailable) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 0, 2, message->routeCtrLevel) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 2, 1, message->delayRouteRelease) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 3, 1, message->routeStatus) != 0)
	return -1;
	if (setBitsOfByte(&buf[11], 4, 1, message->AtpCtrLevel) != 0)
	return -1;

	return 0;
}

///*******************************************************************
// Function name			ats_co_BufferConvertToWCUErrMsg
// description            convert bytes buffer to WCU error message
// parameter
// 	 BYTE	[]						IN				buf
// 	 ats_co_WCUErrMsg *		OUT 			message
// Return value
// 	 true or false
// *******************************************************************/
//int ats_co_BufferConvertToTrainErrMsg(const BYTE buf[] ,ats_co_TrainErrMsg  *message)
//{
//	message->PDI=(u_int32)bytes2int(&buf[0],CHAR2BYTE);
// 	message->msgType=(u_int32)bytes2int(&buf[1],SHT2BYTE);
// 	message->length=(u_int32)bytes2int(&buf[3],CHAR2BYTE);
// 	message->counter=bytes2int(&buf[4],SHT2BYTE);
// 	message->elementType=(u_int32)bytes2int(&buf[6],CHAR2BYTE);
// 	message->trainIndex=bytes2int(&buf[7],SHT2BYTE);
// 	message->minute=(u_int32)bytes2int(&buf[9],CHAR2BYTE);
// 	message->hour=(u_int32)bytes2int(&buf[10],CHAR2BYTE);
// 	message->day=(u_int32)bytes2int(&buf[11],CHAR2BYTE);
// 	message->month=(u_int32)bytes2int(&buf[12],CHAR2BYTE);
// 	message->year=(u_int32)bytes2int(&buf[13],CHAR2BYTE);
// 	message->errCode=(u_int32)bytes2int(&buf[14],SHT2BYTE);
// 	message->moduleCode=(u_int32)bytes2int(&buf[16],SHT2BYTE);
//	return 0;
//}

/*******************************************************************
 Function name			ats_co_BufferConvertToErrIndicationMsg
 description            convert bytes buffer to error indication message
 parameter
 BYTE	[]						IN				buf
 ats_co_WCUErrMsg *		OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToErrIndicationMsg(const BYTE buf[],
ats_co_ErrIndicationMsg *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->index = bytes2int(&buf[7], SHT2BYTE);
	message->minute = (u_int32) bytes2int(&buf[9], CHAR2BYTE);
	message->hour = (u_int32) bytes2int(&buf[10], CHAR2BYTE);
	message->day = (u_int32) bytes2int(&buf[11], CHAR2BYTE);
	message->month = (u_int32) bytes2int(&buf[12], CHAR2BYTE);
	message->year = (u_int32) bytes2int(&buf[13], CHAR2BYTE);
	message->errCode = (u_int32) bytes2int(&buf[14], SHT2BYTE);
	message->isRecover = (u_int32) bytes2int(&buf[16], SHT2BYTE);
	message->remark = (u_int32) bytes2int(&buf[17], INT2BYTE);

	ILOG("receive alarm message Type=%d,ElementType=%d,ElementID=%d,errCode=%d",message->msgType,message->elementType,message->index,message->errCode);


return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToALLMsgUpdate
 description            convert bytes buffer to train identify modify
 parameter
 	 BYTE	[]						IN				buf
 	 ats_co_ALLMsgUpdate *				OUT 			message
 Return value
 	 true or false
 *******************************************************************/
int ats_co_BufferConvertToALLMsgUpdate(const BYTE buf[] ,ats_co_ALLMsgUpdate *message)
{
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->hmiNumber = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	message->faultMsg = countValueofBits(buf[8],0,1);
	message->trainMsg = countValueofBits(buf[8],1,1);
	message->statusMsg = countValueofBits(buf[8],2,1);
	message->diagMsg = countValueofBits(buf[8],3,1);
	return 0;
}

int ats_co_ALLMsgUpdateConvertToBuffer(ats_co_ALLMsgUpdate *message, BYTE buf[])
{
	if (int2bytes(&buf[0], SHT2BYTE, message->msgType) != 0)
	return -1;
	if (int2bytes(&buf[2], INT2BYTE, message->length) != 0)
	return -1;
	if (int2bytes(&buf[6], CHAR2BYTE, message->hmiNumber) != 0)
	return -1;
	if (setBitsOfByte(&buf[8], 0, 1, message->faultMsg) != 0)
	return -1;
	if (setBitsOfByte(&buf[8], 1, 1, message->trainMsg) != 0)
	return -1;
	if (setBitsOfByte(&buf[8], 2, 1, message->statusMsg) != 0)
	return -1;
	if (setBitsOfByte(&buf[8], 3, 1, message->statusMsg) != 0)
	return -1;

	return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToTrainIDModify
 description            convert bytes buffer to train identify modify
 parameter
 	 BYTE	[]						IN				buf
 	 ats_co_TrainIDModify *				OUT 			message
 Return value
 	 true or false
 *******************************************************************/
int ats_co_BufferConvertToTrainIDModify(const BYTE buf[] ,ats_co_TrainIDModify  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->destination_ID = (u_int32)bytes2int(&buf[6],SHT2BYTE);
	message->service_ID = (u_int32)bytes2int(&buf[8],SHT2BYTE);
	message->serial = (u_int32)bytes2int(&buf[10],CHAR2BYTE);
	message->modifyType = (u_int32)bytes2int(&buf[11],CHAR2BYTE);
	message->processNO = (u_int32)bytes2int(&buf[12],CHAR2BYTE);
	message->car_one = (u_int32)bytes2int(&buf[13],THR2BYTE);
	message->car_two = (u_int32)bytes2int(&buf[16],THR2BYTE);
	message->logicalSection = (u_int32)bytes2int(&buf[19],SHT2BYTE);
	message->oldlogicalSection = (u_int32)bytes2int(&buf[21],SHT2BYTE);
	message->olddestination_ID = (u_int32)bytes2int(&buf[23],SHT2BYTE);
	message->oldservice_ID = (u_int32)bytes2int(&buf[25],SHT2BYTE);
	message->oldserial = (u_int32)bytes2int(&buf[27],CHAR2BYTE);
	message->trainIDFault = (u_int32)bytes2int(&buf[28],CHAR2BYTE);
	message->trainIndex = (u_int32)bytes2int(&buf[29],SHT2BYTE);
	message->arsStatus = (u_int32)bytes2int(&buf[34],CHAR2BYTE);
//	message->machineNo = (u_int32)bytes2int(&buf[29],CHAR2BYTE);
	return 0;
}

int ats_co_TrainIDModifyConvertToBuffer(ats_co_TrainIDModify *message, BYTE buf[])
{
	if (int2bytes(&buf[0], SHT2BYTE, message->msgType) != 0)
	return -1;
	if (int2bytes(&buf[2], INT2BYTE, message->length) != 0)
	return -1;
	if (int2bytes(&buf[6], SHT2BYTE, message->destination_ID) != 0)
	return -1;
	if (int2bytes(&buf[8], SHT2BYTE, message->service_ID) != 0)
	return -1;
	if (int2bytes(&buf[10], CHAR2BYTE, message->serial) != 0)
	return -1;
	if (int2bytes(&buf[11], CHAR2BYTE, message->modifyType) != 0)
	return -1;
	if (int2bytes(&buf[12], CHAR2BYTE, message->processNO) != 0)
	return -1;
	if (int2bytes(&buf[13], THR2BYTE, message->car_one) != 0)
	return -1;
	if (int2bytes(&buf[16], THR2BYTE, message->car_two) != 0)
	return -1;
	if (int2bytes(&buf[19], SHT2BYTE, message->logicalSection) != 0)
	return -1;
	if (int2bytes(&buf[21], SHT2BYTE, message->oldlogicalSection) != 0)
	return -1;
	if (int2bytes(&buf[23], SHT2BYTE, message->olddestination_ID) != 0)
	return -1;
	if (int2bytes(&buf[25], SHT2BYTE, message->oldservice_ID) != 0)
	return -1;
	if (int2bytes(&buf[27], CHAR2BYTE, message->oldserial) != 0)
	return -1;
	if (int2bytes(&buf[28], CHAR2BYTE, message->trainIDFault) != 0)
	return -1;

	return 0;
}
/*******************************************************************
 Function name			ats_co_BufferConvertToTimeTableLoad
 description            convert bytes buffer to train identify modify
 parameter
 	 BYTE	[]						IN				buf
 	 ats_co_TimeTableLoad *				OUT 			message
 Return value
 	 true or false
 *******************************************************************/
int ats_co_BufferConvertToTimeTableLoad(const BYTE buf[] ,ats_co_TimeTableLoad  *message)
{
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->innerID = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	return 0;
}


//TrainEventTWCMsg������������������������������������������������������������������������������������������������������������������������������������������
int
ats_co_BufferConvertToTrainEventTWCMsg(const BYTE buf[], ats_co_TrainEventTWCMsg *message)
{
	BYTE buftemp[2];
	int destination_low;
	int destination_high;
	int service_high;
message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
message->counter = bytes2int(&buf[4], SHT2BYTE);
message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
message->TWCIndex = (u_int32) bytes2int(&buf[7], SHT2BYTE);
//message->PTID = message->TWCIndex;
message->ATPbreakOn = countValueofBits(buf[11],1,1);

memset(buftemp, 0 , sizeof(BYTE)*2);
destination_low = countValueofBits(buf[15],4,4);
destination_high = (u_int32) bytes2int(&buf[14], CHAR2BYTE);
message->destination = destination_high* (accumulate(2, 4)) + destination_low;

memset(buftemp, 0 , sizeof(BYTE)*2);
buftemp[0] = buf[16];
service_high = countValueofBits(buf[15],0,4);
setBitsOfByte(&buftemp[1], 0, 4, service_high);
message->service = bytes2int(buftemp, SHT2BYTE);

message->serial = bytes2int(&buf[17], CHAR2BYTE);

message->trainID = message->destination*100000 + message->service*100 + message->serial;
message->carID = (u_int32) bytes2int(&buf[20], SHT2BYTE);
message->crewID = (u_int32) bytes2int(&buf[18], SHT2BYTE);
message->trainIndex = message->carID;
//message->eventTime.minute = (u_int32) bytes2int(&buf[9], CHAR2BYTE);
//message->eventTime.hour = (u_int32) bytes2int(&buf[10], CHAR2BYTE);
//message->eventTime.day = (u_int32) bytes2int(&buf[11], CHAR2BYTE);
//message->eventTime.month = (u_int32) bytes2int(&buf[12], CHAR2BYTE);
//message->eventTime.year = (u_int32) bytes2int(&buf[13], CHAR2BYTE);

//struct tm *tm_loc;
//time_t t;
//t = time(NULL);
//tm_loc = localtime(&t);
//message->eventTime.minute = tm_loc.tm_min;
//message->eventTime.hour = tm_loc.tm_hour;
//message->eventTime.day = tm_loc.tm_mday;
//message->eventTime.month = tm_loc.tm_mon;
//message->eventTime.year = tm_loc.tm_year;
return 0;
}


int
ats_co_TrainEventTWCMsgConvertToBuffer(ats_co_TrainEventTWCMsg *message, BYTE buf[])
{
	BYTE buftemp[2];
	int destination_low;
	int destination_high;
	int service_high;
if (int2bytes(&buf[0], CHAR2BYTE, message->PDI) != 0)
return -1;
if (int2bytes(&buf[1], SHT2BYTE, message->msgType) != 0)
return -1;
if (int2bytes(&buf[3], CHAR2BYTE, message->length) != 0)
return -1;
if (int2bytes(&buf[4], SHT2BYTE, message->counter) != 0)
return -1;
if (int2bytes(&buf[6], CHAR2BYTE, message->elementType) != 0)
return -1;
if (int2bytes(&buf[7], SHT2BYTE, message->TWCIndex) != 0)
return -1;
if (setBitsOfByte(&buf[11], 1, 1, message->ATPbreakOn) != 0)
return -1;

memset(buftemp, 0 , sizeof(BYTE)*2);
if (int2bytes(buftemp, SHT2BYTE, message->destination) != 0)
return -1;
destination_low = countValueofBits(buftemp[0],0,4);
destination_high = countValueofBits(buftemp[0],4,4) + buftemp[1];
if (setBitsOfByte(&buf[15], 4, 4, destination_low) != 0)
return -1;
if (int2bytes(&buf[14], CHAR2BYTE, destination_high) != 0)
return -1;

memset(buftemp, 0 , sizeof(BYTE)*2);
if (int2bytes(buftemp, SHT2BYTE, message->service) != 0)
return -1;
buf[16] = buftemp[0];
service_high = countValueofBits(buftemp[1],0,4);
if (setBitsOfByte(&buf[15], 0, 4, service_high) != 0)
return -1;

if (int2bytes(&buf[17], CHAR2BYTE, message->serial) != 0)
return -1;
if (int2bytes(&buf[20], SHT2BYTE, message->carID) != 0)
return -1;
if (int2bytes(&buf[18], SHT2BYTE, message->crewID) != 0)
return -1;
//if (int2bytes(&buf[9], CHAR2BYTE, message->eventTime.minute) != 0)
//return -1;
//if (int2bytes(&buf[10], CHAR2BYTE, message->eventTime.hour) != 0)
//return -1;
//if (int2bytes(&buf[11], CHAR2BYTE, message->eventTime.day) != 0)
//return -1;
//if (int2bytes(&buf[12], CHAR2BYTE, message->eventTime.month) != 0)
//return -1;
//if (int2bytes(&buf[13], CHAR2BYTE, message->eventTime.year) != 0)
//return -1;
return 0;
}


int ats_co_BufferConvertToRequireRunningMsg(const BYTE buf[] ,ats_co_RequireRunningMsg  *message)
{
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	return 0;
}

int ats_co_BufferConvertToRequireModifyATRModeMsg(const BYTE buf[] ,ats_co_RequireModifyATRModeMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->requireModifyType = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	message->lineNo = (u_int32)bytes2int(&buf[7],CHAR2BYTE);
	message->trainNO = (u_int32)bytes2int(&buf[8],SHT2BYTE);
	message->mode = (u_int32)bytes2int(&buf[10],CHAR2BYTE);
	message->para1 = (u_int32)bytes2int(&buf[11],SHT2BYTE);
	message->para2 = (u_int32)bytes2int(&buf[13],SHT2BYTE);
	message->destination = (u_int32)bytes2int(&buf[15],SHT2BYTE);
	message->serviceID = (u_int32)bytes2int(&buf[17],SHT2BYTE);
	message->serial = (u_int32)bytes2int(&buf[19],CHAR2BYTE);
	message->car1 = (u_int32)bytes2int(&buf[20],SHT2BYTE);
	message->car2 = (u_int32)bytes2int(&buf[22],SHT2BYTE);
	message->machineNO = (u_int32)bytes2int(&buf[24],CHAR2BYTE);
	return 0;
}

int ats_co_BufferConvertToLoadTimeTableMsg(const BYTE buf[] ,ats_co_LoadTimeTableMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->timerableInnerID = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	return 0;
}


//351
int ats_co_BufferConvertToTTPMsg(const BYTE buf[] ,ats_co_InterfaceTTPMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->innerid = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	message->versionno = (u_int32)bytes2int(&buf[7],INT2BYTE);
	return 0;
}
//23 add by quj
int
ats_co_BufferConvertToControlRightAutoMsg(const BYTE buf[] ,ats_co_TsControlRightAutoMsg  *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = (u_int32) bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->controlindex = (u_int32) bytes2int(&buf[7], SHT2BYTE);
	message->controlrightauto = (u_int32) bytes2int(&buf[12], CHAR2BYTE);
	ILOG("ats_co_BufferConvertToFepMsg:elementType=%d,controlindex=%d,controlrightauto=%d",message->elementType,message->controlindex,message->controlrightauto);
	return 0;
}
//1000 add by quj
int
ats_co_BufferConvertToCASBeatMsg(const BYTE buf[] ,ats_co_CASBeatMsg  *message)
{
	message->PDI = (u_int32)bytes2int(&buf[0],SHT2BYTE);

	ILOG("ats_co_BufferConvertToCASBeatMsg %d msg",message->PDI);
	return 0;
}
int
ats_co_BufferConvertToRecieveTrainidRequireSynMsg(const BYTE buf[] ,ats_co_RecieveLoadOKMsg  *message)
{
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	ILOG("ats_co_BufferConvertToRecieveLoadOKMsg %d msg",message->msgType);
	return 0;
}
int
ats_co_BufferConvertToLoadOKMsg(const BYTE buf[] ,ats_co_LoadOKmsg  *message)
{
	message->PDI = (u_int32)bytes2int(&buf[0],CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->IndicationStatus = (u_int32) bytes2int(&buf[4], SHT2BYTE);
	message->IndicationError = (u_int32) bytes2int(&buf[6], SHT2BYTE);
	message->IndicationRing = (u_int32) bytes2int(&buf[8], SHT2BYTE);
	message->IndicationCounter = (u_int32) bytes2int(&buf[10], SHT2BYTE);
	ILOG("ats_co_BufferConvertToLoadOKMsg msg pdi:%d,msgtype:%d,Loadstatus:%d",message->PDI,message->msgType,message->IndicationStatus);
	return 0;
}
int ats_co_BufferConvertToCFepRequireCASDataMsg(const BYTE buf[] ,ats_co_CFepRequireCASDataMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->machineNO = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	return 0;
}


int ats_co_BufferConvertToAlarmMsg(const BYTE buf[] ,ats_co_AlarmMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->equipment_Type = (u_int32)bytes2int(&buf[6],CHAR2BYTE);
	message->equipment_ID = (u_int32)bytes2int(&buf[7],SHT2BYTE);
	message->minute = (u_int32)bytes2int(&buf[9],CHAR2BYTE);
	message->hour = (u_int32)bytes2int(&buf[10],CHAR2BYTE);
	message->day = (u_int32)bytes2int(&buf[11],CHAR2BYTE);
	message->month = (u_int32)bytes2int(&buf[12],CHAR2BYTE);
	message->year = (u_int32)bytes2int(&buf[13],CHAR2BYTE);
	message->alarm_ID = (u_int32)bytes2int(&buf[14],SHT2BYTE);
	message->isRecover = (u_int32)bytes2int(&buf[16],CHAR2BYTE);
	message->remark = (u_int32)bytes2int(&buf[17],INT2BYTE);
	return 0;
}


int ats_co_BufferConvertToLCfepNetStatusMsg(const BYTE buf[] ,ats_co_LCfepNetStatusMsg  *message)
{

	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->equipment_Type = (u_int32)bytes2int(&buf[6],SHT2BYTE);
	memset(message->LCfepMsgData,0,message->length-2);
	memcpy(message->LCfepMsgData,&buf[8],message->length-2);
	return 0;
}

int ats_co_BufferConvertToCASNetStatusMsg(const BYTE buf[] ,ats_co_CASNetStatusMsg *message)
{
	message->fepID = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->fepModule = (u_int32)bytes2int(&buf[2],SHT2BYTE);
	message->fepStatus = (u_int32)bytes2int(&buf[4],SHT2BYTE);
	return 0;
}
int ats_co_CASNetStatusMsgConvertToBuffer(ats_co_CASNetStatusMsg *message, BYTE buf[])
{
	if (int2bytes(&buf[0], SHT2BYTE, message->fepID) != 0)
	return -1;
	if (int2bytes(&buf[2], SHT2BYTE, message->fepModule) != 0)
	return -1;
	if (int2bytes(&buf[4], SHT2BYTE, message->fepStatus) != 0)
	return -1;
	return 0;
}


int
ats_co_BufferConvertToInterfaceTrackStatusMsg(const BYTE buf[], ats_co_InterfaceTrackStatusMsg *message)
{
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->trackid = bytes2int(&buf[6], INT2BYTE);
	message->freestatus = (u_int32) bytes2int(&buf[10], CHAR2BYTE);
	message->car1 = (u_int32) bytes2int(&buf[11], SHT2BYTE);
	message->car2 = (u_int32) bytes2int(&buf[13], SHT2BYTE);
return 0;
}


int
ats_co_BufferConvertToHmiToATRManualRegulationValue(const BYTE buf[], ats_co_HmiToATRManualRegulationValue *message)
{
	DLOG("Receive 260 message");
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length=(u_int32)bytes2int(&buf[2],INT2BYTE);
	message->Type=(u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->LineNO =(u_int32) bytes2int(&buf[7], CHAR2BYTE);
	message->StationNum=(u_int32) bytes2int(&buf[8], CHAR2BYTE);
	message->TrainIndex=(u_int32) bytes2int(&buf[9], SHT2BYTE);
	message->Destination=(u_int32) bytes2int(&buf[11], SHT2BYTE);
	message->ServiceID=(u_int32) bytes2int(&buf[13], SHT2BYTE);
	message->TripID=(u_int32) bytes2int(&buf[15], CHAR2BYTE);
	message->car1=(u_int32) bytes2int(&buf[16], SHT2BYTE);
	message->car2=(u_int32) bytes2int(&buf[18], SHT2BYTE);
	message->AllStationManualReguData = (char*)malloc(14*message->StationNum);
	memset(message->AllStationManualReguData,0,14*message->StationNum);
	ILOG("message->length = %d",message->length);//21
//	memcpy(message->AllStationManualReguData,&buf[20],message->length-14);
	memcpy(message->AllStationManualReguData,&buf[20],14*message->StationNum);

return 0;
}

/*******************************************************************
 Function name			ats_co_BufferConvertToDirectionMsg
 description            convert bytes buffer to Direction  message
 parameter
 BYTE	[]						IN				buf
 ats_co_DirectionMsg *	OUT 			message
 Return value
 true or false
 *******************************************************************/
int
ats_co_BufferConvertToDirectionMsg(const BYTE buf[],ats_co_DirectionMsg *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->directionId = (u_int32) bytes2int(&buf[7], SHT2BYTE);
	message->leftDirection = (u_int32) bytes2int(&buf[11], CHAR2BYTE)&0x01;
	message->rightDirection = ((u_int32) bytes2int(&buf[11], CHAR2BYTE)&0x02)>>1;
	ILOG("ats_co_BufferConvertToDirectionMsg:elementType=%d,directionId=%d,leftDirection=%d,rightDirection=%d",message->elementType,message->directionId,message->leftDirection,message->rightDirection);
	return 0;
}
	/*******************************************************************
Function name			ats_co_BufferConvertToPTIMsg
description            convert bytes buffer to PTI  message
parameter
BYTE	[]						IN				buf
ats_co_PTIMsg *	OUT 			message
Return value
true or false
*******************************************************************/
int
ats_co_BufferConvertToPTIMsg(const BYTE buf[],ats_co_PTIMsg *message)
{
	message->PDI = (u_int32) bytes2int(&buf[0], CHAR2BYTE);
	message->msgType = (u_int32) bytes2int(&buf[1], SHT2BYTE);
	message->length = (u_int32) bytes2int(&buf[3], CHAR2BYTE);
	message->counter = bytes2int(&buf[4], SHT2BYTE);
	message->elementType = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
	message->PTIindex = (u_int32) bytes2int(&buf[7], SHT2BYTE);
	message->crewnumH = (u_int32) bytes2int(&buf[14], CHAR2BYTE);
	message->crewnumT = (u_int32) bytes2int(&buf[15], CHAR2BYTE);
	message->crewnumS = (u_int32) bytes2int(&buf[16], CHAR2BYTE);
	//
	message->tripnumThousand = (u_int32) bytes2int(&buf[17], CHAR2BYTE);
	message->tripnumH = (u_int32) bytes2int(&buf[18], CHAR2BYTE);
	message->tripnumT = (u_int32) bytes2int(&buf[19], CHAR2BYTE);
	message->tripnumS = (u_int32) bytes2int(&buf[20], CHAR2BYTE);
	message->destnum = (u_int32) bytes2int(&buf[21], CHAR2BYTE);
	message->trainstatus = (u_int32) bytes2int(&buf[22], CHAR2BYTE);
	message->direction = (u_int32) bytes2int(&buf[23], CHAR2BYTE);
	message->activeMSB = (u_int32) bytes2int(&buf[24], CHAR2BYTE);
	message->activeLSB = (u_int32) bytes2int(&buf[25], CHAR2BYTE);
	ILOG("ats_co_BufferConvertToPTIMsg msgtype=%d elementType=%d",message->msgType,message->elementType);
	return 0;
}

/*******************************************************************
 Function name			ats_co_DirectionMsgConvertToBuffer
 description            convert Direction  message to bytes buffer
 parameter
 ats_co_AxleCounterMsg *	IN	 		message
 BYTE	[]						OUT			buf
 Return value
 true or false
 *******************************************************************/
int
ats_co_DirectionMsgConvertToBuffer(ats_co_DirectionMsg *message, BYTE buf[])
{
	//TODO:ats_co_DirectionMsgConvertToBuffer
	return 0;
}

int ats_co_BufferConvertToRecieveTrainidSynMsg(const BYTE buf[],ats_co_RecieveTrainidSynMsg *message)
{
//	20130614 sunjh 26->27 ,&buf[6]->&buf[7]
//	int total = 0;
	message->msgType = (u_int32) bytes2int(&buf[0], SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->trainNum = (u_int32) bytes2int(&buf[6], CHAR2BYTE);
    message->AllTrainData = (char*)malloc(27*message->trainNum+1);
    memset(message->AllTrainData,0,27*message->trainNum+1);
    ILOG("message->length = %d",message->length);
    memcpy(message->AllTrainData,&buf[6],27*message->trainNum+1);

	return 0;
}

//205
int ats_co_BufferConvertToHMItoCASMsg(const BYTE buf[] ,ats_co_InterfaceHMItoCASMsg  *message)
{
	DLOG("Receive 205 message");
	message->msgType = (u_int32)bytes2int(&buf[0],SHT2BYTE);
	message->length = (u_int32)bytes2int(&buf[2],INT2BYTE);
	message->type = (u_int32)bytes2int(&buf[6],CHAR2BYTE);

	//20130328 sunjh
	if(message->msgType == 205)
	{
		if(message->type == 1 )
		{
			if(fepStatusType == 0)
			{
				//2
				adapter_co_send_CASToHMIMessage(99,0,2);
			}
			else if(fepStatusType == 1)
			{
				//3
				adapter_co_send_CASToHMIMessage(99,0,3);
			}
		}
	}





	return 0;
}