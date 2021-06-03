/*******************************************************************
	filename���������������������������							ats_co_importdata.c
	author���������������������������								zs
	create data���������������������������						2012-9-1
	description���������������������������						import data (oracle and sqlite)
	modify history���������������������������
	author			data(modify)			modify(list)			description

********************************************************************/

//#include "ats_co_sql.h"
#include "oci.h"
#include "ociap.h"
#include "oratypes.h"
#include <sqlite3.h>
#include "../log.h"

#define MAX_SQL_STR_LEN 2048

OCIEnv           *p_env=NULL;
OCIError         *p_err=NULL;
OCISvcCtx        *p_svc=NULL;
OCIStmt          *p_sql=NULL;
OCIDefine        *p_dfn    = (OCIDefine *) 0;
OCIParam         *p_colhp=NULL;
OCIBind          *p_bind =NULL;

OCIServer *p_srv=NULL;
OCISession *p_ses=NULL;
//text connect_str[MAX_SQL_STR_LEN];

#define TABLE_NUMBER_MAX  	    100    //最多支持的表数
#define ROW_NUMBER_MAX  	    2000  //最多支持表里的行数
#define ROW_SAME_NUMBER_MAX  	1   //最多支持第一个int型字段拥有相同值的行数
#define COLUMN_NAME_LEN  	    100
#define COLUMN_TYPE_LEN  	    10
#define COLUMN_NUMBER_MAX  	    10   //最多支持表的列数
#define COLUMN_NAME_LIST_LEN  	COLUMN_NAME_LEN*COLUMN_NUMBER_MAX
#define COLUMN_ALL_LIST_LEN  	COLUMN_NAME_LEN*COLUMN_NUMBER_MAX+COLUMN_TYPE_LEN*COLUMN_NUMBER_MAX
#define COLUMN_BLOB_NUMBER_MAX  	    2 //最多支持类型为BLOB的列数
#define BLOB_SIZE  	            200

typedef char textValueName[COLUMN_NAME_LEN];
typedef char blobFieldValue[BLOB_SIZE];

typedef struct _blobinfo
{
	blobFieldValue *blobFieldinfo;
}blobinfo;


