/*
 * mpusender.h
 *
 *  Created on: 2012-6-22
 *      Author: root
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_
#include "MPUSendConvert.h"
#include "sqlite3.h"
void onclose(void);
int app2netfd;
int app2TSnetfd;
int adapter_init(void);
int mpuSender_SendTask_process (sqlite3 *db);
int mpuSender_SendNetStatus_process (sqlite3 *db);
int mpusender_trcsend_judge (sqlite3 *db,int interlock_num);
#endif /* ADAPTER_H_ */
