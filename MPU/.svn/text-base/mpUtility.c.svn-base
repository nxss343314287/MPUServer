/*******************************************************************
 filename      mpUtility.c
 author        root
 created date  2012-1-9
 description
 warning
 modify history
 author        date        modify        description
 *******************************************************************/

#include "include/MPUtility.h"
#include "include/MPUConvert.h"
#include "include/MPUThreadSyncMutex.h"



unsigned long long int
get_time_msecond(void)
{
  time_t t;
  struct tm * now;
  struct timeval tv;
  unsigned long long int msec;

  gettimeofday(&tv, NULL);
  t = tv.tv_sec;
  now = localtime(&t);

  msec = (now->tm_hour * 60 * 60 + now->tm_min * 60 + now->tm_sec) * 1000
      + tv.tv_usec / 1000;
  return ((unsigned long long int) msec);
}

char *
get_time_string(char *curtime)
{
  time_t t;
  struct tm * now;
  struct timeval tv;
  char ptime[sizeof(struct timeval)] ={ 0 };

  gettimeofday(&tv, NULL);

  now = localtime(&t);
  sprintf(ptime, "%02d:%02d:%02d.%lld", now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec);

  strncpy(curtime, ptime, sizeof(struct timeval));
  return ((char *)curtime);
}

//time of long long int format convert to string
void
time_to_string(unsigned long long int lnTime, char *szTime)
{

  //char *  asctime(const struct tm *tm);
  //char *  asctime_r(const struct tm *tm, char *buf);

  //char *  ctime(const time_t *timep);
  //char *  ctime_r(const time_t *timep, char *buf);

  //struct tm *  gmtime(const time_t *timep);
  //struct tm *  gmtime_r(const time_t *timep, struct tm *result);

  //struct tm *  localtime(const time_t *timep);
  //struct tm *  localtime_r(const time_t *timep, struct tm *result);


	return ;
}

//time of string format convert to long long int
void
time_to_msecond(char *szTime, unsigned long long int lngTime)
{

	return ;
}

//telegramTime conversion , timespan value equal time passed from 2000-1-1
void datetime_conversion(DateTime *time)
{

	u_int64  timespan ,timeticks;
	BYTE ticksBytes[4];
	struct tm  tms ,tmp;
	time_t  timep;


	ticksBytes[0]=time->hour;
	ticksBytes[1]=time->day;
	ticksBytes[2]=time->month;
	ticksBytes[3]=time->year;

	timespan=bytes2int(ticksBytes, LONGLONG2BYTE);
	//tv=localtime();
	tms.tm_year=2000;
	tms.tm_mon=1;
	tms.tm_mday=1;
	tms.tm_hour=0;
	tms.tm_min=0;
	tms.tm_sec=0;
	timep= mktime(&tms);

	timeticks = timespan*10000000 + timep ;
	timep = timeticks;

	gmtime_r(&timep,&tmp);

	time->hour=tmp.tm_hour;
	time->day=tmp.tm_mday;
	time->month=tmp.tm_mon;
	time->year=tmp.tm_year;
//
//DateTime typeTelegramTime = new DateTime(2000, 1, 1, 0, 0, 0);//siemens send time in seconds mode,the initial time;
//long timeSpan = (System.DateTime.Now.Ticks - typeTelegramTime.Ticks) / 10000000;
//trainEvePackage.Minute = 0xff;
//trainEvePackage.Hour = Convert.ToByte(timeSpan & 0x00ff);
//trainEvePackage.Day = Convert.ToByte((timeSpan & 0xff00) >> 8);
//trainEvePackage.Month = Convert.ToByte((timeSpan & 0xff0000) >> 16);
//trainEvePackage.Year = (byte)((timeSpan & 0xff000000) >> 24);

	//return ;
}