int ats_co_import_Connect_to_Oracle (char* tnsName, char* userName, char* password)
{
	int rc = 0;

	/* Initialize OCI evironment*/
   rc = OCIEnvCreate((OCIEnv **) &p_env,OCI_DEFAULT,(dvoid *)0,
          (dvoid * (*)(dvoid *, size_t)) 0,
          (dvoid * (*)(dvoid *, dvoid *, size_t))0,
          (void (*)(dvoid *, dvoid *)) 0,
          (size_t) 0, (dvoid **) 0);

   /* Initialize handles */
   rc = OCIHandleAlloc((dvoid *) p_env, (dvoid **) &p_sql,
              OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);

      rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_err, OCI_HTYPE_ERROR,
                 (size_t) 0, (dvoid **) 0);
      rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_srv, OCI_HTYPE_SERVER,
                 (size_t) 0, (dvoid **) 0);
      rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_svc, OCI_HTYPE_SVCCTX,
                 (size_t) 0, (dvoid **) 0);
      rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_ses, OCI_HTYPE_SESSION,
                 (size_t) 0, (dvoid **) 0);

      /* Connect to database server */
      //sprintf(connect_str, "Data Source=(DESCRIPTION =(ADDRESS_LIST =(ADDRESS = (PROTOCOL = TCP)(HOST = 192.168.35.56)(PORT = 1521)))(CONNECT_DATA =(SERVICE_NAME = orcl.nriet)));User ID=HMI;pwd=HMI;Unicode=True");
      sprintf(connect_str, "Data Source=(DESCRIPTION =(ADDRESS_LIST =(ADDRESS = (PROTOCOL = TCP)(HOST = 130.50.150.1)(PORT = 1521)))(CONNECT_DATA =(SERVICE_NAME = ATSForSZL2.nriet)))");
      rc = OCIServerAttach(p_srv, p_err, tnsName, strlen(tnsName),(ub4)OCI_DEFAULT);
   //		   strlen(userName), (unsigned char*)password, strlen(password), (unsigned char*)tnsName,strlen(tnsName));
      if (rc != OCI_SUCCESS)
         {
   	   text errbuf[512];
   	   ub4 buflen;
   	   ub4 errcode;
   	   OCIErrorGet((dvoid *)p_err, (ub4) 1, (text *) NULL, &errcode, (unsigned char*)errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
   	   FLOG("Error - %.*s\n", 512, errbuf);
      	  FLOG("Connect to orcle fail\n");
      	  printf("Connect to orcle fail\n");
         return -1;
         }
         else
         {
          //ILOG("Connect to orcle successful! \n");
          printf("Connect to orcle successful! \n");
         }

      OCIAttrSet(p_svc, OCI_HTYPE_SVCCTX,p_srv,0, OCI_ATTR_SERVER,p_err);
      OCIAttrSet(p_ses, OCI_HTYPE_SESSION,userName,strlen(userName), OCI_ATTR_USERNAME,p_err);
      OCIAttrSet(p_ses, OCI_HTYPE_SESSION,password,strlen(password), OCI_ATTR_PASSWORD,p_err);

      rc = OCISessionBegin(p_svc, p_err, p_ses, OCI_CRED_RDBMS, OCI_DEFAULT);

      OCIAttrSet(p_svc, OCI_HTYPE_SVCCTX,p_ses,0, OCI_ATTR_SESSION,p_err);

   rc = OCIHandleAlloc((dvoid *) p_env, (dvoid **) &p_sql,
           OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
   return 0;
}

int ats_co_import_DisConnect_to_Oracle ()
{
	OCISessionEnd(p_svc,p_err,p_ses,0);
	OCIServerDetach(p_srv, p_err,OCI_DEFAULT);          /* Disconnect */
	OCIHandleFree((dvoid *) p_srv, OCI_HTYPE_SERVER);   /* Free handles */
	OCIHandleFree((dvoid *) p_sql, OCI_HTYPE_STMT);
	OCIHandleFree((dvoid *) p_svc, OCI_HTYPE_SVCCTX);
	OCIHandleFree((dvoid *) p_err, OCI_HTYPE_ERROR);
	OCIHandleFree((dvoid *) p_ses, OCI_HTYPE_SESSION);
	printf("DisConnect to orcle successful! \n");
//	DLOG("DisConnect to orcle successful! \n");
	return 0;
}

int ats_co_import_ErrorProc_to_Oracle_fortest (sword status)
{
	sb4 errcode;
	text errbuf[MAX_SQL_STR_LEN];
	memset(errbuf, 0, MAX_SQL_STR_LEN);
	if(status == OCI_ERROR)
	{
		OCIErrorGet(p_err, (ub4)1, NULL, &errcode, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
		ELOG("oracle occur error! errcode:%d,errinfo:%s\n",errcode,errbuf);
		printf("oracle occur error! errcode:%d,errinfo:%s\n",errcode,errbuf);
	}
	return 0;
}


int ats_co_import_O2S_ImportData_for_tableName (sqlite3 *db, char* tableName)
{

   int             rc;
   char            mysql[MAX_SQL_STR_LEN];
   char *errMsg = NULL;
   char sqlstr[MAX_SQL_STR_LEN];
   sqlite3_stmt *stmt;
   int  fieldname_sizep = 0;
   char fieldname[COLUMN_NAME_LEN];
   int  columntype[COLUMN_NUMBER_MAX];
   char fieldalllist[COLUMN_ALL_LIST_LEN];  //for create sql
   char fieldnamelist[COLUMN_NAME_LIST_LEN]; //for insert sql
   char fieldvaluelist[COLUMN_NAME_LIST_LEN];//for insert sql
   int  fieldtypeIndicator[COLUMN_NUMBER_MAX];
   char firstintfieldname[COLUMN_NAME_LEN];
   int  firstintfieldvalue[ROW_NUMBER_MAX];

   int row_number = 0;
   int column_number = 0;

   memset(fieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(columntype, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(fieldalllist, 0, sizeof(char)*COLUMN_ALL_LIST_LEN);
   memset(fieldnamelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldvaluelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldtypeIndicator, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(firstintfieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(firstintfieldvalue, 0, sizeof(int)*ROW_NUMBER_MAX);


   /* import to tableName*/
   int i = 0;
   int j = 0;
   int k = 0;
   int l = 0;
   char *stringtmp;

   //get primary key of tableName from oracle
   int primarykey_exist_flag = 0;
   char primkeyfieldname[COLUMN_NAME_LEN];
   memset(primkeyfieldname, 0, COLUMN_NAME_LEN);
   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select column_name from user_cons_columns e inner join user_constraints f on e.table_name = f.table_name and f.constraint_type = 'P' where e.table_name = '%s'",tableName);
   //sprintf(mysql, "select a.column_name from user_cons_columns a, user_constraints b where a.constraint_name = b.constraint_name  and b.constraint_type = 'P' and a.table_name = '%s'",tableName);
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
              (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) &primkeyfieldname,
              (sb4) sizeof(primkeyfieldname), (ub2)SQLT_STR, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
              (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   OCIAttrGet((const void *)p_sql, OCI_HTYPE_STMT, (void *) &primarykey_exist_flag,
    		   (ub4 *) sizeof(primarykey_exist_flag), OCI_ATTR_ROW_COUNT, p_err);

   //make up slq statement(create,insert) for sqlite3, we need column_name and column_type
   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select * from %s",tableName);
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
			 (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   if(rc != 0)
   {
	   ats_co_import_ErrorProc_to_Oracle_fortest (rc);
   }
   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 0, (ub4) 0,
			 (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   if(rc != 0)
   {
	   ats_co_import_ErrorProc_to_Oracle_fortest (rc);
   }
   OCIAttrGet(p_sql, OCI_HTYPE_STMT, (void *) &column_number,
    		   0, OCI_ATTR_PARAM_COUNT, p_err);
   if(column_number == 0)
   {
	   ELOG("oracle occur error! table name: %s\n", tableName);
	   return -1;
   }

   int firstintfield_set_flag = 0;
   char fieldnametmp[COLUMN_NAME_LEN];
   char fieldalltmp[COLUMN_NAME_LEN + COLUMN_TYPE_LEN];
   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
   memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);
   while(i < column_number)
   {
	   OCIParamGet(p_sql,OCI_HTYPE_STMT,p_err,(void **)&p_colhp,(ub4)(i+1));

	   OCIAttrGet(p_colhp, OCI_DTYPE_PARAM, &columntype[i],
    		   0, OCI_ATTR_DATA_TYPE, p_err);

	   OCIAttrGet(p_colhp, OCI_DTYPE_PARAM, (void *) &stringtmp,
    		   (ub4 *) &fieldname_sizep, OCI_ATTR_NAME, p_err);

	   if(columntype[i] == SQLT_INT || columntype[i] == SQLT_NUM)
		   fieldtypeIndicator[i] = SQLITE_INTEGER;
	   else if(columntype[i] == SQLT_BLOB)
		   fieldtypeIndicator[i] = SQLITE_BLOB;
	   else if(columntype[i] == SQLT_CLOB)
		   fieldtypeIndicator[i] = SQLITE_BLOB;
	   else
 		   fieldtypeIndicator[i] = SQLITE_TEXT;

	   memset(fieldname, 0, COLUMN_NAME_LEN);
       strncpy(fieldname,stringtmp,fieldname_sizep);

	   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
       memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);
	   if(i != column_number - 1)
	   {
		   sprintf(fieldnametmp, "%s,",fieldname);

		   if(fieldtypeIndicator[i] == SQLITE_INTEGER)
		   {
			   if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"integer");
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"integer");

			   if(firstintfield_set_flag == 0)
			   {
				   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
				   strncpy(firstintfieldname, fieldname, strlen(fieldname));
				   firstintfield_set_flag = 1;
			   }
		   }
		   else if(fieldtypeIndicator[i] == SQLITE_TEXT)
		   {
			   if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"text");
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"text");
		   }
		   else
		   {
			   if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"blob");
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"blob");
		   }
	   }
	   else
	   {
		   sprintf(fieldnametmp, "%s",fieldname);

		   if(fieldtypeIndicator[i] == SQLITE_INTEGER)
		   {
			   if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"integer");
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"integer");

			   if(firstintfield_set_flag == 0)
			   {
				   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
				   strncpy(firstintfieldname, fieldname, strlen(fieldname));
				   firstintfield_set_flag = 1;
			   }
		   }
		   else if(fieldtypeIndicator[i] == SQLITE_TEXT)
		   {
			   if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"text");
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"text");
		   }
		   else
		   {
			    if(primarykey_exist_flag == 1 && strcmp(primkeyfieldname,fieldname) == 0)
				   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"blob");
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"blob");
		   }
	   }
	   strcat(fieldnamelist, fieldnametmp);
	   strcat(fieldalllist, fieldalltmp);

	   i++;
   }

   //drop old table and create new table in sqlite3
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "drop table %s", tableName);
   sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "create table %s(%s)", tableName,fieldalllist);
   sqlite3_exec(db, sqlstr, 0, 0, &errMsg);

   //begin to get data from oracle
   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select count(*) from %s",tableName);
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
              (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) &row_number,
              (sb4) sizeof(int), (ub2)SQLT_INT, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
              (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   if(row_number == 0)
   {
//	   DLOG("import from oracle to sqlite sucess! %s->data number:%d\n",tableName,row_number);
	   printf("import from oracle to sqlite sucess! %s->data number:%d\n",tableName,row_number);
   	   return -1;
   }

   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select distinct %s from %s order by %s",firstintfieldname,tableName,firstintfieldname);
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
			 (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 0, (ub4) 0,
			 (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

   int firstintfield_listnum = 0;
   int value_index = 0;
   if(rc == OCI_SUCCESS)
   {
	   while(rc != OCI_NO_DATA)
	   {
		   OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) &firstintfieldvalue[value_index],
			 (sb4) sizeof(int), (ub2)SQLT_INT, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
		   rc = OCIStmtFetch2(p_sql, p_err, 1, (ub2)OCI_FETCH_NEXT, 1, OCI_DEFAULT);
		   value_index++;
	   }
       OCIAttrGet((const void *)p_sql, OCI_HTYPE_STMT, (void *) &firstintfield_listnum,
    		   (ub4 *) sizeof(firstintfield_listnum), OCI_ATTR_ROW_COUNT, p_err);
   }
   if(firstintfield_listnum == 0)
	   return -1;

   sqlite3_exec(db , "begin transaction" , 0 , 0 ,&errMsg);

   while(j < firstintfield_listnum)
   {
       k = 0;
       l = 0;
   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select * from %s where %s = '%d'",
		   tableName,firstintfieldname,firstintfieldvalue[j]);
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
			  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

   int single_field_listnum = 0;
   int intValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];
   textValueName textValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];

   int single_intValueTmp[ROW_SAME_NUMBER_MAX];
   textValueName single_textValueTmp[ROW_SAME_NUMBER_MAX];

   memset(intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
   memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

   while(k < column_number)
   {
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 0, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

       memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
       memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

	   if(rc == OCI_SUCCESS)
	   {
		   value_index = 0;

		   while(rc != OCI_NO_DATA)
		   {
			   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
			   {
				   OCIDefineByPos(p_sql, &p_dfn, p_err, k+1, (dvoid *) &(single_intValueTmp[value_index]),
					   (sb4) sizeof(int), (ub2)SQLT_INT, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
				   rc = OCIStmtFetch2(p_sql, p_err, 1, (ub2)OCI_FETCH_NEXT, 1, OCI_DEFAULT);
				   if(rc != OCI_NO_DATA)
					   intValueTmp[value_index][k] = single_intValueTmp[value_index];

				   }
				   else if(fieldtypeIndicator[k] == SQLITE_TEXT)
				   {
					   OCIDefineByPos(p_sql, &p_dfn, p_err, k+1, (dvoid *) (single_textValueTmp[value_index]),
					   (sb4) sizeof(textValueName), (ub2)SQLT_STR, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
					   rc = OCIStmtFetch2(p_sql, p_err, 1, (ub2)OCI_FETCH_NEXT, 1, OCI_DEFAULT);
					   if(rc != OCI_NO_DATA)
						   strncpy(textValueTmp[value_index][k], single_textValueTmp[value_index], strlen(single_textValueTmp[value_index]));
				   }
				   else
				   {
					   return -1;
				   }

				   value_index++;
			   }
			   if(k == 0)
			   {
				   OCIAttrGet((const void *)p_sql, OCI_HTYPE_STMT, (void *) &single_field_listnum,
				   (ub4 *) sizeof(single_field_listnum), OCI_ATTR_ROW_COUNT, p_err);
				   if(single_field_listnum == 0)
				   {
					   return -1;
				   }
			   }
		   }

	   k++;
   }

   k = 0;

   while(l < single_field_listnum)
   {
	   char fieldvaluetmp[COLUMN_NAME_LEN*2];
	   k = 0;
	   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
	   memset(fieldvaluelist, 0, COLUMN_NAME_LIST_LEN);
	   while(k < column_number )
	   {
		   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
		   if(k != column_number - 1)
		   {
			   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
			   {
				   sprintf(fieldvaluetmp, "'%d',",intValueTmp[l][k]);
			   }
			   if(fieldtypeIndicator[k] == SQLITE_TEXT)
			   {
				   sprintf(fieldvaluetmp, "'%s',",textValueTmp[l][k]);
			   }
		   }
		   else
		   {
			   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
			   {
				   sprintf(fieldvaluetmp, "'%d'",intValueTmp[l][k]);
			   }
			   if(fieldtypeIndicator[k] == SQLITE_TEXT)
			   {
				   sprintf(fieldvaluetmp, "'%s'",textValueTmp[l][k]);
			   }
		   }
		   strcat(fieldvaluelist, fieldvaluetmp);
		   k++;
	   }
	   memset(sqlstr, 0, MAX_SQL_STR_LEN);
       sprintf(sqlstr, "insert into %s(%s) values(%s)",
		   tableName,fieldnamelist,fieldvaluelist);
	   rc = sqlite3_exec(db, sqlstr, 0, 0, &errMsg);
       l++;
   }

   j++;
   }
   sqlite3_exec(db , "commit transaction" , 0 , 0 , &errMsg);
//   DLOG("import from oracle to sqlite sucess! %s->data number:%d\n",tableName,row_number);
   printf("import from oracle to sqlite sucess! %s->data number:%d\n",tableName,row_number);

   return 0;
}


int ats_co_import_O2S_ImportData_for_all (sqlite3 *db, char* tnsName, char* userName, char* password)
{
   int rc = 0;
   char mysql[MAX_SQL_STR_LEN];
   textValueName   tablelist[TABLE_NUMBER_MAX];

   memset(tablelist, 0, sizeof(textValueName)*TABLE_NUMBER_MAX);

   rc = ats_co_import_Connect_to_Oracle (tnsName, userName, password);
   if(rc == -1)
	   return -1;

   rc = OCIHandleAlloc((dvoid *) p_env, (dvoid **) &p_sql,
           OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);

   memset(mysql, 0, MAX_SQL_STR_LEN);
   sprintf(mysql, "select table_name from user_tables");
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
			 (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 0, (ub4) 0,
			 (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

   int list_index = 0;
   int list_number = 0;
   if(rc == OCI_SUCCESS)
   {
	   while(rc != OCI_NO_DATA)
	   {
		   OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) tablelist[list_index],
			 (sb4) sizeof(textValueName), (ub2)SQLT_STR, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
		   rc = OCIStmtFetch2(p_sql, p_err, 1, (ub2)OCI_FETCH_NEXT, 1, OCI_DEFAULT);
		   list_index++;
	   }
	   OCIAttrGet((const void *)p_sql, OCI_HTYPE_STMT, (void *) &list_number,
    		   (ub4 *) sizeof(list_number), OCI_ATTR_ROW_COUNT, p_err);
   }
   if(list_number == 0)
   {
	   DLOG("oracle has no table!\n");
	   printf("oracle has no table!\n");
	   return -1;
   }

//   DLOG("begin to import from oracle to sqlite! oracle has %d table\n",list_number);
   printf("begin to import from oracle to sqlite! oracle has %d table\n",list_number);

   list_index = 0;
   while(list_index < list_number)
   {
	   ats_co_import_O2S_ImportData_for_tableName (db, tablelist[list_index]);
	   list_index++;
   }

  ats_co_import_DisConnect_to_Oracle();

  return 0;
}


int ats_co_import_S2O_ImportData_for_tableName_old (sqlite3 *db, char* tableName,int iscreatetable)
{

   int             rc;
   char            mysql[MAX_SQL_STR_LEN];
   char *errMsg = NULL;
   char sqlstr[MAX_SQL_STR_LEN];
   sqlite3_stmt *stmt;
   int  fieldname_sizep = 0;
   char fieldname[COLUMN_NAME_LEN];
   int  columntype[COLUMN_NUMBER_MAX];
   char fieldalllist[COLUMN_ALL_LIST_LEN];  //for create sql
   char fieldnamelist[COLUMN_NAME_LIST_LEN]; //for insert sql
   char fieldvaluelist[COLUMN_NAME_LIST_LEN];//for insert sql
   int  fieldtypeIndicator[COLUMN_NUMBER_MAX];
   char firstintfieldname[COLUMN_NAME_LEN];
   int  firstintfieldvalue[ROW_NUMBER_MAX];
   int row_number = 0;
   int column_number = 0;

   memset(fieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(columntype, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(fieldalllist, 0, sizeof(char)*COLUMN_ALL_LIST_LEN);
   memset(fieldnamelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldvaluelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldtypeIndicator, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(firstintfieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(firstintfieldvalue, 0, sizeof(int)*ROW_NUMBER_MAX);

   /* import to tableName*/
   int i = 0;
   int j = 0;
   int k = 0;
   int l = 0;
   char *stringtmp = NULL;

   if(iscreatetable == 1)
   {
	   //make up slq statement(create,insert) for oracle, we need column_name and column_type
       memset(sqlstr, 0, MAX_SQL_STR_LEN);
	   sprintf(sqlstr, "select * from %s", tableName);
	   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	   column_number = sqlite3_column_count(stmt);
	   if(column_number == 0)
		   return -1;

	   int primarykey_exist_flag = 0;
	   int primarykey_set_flag = 0;
	   int firstintfield_set_flag = 0;
	   char fieldnametmp[COLUMN_NAME_LEN];
	   char fieldalltmp[COLUMN_NAME_LEN + COLUMN_TYPE_LEN];
	   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
	   memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);

	   while(i < column_number)
	   {
		   char *fieldtypetmp = NULL;
		   fieldtypetmp = (char *)sqlite3_column_decltype(stmt, i);
		   if(fieldtypetmp != NULL)
		   {
			   if(strcmp(fieldtypetmp, "integer") == 0)
				   fieldtypeIndicator[i] = SQLITE_INTEGER;
			   if(strcmp(fieldtypetmp, "text") == 0)
				   fieldtypeIndicator[i] = SQLITE_TEXT;
		   }
		   else
			   return -1;

		   memset(fieldname, 0, COLUMN_NAME_LEN);
		   stringtmp = (char *)sqlite3_column_name(stmt, i);
		   strncpy(fieldname, stringtmp, strlen(stringtmp));

		   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
		   memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);
		   if(i != column_number - 1)
		   {
			   sprintf(fieldnametmp, "%s,",fieldname);

			   if(fieldtypeIndicator[i] == SQLITE_INTEGER)
			   {
				   if(primarykey_set_flag == 0)
				   {
					   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
					   if(primarykey_exist_flag == 1)
					   {
						   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"INTEGER");
						   primarykey_set_flag = 1;
					   }
					   else
						   sprintf(fieldalltmp, "%s %s,",fieldname,"INTEGER");
				   }
				   else
					   sprintf(fieldalltmp, "%s %s,",fieldname,"INTEGER");

				   if(firstintfield_set_flag == 0)
				   {
					   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
					   strncpy(firstintfieldname, fieldname, strlen(fieldname));
					   firstintfield_set_flag = 1;
				   }
			   }
			   else
			   {
				   if(primarykey_set_flag == 0)
				   {
					   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
					   if(primarykey_exist_flag == 1)
					   {
						   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"VARCHAR2(100)");
						   primarykey_set_flag = 1;
					   }
					   else
						   sprintf(fieldalltmp, "%s %s,",fieldname,"VARCHAR2(100)");
				   }
				   else
					   sprintf(fieldalltmp, "%s %s,",fieldname,"VARCHAR2(100)");
			   }
		   }
		   else
		   {
			   sprintf(fieldnametmp, "%s",fieldname);

			    if(fieldtypeIndicator[i] == SQLITE_INTEGER)
			   {
				   if(primarykey_set_flag == 0)
				   {
					   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
					   if(primarykey_exist_flag == 1)
					   {
						   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"INTEGER");
						   primarykey_set_flag = 1;
					   }
					   else
						   sprintf(fieldalltmp, "%s %s",fieldname,"INTEGER");
				   }
				   else
					   sprintf(fieldalltmp, "%s %s",fieldname,"INTEGER");

				   if(firstintfield_set_flag == 0)
				   {
					   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
					   strncpy(firstintfieldname, fieldname, strlen(fieldname));
					   firstintfield_set_flag = 1;
				   }
			   }
			   else
			   {
				   if(primarykey_set_flag == 0)
				   {
					   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
					   if(primarykey_exist_flag == 1)
					   {
						   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"VARCHAR2(100)");
						   primarykey_set_flag = 1;
					   }
					   else
						   sprintf(fieldalltmp, "%s %s",fieldname,"VARCHAR2(100)");
				   }
				   else
					   sprintf(fieldalltmp, "%s %s",fieldname,"VARCHAR2(100)");
			   }
		   }
		   strcat(fieldnamelist, fieldnametmp);
		   strcat(fieldalllist, fieldalltmp);

		   i++;
	   }
       sqlite3_finalize(stmt);

	   //drop old table and create new table in oracle
	   memset(mysql, 0, MAX_SQL_STR_LEN);
	   sprintf(mysql, "drop table %s", tableName);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
	   memset(mysql, 0, MAX_SQL_STR_LEN);
	   sprintf(mysql, "create table %s(%s)", tableName,fieldalllist);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   }
   else
   {
	   memset(mysql, 0, MAX_SQL_STR_LEN);
	   sprintf(mysql, "delete from %s", tableName);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   }


   //begin to get data from sqlite
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select count(*) from %s",tableName);
   sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);
   if (SQLITE_ROW == rc)
   {
	   row_number = sqlite3_column_int(stmt, 0);
   }
   else
   {
	   sqlite3_finalize(stmt);
	   return -1;
   }
   if(row_number == 0)
   {
	   sqlite3_finalize(stmt);
	   printf("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);
   	   return -1;
   }
   sqlite3_finalize(stmt);

   int firstintfield_listnum = 0;
   int value_index = 0;
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select distinct %s from %s order by %s",firstintfieldname,tableName,firstintfieldname);
   sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);
   while (SQLITE_ROW == rc)
   {
	   firstintfieldvalue[value_index] = sqlite3_column_int(stmt, 0);
	   value_index++;
	   rc = sqlite3_step(stmt);
   }
   sqlite3_finalize(stmt);
   firstintfield_listnum = value_index;
   if(firstintfield_listnum == 0)
	   return -1;


   while(j < firstintfield_listnum)
   {
       k = 0;
       l = 0;
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select * from %s where %s = '%d'",
		   tableName,firstintfieldname,firstintfieldvalue[j]);
   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);

   int single_field_listnum = 0;
   int intValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];
   textValueName textValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];

   int single_intValueTmp[ROW_SAME_NUMBER_MAX];
   textValueName single_textValueTmp[ROW_SAME_NUMBER_MAX];

   memset(intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
   memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

   while(k < column_number)
   {
       memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
       memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

	   value_index = 0;
       rc = sqlite3_step(stmt);
	   while (SQLITE_ROW == rc)
	   {
		   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
		   {
			   single_intValueTmp[value_index] = sqlite3_column_int(stmt, k);
			   intValueTmp[value_index][k] = single_intValueTmp[value_index];

		   }
		   else if(fieldtypeIndicator[k] == SQLITE_TEXT)
		   {
			   stringtmp = (char *)sqlite3_column_text(stmt, k);
		       strncpy(single_textValueTmp[value_index], stringtmp, strlen(stringtmp));
			   strncpy(textValueTmp[value_index][k], single_textValueTmp[value_index], strlen(single_textValueTmp[value_index]));
		   }
		   else
			   return -1;

		   value_index++;
		   rc = sqlite3_step(stmt);
	   }

	   if(k == 0)
	   {
		   single_field_listnum = value_index;
		   if(single_field_listnum == 0)
		   	   return -1;
	   }

	   k++;
   }
   sqlite3_finalize(stmt);

   k = 0;
   while(l < single_field_listnum)
   {
	   char fieldvaluetmp[COLUMN_NAME_LEN*2];
	   k = 0;
	   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
	   memset(fieldvaluelist, 0, COLUMN_NAME_LIST_LEN);
	   while(k < column_number )
	   {
		   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
		   if(k != column_number - 1)
		   {
			   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
			   {
				   sprintf(fieldvaluetmp, "'%d',",intValueTmp[l][k]);
			   }
			   if(fieldtypeIndicator[k] == SQLITE_TEXT)
			   {
				   sprintf(fieldvaluetmp, "'%s',",textValueTmp[l][k]);
			   }
		   }
		   else
		   {
			   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
			   {
				   sprintf(fieldvaluetmp, "'%d'",intValueTmp[l][k]);
			   }
			   if(fieldtypeIndicator[k] == SQLITE_TEXT)
			   {
				   sprintf(fieldvaluetmp, "'%s'",textValueTmp[l][k]);
			   }
		   }
		   strcat(fieldvaluelist, fieldvaluetmp);
		   k++;
	   }
	   memset(mysql, 0, MAX_SQL_STR_LEN);
       sprintf(mysql, "insert into %s(%s) values (%s)",
		   tableName,fieldnamelist,fieldvaluelist);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
       l++;
   }

   j++;
   }
   OCITransCommit(p_svc, p_err,(ub4)0);
   printf("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);

   return 0;
}


