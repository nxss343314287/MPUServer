/*******************************************************************
        filename��                   mpuclient.c
        author��                     root
        create data��                05-Aug-2011
        description��
        detailed description��
  warning��
        modify history��
        author       data(modify)   modify(list)           description

********************************************************************/

#include "include/MPU.h"
#include "include/MPUMessage.h"
#include "include/MPUCenter.h"
#include	 <fcntl.h>

#define PACKET_LEN 128




/*******************************************************************
        Function name��                                 ats_co_MsgUnpack
        description��                                           ת�������ֽڵ���ݿ���¼�����Ҹ��µ���ݿ����
        parameter��                                                     ��������
                                                ���                                     ����/���                        ����                    ����
                                                db                                              ����                            sqlite3*                ��ݿ�����
                                                packet                          ����                            BYTE *                  ��ʽ���ı��ĵ�ָ��
                                                len                                             ����                            size_t                  ���ĳ���
        Return value��                                  �ɹ�����0;ʧ�ܷ��ش�����

*******************************************************************/
/*

int ats_co_MsgUnpack(sqlite3 *db, BYTE *packet , size_t len)
{
        //we think that buffer is serial bytes received,and length is PACKET_LEN.
        //in fact,if we received message from a pipe connected to IO module,we can
        //get the length of message .
        //unfortunitely,when we read from a pipe , perhaps we don't know how to read buffer,
        //but it isn't the object of this task. here we just have known buffer and length.
        ATS_CO_MSG  *this_message ;

        this_message=malloc(sizeof(ATS_CO_MSG));
        memset(this_message,0,sizeof(ATS_CO_MSG));

        //init member of message
        if(ats_co_initATSCOMessage(this_message,packet,len)==false)
        {
                printf("init message error!\n");
                exit(-1);
        }

        //now ,all of things are ready,so we can start to control progress of parsing...
        //unpack message packet
        ats_co_unpackATSCOMessage(this_message);

        //then refresh them to database tables
        ats_co_refreshDBATSCOMessage(db,this_message);

        //all things are over ,free all of resources
        ats_co_freeATSCOMessage(this_message);

        return true;
}

*/

/*******************************************************************
        Function name��                                 ats_co_MsgPack
        description��                                           ����������ݣ�����ݿ��ȡ��¼��ݲ���ת�������ĸ�ʽ
        parameter��                                                     ��������
                                                ���                                     ����/���                        ����                    ����
                                                db                                              ����                            sqlite3*                ��ݿ�����
                                                packet                          ����/���                BYTE *                  ���ĸ�ʽ�洢�ռ�ָ��
                                                len                                             ����                            size_t                  ���ĳ���
        Return value��                                  �ɹ�����0;ʧ�ܷ��ش�����

*******************************************************************/
/*
int ats_co_MsgPack(sqlite3 *db, BYTE *packet , size_t len)
{
        return true;
}
*/
//to creat a pipe management thread
int FIFOClient(BYTE *buf, size_t len)
{
        int fp;
        int re;
        if((fp=open(NET2APP_PIPE,O_WRONLY))<0)
        {
                printf("open file for write error\n");
                ELOG("open file for write error\n");
        }
        //show_buf(buf,len);

        if((re=write(fp,buf,len))<0)
        {
                printf("write file error,re=%d\n",re);
                ELOG("write file error,re=%d\n",re);
        }
        //printf("has write file len,len=%d\n",re);

        //close(fp);
        return 1;
}


//pro(FILE *fp)
//{
//        close(fp);
//}
//
//
//
//show_buf(BYTE *buf,int len)
//{
//        int i=0;
//
//        for(i=0;i<len;i++)
//        {
//                printf("buf[%d]=%02x",i,*(buf+i));
//        }
//        printf("\n");
//}

//
//
//
////
int main1(int argc, char **argv)
{

        //to init a service process for monitor FIFO pipe. we supposed that
        //there are many bytes of packets in pipe,we read all of them
        //at one time.how to parse them correctly? we can search head of
        //packet , then parse the head,and do the analys of packet throughtly.

        int count=0;
        BYTE *buf;
        BYTE  *buffer;
        //sqlite3 *db;

//        rc = sqlite3_open(DB_NAME, &db);
//        if (0 != rc)
//        {
//                printf("SQLerror: %s \n", sqlite3_errmsg(db));
//                sqlite3_close(db);
//                exit(-1);
//        }
//

        /*
        ats_co_SignalMsg message ;
        message.PDI=1;
        message.msgType=STATUS_TYPE;
        message.length=PACKET_LEN-MSG_HEADER_LEN ;
        message.counter=14;
        message.elementType=SIGNAL;
        message.signalId=id++;//<�źŻ���><value=><type=int><postion=7-8><bits=>
        message.signalStatus=6;//<�źŻ���ʾ״̬><value=><type=int><postion=11><bits=4-5-6>
        message.signalAutoStatus=2;//<�ź�><value=0� RS 2ARC 3��FT><type=int><postion=12><bits=2-3>
        message.lockFlag=0;//<�źŻ�����value=0=������ 1=����><type=int><postion=12><bits=1>
        message.signalFault=1;//<�źŻ����״̬><value=0û�й��� 1�й���><type=int><postion=13><bits=4>
        */

        ats_co_TrackMsg message ;
        message.PDI=1;
        message.msgType=400;
        message.length=119 ;
        message.counter=1;
        message.elementType=207;                      //bits 6
        message.trackid=1;                                                      //bits 7 8
        //message.direction=1;
        message.lockFlag=1;             //11:b3
        message.freeStatus=1;           //11:b2
        //message.status=6;               //22:b3 b4 b5


        buffer=malloc(PACKET_LEN);
        memset(buffer,0,PACKET_LEN);


        //add net header
        buf=malloc(MSG_HEADER_LEN+PACKET_LEN);
        memset(buf,0,MSG_HEADER_LEN+PACKET_LEN);
        printf("buf size=%d\n",sizeof(message_header_t));
        message_header_t  header;

        for(;;)
                  {

                  /*
        if(ats_co_SignalMsgConvertToBuffer(&message,buffer)==ERROR_MESSAGE_TYPE)
                  {
           printf("convert field error! cause :data !\n");
           exit(-1);
                  }
                  */

        message.counter+=1;
        message.trackid+=1;

        if(ats_co_TrackMsgConvertToBuffer(&message,buffer)==ERROR_MESSAGE_TYPE)
                    {
                printf("convert field error! cause :data !\n");
              //  sqlite3_close(db);
                exit(-1);
                     }

        header.seqno=count++;
        header.datalen=PACKET_LEN;
        header.datatype=2;
        header.protocoltype=8;

        header.timestamp=get_time_msecond();

        printf("h1index,SrcNo,DestNo,FrameNum,Protocol,InoutFlag,timestamp=(%d,%d,%d,%d,%d,%d,%lld)",
        header.seqno,header.srcno,header.destno,header.seqno,header.protocoltype,header.mainorbak,header.timestamp);

        if(ats_co_packNetHeader(&header,buf)==-1)
                  {

            printf("error pack net header\n");
            //sqlite3_close(db);
            exit(-1) ;

                   }

        memcpy(buf+MSG_HEADER_LEN,buffer,PACKET_LEN);

        //demo procedure of message received
        FIFOClient(buf,PACKET_LEN+MSG_HEADER_LEN);
        usleep(2000);

                  }

        //free database connection
        //sqlite3_close(db);

        //now , do a test for return a result from datatabase query
        exit(0);
}
//
//