int ats_co_import_S2O_ImportData_for_tableName (sqlite3 *db, char* tableName,int iscreatetable)
{

   int             rc;
   char            mysql[MAX_SQL_STR_LEN];
   char *errMsg = NULL;
   char sqlstr[MAX_SQL_STR_LEN];
   sqlite3_stmt *stmt;
   int  fieldname_sizep = 0;
   char fieldname[COLUMN_NAME_LEN];
   int  columntype[COLUMN_NUMBER_MAX];
   char fieldalllist[COLUMN_ALL_LIST_LEN];  //for create sql
   char fieldnamelist[COLUMN_NAME_LIST_LEN]; //for insert sql
   char fieldvaluelist[COLUMN_NAME_LIST_LEN];//for insert sql
   int  fieldtypeIndicator[COLUMN_NUMBER_MAX];
   char firstintfieldname[COLUMN_NAME_LEN];
   int  firstintfieldvalue[ROW_NUMBER_MAX];






   int row_number = 0;
   int column_number = 0;
   textValueName blob_field_name[COLUMN_NUMBER_MAX];

   memset(fieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(columntype, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(fieldalllist, 0, sizeof(char)*COLUMN_ALL_LIST_LEN);
   memset(fieldnamelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldvaluelist, 0, sizeof(char)*COLUMN_NAME_LIST_LEN);
   memset(fieldtypeIndicator, 0, sizeof(int)*COLUMN_NUMBER_MAX);
   memset(firstintfieldname, 0, sizeof(char)*COLUMN_NAME_LEN);
   memset(firstintfieldvalue, 0, sizeof(int)*ROW_NUMBER_MAX);
   memset(blob_field_name, 0, sizeof(textValueName)*COLUMN_NUMBER_MAX);


   /* import to tableName*/
   int i = 0;
   int j = 0;
   int k = 0;
   int l = 0;
   char *stringtmp = NULL;


   //make up slq statement(create,insert) for oracle, we need column_name and column_type
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select * from %s", tableName);
   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   column_number = sqlite3_column_count(stmt);
   if(column_number == 0)
	   return -1;

   int primarykey_exist_flag = 0;
   int primarykey_set_flag = 0;
   int firstintfield_set_flag = 0;
   char fieldnametmp[COLUMN_NAME_LEN];
   char fieldalltmp[COLUMN_NAME_LEN + COLUMN_TYPE_LEN];
   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
   memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);
   int blob_column_count = 0;
   while(i < column_number)
   {
	   char *fieldtypetmp = NULL;
	   fieldtypetmp = (char *)sqlite3_column_decltype(stmt, i);
	   if(fieldtypetmp != NULL)
	   {
		   if(strcmp(fieldtypetmp, "integer") == 0 || strcmp(fieldtypetmp, "INTEGER") == 0)
			   fieldtypeIndicator[i] = SQLITE_INTEGER;
		   if(strcmp(fieldtypetmp, "text") == 0 || strcmp(fieldtypetmp, "TEXT") == 0)
			   fieldtypeIndicator[i] = SQLITE_TEXT;
		   if(strcmp(fieldtypetmp, "blob") == 0 || strcmp(fieldtypetmp, "BLOB") == 0)
		   {

			   fieldtypeIndicator[i] = SQLITE_BLOB;
//			   ILOG("T_INTERLOCKDATA Updata blob_column_count fieldtypeIndicator[%d] = SQLITE_BLOB",i);
		   }
	   }
	   else{
		   ILOG("T_INTERLOCKDATA Updata error");
		   return -1;
	   }

	   //DLOG("fieldtypeIndicator[%d] = %d",i,fieldtypeIndicator[i]);

	   memset(fieldname, 0, COLUMN_NAME_LEN);
	   stringtmp = (char *)sqlite3_column_name(stmt, i);
	   strncpy(fieldname, stringtmp, strlen(stringtmp));

	   memset(fieldnametmp, 0, COLUMN_NAME_LEN);
	   memset(fieldalltmp, 0, COLUMN_NAME_LEN + COLUMN_TYPE_LEN);
	   if(i != column_number - 1)
	   {
		   sprintf(fieldnametmp, "%s,",fieldname);


		   if(fieldtypeIndicator[i] == SQLITE_INTEGER)
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"INTEGER");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s,",fieldname,"INTEGER");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"INTEGER");

			   if(firstintfield_set_flag == 0)
			   {
				   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
				   strncpy(firstintfieldname, fieldname, strlen(fieldname));
				   firstintfield_set_flag = 1;
				   //DLOG("firstintfieldname : %s",firstintfieldname);
			   }
		   }
		   else if(fieldtypeIndicator[i] == SQLITE_TEXT)
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"VARCHAR2(200)");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s,",fieldname,"VARCHAR2(200)");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"VARCHAR2(200)");
		   }
		   else
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null,",fieldname,"BLOB");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s,",fieldname,"BLOB");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s,",fieldname,"BLOB");

			   if(blob_column_count < COLUMN_BLOB_NUMBER_MAX)
			   {
				   strncpy(blob_field_name[i], fieldname, strlen(fieldname));
//				   ILOG("T_INTERLOCKDATA Updata blob_column_count fieldname[%s]",fieldname);
				   blob_column_count++;
			   }
		   }
	   }
	   else
	   {
		   sprintf(fieldnametmp, "%s",fieldname);


		    if(fieldtypeIndicator[i] == SQLITE_INTEGER)
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"INTEGER");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s",fieldname,"INTEGER");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"INTEGER");

			   if(firstintfield_set_flag == 0)
			   {
				   memset(firstintfieldname, 0, COLUMN_NAME_LEN);
				   strncpy(firstintfieldname, fieldname, strlen(fieldname));
				   firstintfield_set_flag = 1;
			   }
		   }
		   else if(fieldtypeIndicator[i] == SQLITE_TEXT)
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"VARCHAR2(200)");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s",fieldname,"VARCHAR2(200)");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"VARCHAR2(200)");
		   }
		    else
		   {
			   if(primarykey_set_flag == 0)
			   {
				   sqlite3_table_column_metadata(db,NULL,tableName,fieldname,NULL,NULL,NULL,&primarykey_exist_flag,NULL);
				   if(primarykey_exist_flag == 1)
				   {
					   sprintf(fieldalltmp, "%s %s primary key not null",fieldname,"BLOB");
					   primarykey_set_flag = 1;
				   }
				   else
					   sprintf(fieldalltmp, "%s %s",fieldname,"BLOB");
			   }
			   else
				   sprintf(fieldalltmp, "%s %s",fieldname,"BLOB");

			   if(blob_column_count < COLUMN_BLOB_NUMBER_MAX)
			   {
				   strncpy(blob_field_name[i], fieldname, strlen(fieldname));
//				   ILOG("T_INTERLOCKDATA Updata blob_column_count fieldname[%s]",fieldname);
				   blob_column_count++;
			   }
		   }
	   }
	   strcat(fieldnamelist, fieldnametmp);
	   strcat(fieldalllist, fieldalltmp);

	   i++;
   }
   sqlite3_finalize(stmt);

    if(iscreatetable == 1)
    {
	   //drop old table and create new table in oracle
	   memset(mysql, 0, MAX_SQL_STR_LEN);
	   sprintf(mysql, "drop table %s", tableName);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
	   memset(mysql, 0, MAX_SQL_STR_LEN);
	   sprintf(mysql, "create table %s(%s)", tableName,fieldalllist);
	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
   }
//   else
//   {
//	   memset(mysql, 0, MAX_SQL_STR_LEN);
//	   sprintf(mysql, "delete from %s", tableName);
//	   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
//				  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
//	   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
//				  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
//   }


   //begin to get data from sqlite
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select count(*) from %s",tableName);
   sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);
   if (SQLITE_ROW == rc)
   {
	   row_number = sqlite3_column_int(stmt, 0);
   }
   else
   {
	   sqlite3_finalize(stmt);
	   return -1;
   }

   if(row_number == 0)
   {
	   sqlite3_finalize(stmt);
	   printf("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);
	   //DLOG("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);
   	   return -1;
   }
   sqlite3_finalize(stmt);

   int firstintfield_listnum = 0;
   int value_index = 0;
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select distinct %s from %s order by %s",firstintfieldname,tableName,firstintfieldname);
   sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);
   while (SQLITE_ROW == rc)
   {
	   firstintfieldvalue[value_index] = sqlite3_column_int(stmt, 0);
	   value_index++;
	   rc = sqlite3_step(stmt);
   }
   sqlite3_finalize(stmt);
   firstintfield_listnum = value_index;
   if(firstintfield_listnum == 0)
	   return -1;


   long intValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];
   textValueName textValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];

   long single_intValueTmp[ROW_SAME_NUMBER_MAX];
   textValueName single_textValueTmp[ROW_SAME_NUMBER_MAX];

   memset(intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX*COLUMN_NUMBER_MAX);
   memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
   memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

//   ILOG("T_INTERLOCKDATA Updata blob_column_count =%d",blob_column_count);
   if(blob_column_count > 0)
   {

	   while(j < firstintfield_listnum)
	   {
		   k = 0;
		   l = 0;
		   memset(sqlstr, 0, MAX_SQL_STR_LEN);
		   sprintf(sqlstr, "select * from %s where %s = '%d'",
				   tableName,firstintfieldname,firstintfieldvalue[j]);
		   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);

		   int single_field_listnum = 0;

		   blobFieldValue *blobValueTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX];
		   int blobLenTmp[ROW_SAME_NUMBER_MAX][COLUMN_NUMBER_MAX]={0};

		   int malloc_blob_count_row = 0;
		   int malloc_blob_count_column = 0;
		   while(malloc_blob_count_row < ROW_SAME_NUMBER_MAX)
		   {
			   malloc_blob_count_column = 0;
			   while(malloc_blob_count_column < COLUMN_NUMBER_MAX)
			   {
				   blobValueTmp[malloc_blob_count_row][malloc_blob_count_column] = (blobFieldValue *)malloc(sizeof(blobFieldValue));
				   memset(blobValueTmp[malloc_blob_count_row][malloc_blob_count_column], 0, sizeof(blobFieldValue));

				   malloc_blob_count_column++;
			   }
			   malloc_blob_count_row++;
		   }

		   blobFieldValue *single_blobValueTmp[ROW_SAME_NUMBER_MAX];
		   int malloc_single_blob_count = 0;
		   while(malloc_single_blob_count < ROW_SAME_NUMBER_MAX)
		   {
			   single_blobValueTmp[malloc_single_blob_count]  = (blobFieldValue *)malloc(sizeof(blobFieldValue));
			   memset(single_blobValueTmp[malloc_single_blob_count], 0, sizeof(blobFieldValue));
			   malloc_single_blob_count++;
		   }
           //DLOG("column_number = %d",column_number);
		   if(k < column_number)
		   {
			   memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
			   memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);

			   malloc_single_blob_count = 0;
			   while(malloc_single_blob_count < ROW_SAME_NUMBER_MAX)
			   {
				   memset(single_blobValueTmp[malloc_single_blob_count], 0, sizeof(blobFieldValue));
				   malloc_single_blob_count++;
			   }
			   value_index = 0;
			   rc = sqlite3_step(stmt);
			   while (SQLITE_ROW == rc)
			   {
				   k = 0;
				   while(k < column_number)
				   {
				   //DLOG("fieldtypeIndicator[%d] = %d",k,fieldtypeIndicator[k]);
				   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
				   {
					   single_intValueTmp[value_index] = sqlite3_column_int64(stmt, k);
					   intValueTmp[value_index][k] = single_intValueTmp[value_index];
				   }
				   else if(fieldtypeIndicator[k] == SQLITE_TEXT)
				   {
					   stringtmp = (char *)sqlite3_column_text(stmt, k);
					   strncpy(single_textValueTmp[value_index], stringtmp, strlen(stringtmp));
					   strncpy(textValueTmp[value_index][k], single_textValueTmp[value_index], strlen(single_textValueTmp[value_index]));
				   }
				   else
				   {
					   stringtmp = (char *)sqlite3_column_blob(stmt, k);
					   int get_blob_size = sqlite3_column_bytes(stmt, k);
					   memcpy((single_blobValueTmp[value_index]), stringtmp, get_blob_size);
					   memcpy((blobValueTmp[value_index][k]), (single_blobValueTmp[value_index]), get_blob_size);
					   blobLenTmp[value_index][k]=get_blob_size;
					   //ILOG("blobLenTmp get_blob_size=%d",blobLenTmp[value_index][k]);
	   				}

				   k++;
				   }
				   value_index++;
				   rc = sqlite3_step(stmt);


			   }

			   if(k >= 0)
			   {
				   single_field_listnum = value_index;
				   //DLOG("value_index = %d",value_index);
				   if(single_field_listnum == 0)
	   				   return -1;
			   }
		   }
		   sqlite3_finalize(stmt);



		   k = 0;
		   while(l < single_field_listnum)
		   {
			   char fieldvaluetmp[COLUMN_NAME_LEN*2];
			   k = 0;
			   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
			   memset(fieldvaluelist, 0, COLUMN_NAME_LIST_LEN);
			   while(k < column_number )
			   {
				   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
				   if(k != column_number - 1)
				   {
					   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
					   {
						   sprintf(fieldvaluetmp, "'%ld',",intValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_TEXT)
					   {
						   sprintf(fieldvaluetmp, "'%s',",textValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_BLOB)
					   {
						   sprintf(fieldvaluetmp, "empty_blob(),");
					   }
				   }
				   else
				   {
					   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
					   {
						   sprintf(fieldvaluetmp, "'%ld'",intValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_TEXT)
					   {
						   sprintf(fieldvaluetmp, "'%s'",textValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_BLOB)
					   {
						   sprintf(fieldvaluetmp, "empty_blob()");
					   }
				   }
				   strcat(fieldvaluelist, fieldvaluetmp);
				   k++;
			   }
			   memset(mysql, 0, MAX_SQL_STR_LEN);
			   sprintf(mysql, "insert into %s(%s) values (%s)",
				   tableName,fieldnamelist,fieldvaluelist);
//			   DLOG("insert into %s(%s) values (%s)",
//			   				   tableName,fieldnamelist,fieldvaluelist);
			   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
						  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
//			   ILOG("T_INTERLOCKDATA Updata blob rc1=%d",rc);


			   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
						  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
//			   ILOG("T_INTERLOCKDATA Updata blob rc2=%d",rc);

			   if(blob_column_count > 0)
			   {
				   OCILobLocator *p_lob[ROW_SAME_NUMBER_MAX];
				   int p_lob_index = 0;
				   while(p_lob_index < ROW_SAME_NUMBER_MAX)
				   {
					   rc = OCIDescriptorAlloc((dvoid *) p_env, (dvoid **) &(p_lob[p_lob_index]),
							 OCI_DTYPE_LOB, (size_t) 0, (dvoid **) 0);
					   p_lob_index++;
				   }
				   p_lob_index = 0;
				   int blob_column_index = 0;
				   /*while(blob_column_index < column_number)
				   {
					   if(fieldtypeIndicator[blob_column_index] == SQLITE_BLOB)
					   {
						  memset(mysql, 0, MAX_SQL_STR_LEN);
						  sprintf(mysql, "select %s from %s where %s = '%d'",
								  blob_field_name[blob_column_index],
								  tableName,
								  firstintfieldname,
								  firstintfieldvalue[j]);
						  rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
							   (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
						  OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) &p_lob,
	      						 (sb4) 0, (ub2)SQLT_BLOB, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
						  rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
							   (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
						  int offset = 1;
						  ub4 amtp = BLOB_SIZE;
						  OCILobWrite(p_svc, p_err, p_lob, &amtp, offset,(dvoid *) blobValueTmp[l][blob_column_index],
								  (ub4)BLOB_SIZE, OCI_ONE_PIECE,(dvoid *) 0,0,(ub2)0,(ub1)SQLCS_IMPLICIT);
					   }
					   blob_column_index++;
				   }*/

				   while(blob_column_index < column_number)
				   {
					   if(fieldtypeIndicator[blob_column_index] == SQLITE_BLOB)
					   {
						  memset(mysql, 0, MAX_SQL_STR_LEN);
						  sprintf(mysql, "select %s from %s where %s = '%d'",
								  blob_field_name[blob_column_index],
								  tableName,
								  firstintfieldname,
								  firstintfieldvalue[j]);
						  rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
							   (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
						  rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 0, (ub4) 0,
							   (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

						  if(rc == OCI_SUCCESS)
						  {
							  p_lob_index = 0;
							  while(rc != OCI_NO_DATA)
							  {
								  OCIDefineByPos(p_sql, &p_dfn, p_err, 1, (dvoid *) &(p_lob[p_lob_index]),
									  (sb4) 0, (ub2)SQLT_BLOB, (dvoid *) 0, (ub2 *)0,(ub2 *)0, OCI_DEFAULT);
								  rc = OCIStmtFetch2(p_sql, p_err, 1, (ub2)OCI_FETCH_NEXT, 1, OCI_DEFAULT);
								  if(rc==OCI_NO_DATA)
								  {
									  break;
								  }
								  int offset = 1;
								  //ub4 amtp = BLOB_SIZE;
								  //int result = OCILobWrite(p_svc, p_err, p_lob[p_lob_index], &amtp, offset,(dvoid *) blobValueTmp[p_lob_index][blob_column_index],
								  	//	  (ub4)BLOB_SIZE, OCI_ONE_PIECE,(dvoid *) 0,0,(ub2)0,(ub1)SQLCS_IMPLICIT);
								  ub4 amtp = blobLenTmp[p_lob_index][blob_column_index];
								  int result=OCILobWrite(p_svc, p_err, p_lob[p_lob_index], &amtp, offset,(dvoid *) blobValueTmp[p_lob_index][blob_column_index],
										  (ub4)BLOB_SIZE, OCI_ONE_PIECE,(dvoid *) 0,0,(ub2)0,(ub1)SQLCS_IMPLICIT);
								  //ILOG("OCILobWrite result =%d blobLenTmp =%d",result,blobLenTmp[p_lob_index][blob_column_index] );
								  p_lob_index++;

							  }
						  }
//						  int offset = 1;
//						  ub4 amtp = BLOB_SIZE;
//						  OCILobWrite(p_svc, p_err, p_lob[l], &amtp, offset,(dvoid *) blobValueTmp[l][blob_column_index],
//								  (ub4)BLOB_SIZE, OCI_ONE_PIECE,(dvoid *) 0,0,(ub2)0,(ub1)SQLCS_IMPLICIT);
					   }
					   blob_column_index++;
				   }

			   }
			   l++;
		   }

		   malloc_single_blob_count = 0;
		   if(single_blobValueTmp != NULL)
		   {
			   while(malloc_single_blob_count < ROW_SAME_NUMBER_MAX)
			   {
				   if(single_blobValueTmp[malloc_single_blob_count] != NULL)
					   free(single_blobValueTmp[malloc_single_blob_count]);
				   malloc_single_blob_count++;
			   }
			   /*if(single_blobValueTmp != NULL)
				   free(single_blobValueTmp);*/
		   }
		   malloc_blob_count_row = 0;
		   malloc_blob_count_column = 0;
		   if(blobValueTmp != NULL)
		   {
				while(malloc_blob_count_row < ROW_SAME_NUMBER_MAX)
				{
					malloc_blob_count_column = 0;
					while(malloc_blob_count_column < COLUMN_NUMBER_MAX)
					{
						if(blobValueTmp[malloc_blob_count_row][malloc_blob_count_column] != NULL)
							free(blobValueTmp[malloc_blob_count_row][malloc_blob_count_column]);
						malloc_blob_count_column++;
					}
					malloc_blob_count_row++;
			   }
			   /*int columun_count = 0;
			   while(columun_count < COLUMN_NUMBER_MAX)
			   {
				   if(blobValueTmp[columun_count] != NULL)
					   free(blobValueTmp[columun_count]);
				   columun_count++;
			   }*/
		   }

		   j++;


	   }

   }
   else
   {
//	   ILOG("T_INTERLOCKDATA Updata blob <0");
	   while(j < firstintfield_listnum)
	   {
		   k = 0;
		   l = 0;
		   memset(sqlstr, 0, MAX_SQL_STR_LEN);
		   sprintf(sqlstr, "select * from %s where %s = '%d'",
				   tableName,firstintfieldname,firstintfieldvalue[j]);
		   rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);

		   int single_field_listnum = 0;

		   if(k < column_number)
		   {
			   memset(single_intValueTmp, 0, sizeof(int)*ROW_SAME_NUMBER_MAX);
			   memset(single_textValueTmp, 0, sizeof(textValueName)*ROW_SAME_NUMBER_MAX);


			   value_index = 0;
			   rc = sqlite3_step(stmt);
			   while (SQLITE_ROW == rc)
			   {
				   while (k < column_number)
				   {
				   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
				   {
					   single_intValueTmp[value_index] = sqlite3_column_int(stmt, k);
					   intValueTmp[value_index][k] = single_intValueTmp[value_index];

				   }
				   else if(fieldtypeIndicator[k] == SQLITE_TEXT)
				   {
					   stringtmp = (char *)sqlite3_column_text(stmt, k);
					   strncpy(single_textValueTmp[value_index], stringtmp, strlen(stringtmp));
					   strncpy(textValueTmp[value_index][k], single_textValueTmp[value_index], strlen(single_textValueTmp[value_index]));
				   }
				   else
				   {
					  break;
	   				}

				   k++;
				   }
				   value_index++;
				   rc = sqlite3_step(stmt);
			   }

			   if(k >= 0)
			   {
				   single_field_listnum = value_index;
				   if(single_field_listnum == 0)
	   				   return -1;
			   }

		   }
		   sqlite3_finalize(stmt);



		   k = 0;
		   while(l < single_field_listnum)
		   {
			   char fieldvaluetmp[COLUMN_NAME_LEN*2];
			   k = 0;
			   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
			   memset(fieldvaluelist, 0, COLUMN_NAME_LIST_LEN);
			   while(k < column_number )
			   {
				   memset(fieldvaluetmp, 0, COLUMN_NAME_LEN*2);
				   if(k != column_number - 1)
				   {
					   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
					   {
						   sprintf(fieldvaluetmp, "'%ld',",intValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_TEXT)
					   {
						   sprintf(fieldvaluetmp, "'%s',",textValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_BLOB)
					   {
						   sprintf(fieldvaluetmp, "empty_blob(),");
					   }
				   }
				   else
				   {
					   if(fieldtypeIndicator[k] == SQLITE_INTEGER)
					   {
						   sprintf(fieldvaluetmp, "'%ld'",intValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_TEXT)
					   {
						   sprintf(fieldvaluetmp, "'%s'",textValueTmp[l][k]);
					   }
					   if(fieldtypeIndicator[k] == SQLITE_BLOB)
					   {
						   sprintf(fieldvaluetmp, "empty_blob()");
					   }
				   }
				   strcat(fieldvaluelist, fieldvaluetmp);
				   k++;
			   }
			   memset(mysql, 0, MAX_SQL_STR_LEN);
			   sprintf(mysql, "insert into %s(%s) values (%s)",
				   tableName,fieldnamelist,fieldvaluelist);
			   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
						  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
			   ILOG("T_INTERLOCKDATA Updata rc1=%d",rc);
			   rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
						  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
			   ILOG("T_INTERLOCKDATA Updata rc2=%d",rc);

			   l++;
		   }


		   j++;


	   }
   }

   OCITransCommit(p_svc, p_err,(ub4)0);
   printf("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);
   //DLOG("import from sqlite to oracle sucess! %s data number:%d\n",tableName,row_number);

   return 0;
}


int ats_co_import_S2O_ImportData_for_all (sqlite3 *db, char* tnsName, char* userName, char* password,int iscreatetable)
{
   int rc = 0;
   char *stringtmp = NULL;
   char sqlstr[MAX_SQL_STR_LEN];
   sqlite3_stmt *stmt;
   textValueName   tablelist[TABLE_NUMBER_MAX];

   memset(tablelist, 0, sizeof(textValueName)*TABLE_NUMBER_MAX);

   rc = ats_co_import_Connect_to_Oracle (tnsName, userName, password);
   if(rc == -1)
	   return -1;

   int list_index = 0;
   int list_number = 0;
   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "select name from sqlite_master where type = 'table'");
   sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
   rc = sqlite3_step(stmt);
   while (SQLITE_ROW == rc)
   {
	   stringtmp = (char *)sqlite3_column_text(stmt, 0);
	   if(stringtmp != NULL)
	   {
		   strncpy(tablelist[list_index], stringtmp, strlen(stringtmp));
	   }
	   list_index++;
	   rc = sqlite3_step(stmt);
   }
   sqlite3_finalize(stmt);

   list_number = list_index;
   if(list_number == 0)
   {
	   printf("sqlite has no table!\n");
	   return -1;
   }
   printf("begin to import from sqlite to oracle! sqlite has %d table\n",list_number);

   list_index = 0;
   while(list_index < list_number)
   {
	   if(strcmp(tablelist[list_index], "sqlite_sequence") != 0)
		   ats_co_import_S2O_ImportData_for_tableName (db, tablelist[list_index],iscreatetable);
	   list_index++;
   }

  ats_co_import_DisConnect_to_Oracle();

  return 0;
}

int ats_co_import_update_cas_status()
{
	const char *mysql="update T_CASSTATUS set CAS_STATUS=1";
	int rc=0;

	rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)mysql,
			  (ub4) strlen(mysql), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	if(rc!=OCI_SUCCESS)
	{
		DLOG("OCIStmtPrepare rc=%d",rc);
		return -1;
	}
	rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
			  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
	if(rc!=OCI_SUCCESS)
	{
		DLOG("OCIStmtExecute rc=%d",rc);
		return -1;
	}

}
/*******************************************************************
 Function name			ats_co_import_S2O_ImportData_for_replayInterlock
 description			insert the replay interlock data into the oracle
 parameter
 sqlite3 *         IN/OUT           db
 Return value
 true or false

 *******************************************************************/
int ats_co_import_S2O_ImportData_for_replayInterlock (sqlite3 *db)
{
	if (NULL == db)
	{
		ELOG("get ImportData replay data failed: db is null");
		return -1;
	}
	int rc;
	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN]={0};

	//sqlstr="select from T_INTERLOCKDATA";
	sprintf(sqlstr, "select t.DATATYPE,t.ELEMENTTYPE,t.ELEMENTINDEX,t.TIME,t.DATA from T_INTERLOCKDATA t order by t.REPLAYINDEX asc");
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: rc=%d,%s while get from T_INTERLOCKDATA",SQLITE_OK, sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return -1;
	}
	memset(sqlstr, 0, MAX_SQL_STR_LEN);
	int elementType=0;
	int elementIndex=0;
	int dataType=0;
	long time=0;
	char *stringtmp =NULL;

	ub4 get_blob_size =0 ;

	int timeout =90;
	int result=1;

	OCITransStart  (p_svc, p_err,timeout,OCI_TRANS_NEW );

	int number=0;
	rc = sqlite3_step(stmt);
	int errorCode=0;
	char errorBuf[200]={0};
	while ((SQLITE_ROW == rc))
	{
		number++;
		dataType=sqlite3_column_int(stmt, 0);
		elementType = sqlite3_column_int(stmt, 1);
		elementIndex = sqlite3_column_int(stmt, 2);
		time = sqlite3_column_int64(stmt, 3);
		stringtmp = (char *)sqlite3_column_blob(stmt, 4);
	    get_blob_size = sqlite3_column_bytes(stmt, 4);
	    char stringOutput[512] ={0};
	    //memset(stringOutput, 0, 512);

	    byte_to_hex(stringOutput,stringtmp,get_blob_size);
	    //DLOG("Replay elementType=%d,size=%d,data=%s",elementType,get_blob_size,stringOutput);
		sprintf(sqlstr, "insert into T_INTERLOCKDATA(DATATYPE,ELEMENTTYPE,ELEMENTINDEX,TIME,DATA) values (%d,%d,%d,%ld,%s)",
				dataType,elementType,elementIndex,time,":1");
		rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)sqlstr,
			   (ub4) strlen(sqlstr), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIStmtPrepare ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		//char *tmp ='01020304050607080910111213141516171819202122232425';
		rc = OCIBindByPos(p_sql,&p_bind,p_err,1,(dvoid *)stringOutput,get_blob_size*2,
				(ub4)SQLT_LNG,0,0,0,0,0,OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIBindByPos ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
			   (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
		DLOG("OCIStmtExecute result =%d ",rc);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIStmtExecute ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		rc = sqlite3_step(stmt);
	}
	DLOG("import Data from T_INTERLOCKDATA,data number =%d",number);
	sqlite3_finalize(stmt);
	if(result ==1)
	{
		rc =OCITransCommit(p_svc,p_err,OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCITransCommit ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			FLOG("import T_INTERLOCKDATA Data error");
		}

	}
	else
	{
		FLOG("import T_INTERLOCKDATA Data error");
		OCITransRollback(p_svc,p_err,OCI_DEFAULT);
	}
	return result;

}
/*******************************************************************
 Function name			ats_co_import_S2O_ImportData_for_replayTMT
 description			insert the replay TMT data into the oracle
 parameter
 sqlite3 *         IN/OUT           db
 Return value
 true or false

 *******************************************************************/
int ats_co_import_S2O_ImportData_for_replayTMT (sqlite3 *db)
{
	if (NULL == db)
	{
		ELOG("get ImportData replay data failed: db is null");
		return -1;
	}
	int rc;
	sqlite3_stmt *stmt;
	char sqlstr[MAX_SQL_STR_LEN]={0};

	//sqlstr="select from T_INTERLOCKDATA";
	sprintf(sqlstr, "select t.TRACKINDEX,t.STATUS,t.TIME,t.DATA from T_TMTDATA t order by t.REPLAYINDEX asc");
	rc = sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
	if (SQLITE_OK != rc)
	{
		ELOG("SQLerror: rc=%d,%s while get from T_TMTDATA",SQLITE_OK, sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return -1;
	}
	memset(sqlstr, 0, MAX_SQL_STR_LEN);

	int tracckIndex=0;
	int status=0;
	long time=0;

	char *stringtmp =NULL;

	ub4 get_blob_size =0 ;

	int timeout =90;
	int result=1;

	//TODO:oracle begin transcation
	OCITransStart  (p_svc, p_err,timeout,OCI_TRANS_NEW );

	int number=0;
	rc = sqlite3_step(stmt);
	int errorCode=0;
	char errorBuf[200]={0};
	while ((SQLITE_ROW == rc))
	{
		number++;
		tracckIndex=sqlite3_column_int(stmt, 0);
		status = sqlite3_column_int(stmt, 1);
		time = sqlite3_column_int64(stmt, 2);
		stringtmp = (char *)sqlite3_column_blob(stmt, 3);
	    get_blob_size = sqlite3_column_bytes(stmt, 3);
	    char stringOutput[512] ={0};
	    //memset(stringOutput, 0, 512);

	    byte_to_hex(stringOutput,stringtmp,get_blob_size);
	    //DLOG("Replay elementType=%d,size=%d,data=%s",elementType,get_blob_size,stringOutput);
		sprintf(sqlstr, "insert into T_TMTDATA(TRACKINDEX,STATUS,TIME,DATA) values (%d,%d,%ld,%s)",
				tracckIndex,status,time,":1");
		rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)sqlstr,
			   (ub4) strlen(sqlstr), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIStmtPrepare ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		//char *tmp ='01020304050607080910111213141516171819202122232425';
		rc = OCIBindByPos(p_sql,&p_bind,p_err,1,(dvoid *)stringOutput,get_blob_size*2,
				(ub4)SQLT_LNG,0,0,0,0,0,OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIBindByPos ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
			   (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
		DLOG("OCIStmtExecute result =%d ",rc);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCIStmtExecute ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			break;
		}
		rc = sqlite3_step(stmt);
	}
	DLOG("import Data from T_TMTDATA,data number =%d",number);
	sqlite3_finalize(stmt);
	//TODO:oracle end transcation
	if(result ==1)
	{
		rc =OCITransCommit(p_svc,p_err,OCI_DEFAULT);
		if(rc!=OCI_SUCCESS)
		{
			OCIErrorGet((void  *)p_err, (ub4) 1, (OraText*) NULL, &errorCode, (OraText *)errorBuf, (ub4)sizeof(errorBuf), (ub4)OCI_HTYPE_ERROR);
			DLOG("OCITransCommit ErrorCode=%d, Error =%s ",errorCode,errorBuf);
			result =-1;
			FLOG("import T_TMTDATA Data error");
		}
	}
	else
	{
		FLOG("import T_TMTDATA Data error");
		OCITransRollback(p_svc,p_err,OCI_DEFAULT);
	}
	return result;

}
/*******************************************************************
 Function name			byte_to_hex
 description			covert the byte to hex
 parameter
 unsigned char *         IN/OUT           db
 void *					 IN/OUT           msg
 int 					 IN				  msglen
 Return value
 true or false

 *******************************************************************/
void byte_to_hex(unsigned char *output, void* msg, int msglen)
{
	int flag = 0;
	if(msglen > 200)
	{
		msglen = 200;
		flag = 1;
	}
	unsigned char *p1 = output;
	unsigned char *p = (unsigned char *) msg;

	int i;;
	for (i = 0; i < msglen; i++)
	{
		sprintf((char *) p1, "%02x", *p);
		p1 += 2;
		p++;
	}

}
/*******************************************************************
 Function name			ats_co_import_init_replayData
 description			init the replay data into the oracle
 parameter
 char *         		 IN    	 	    tnsName
 char *         		 IN        	   	userName
 char *         		 IN       	    password
 Return value
 true or false

 *******************************************************************/
int ats_co_import_init_replayData (char* tnsName, char* userName, char* password)
{
   int rc = 0;
   char sqlstr[MAX_SQL_STR_LEN];


   rc = ats_co_import_Connect_to_Oracle (tnsName, userName, password);
   if(rc != OCI_SUCCESS)
   {
	   DLOG("ats_co_import_Connect_to_Oracle Connect fail rc=%d",rc);
	   return -1;
   }

   memset(sqlstr, 0, MAX_SQL_STR_LEN);
   sprintf(sqlstr, "delete from T_CHECKPOINTELEMENT ");
   rc = OCIStmtPrepare(p_sql, p_err, (unsigned char*)sqlstr,
  				  (ub4) strlen(sqlstr), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   if(rc != OCI_SUCCESS)
   {
	   DLOG("ats_co_import_Connect_to_Oracle OCIStmtPrepare fail rc=%d",rc);
   }
   else
   {
	    rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
	   			  (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
	    if(rc != OCI_SUCCESS)
	    {
		     DLOG("ats_co_import_Connect_to_Oracle OCIStmtExecute fail rc=%d",rc);
	    }
   }

   ats_co_import_DisConnect_to_Oracle();

  return 0;
}







