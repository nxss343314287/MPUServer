/**************************************************************************//**

 @brief      This file is used for implement ...
 @author     chenzl
 @date       2011-07-11
 @version    1.0
 
 @tablename{ Modify History }
 @tablehead{ mender,  date,        modify,      description }
 @tabledata{ chenzl,  2011-07-11,  All,         New issue.  }
 @tablefoot
 
 ******************************************************************************/

/* System Header Files */
/* None */

/* Project Header Files */
/*#include "public.h"*/
#include "log.h"
/*---------------------------- C/C++ shared code begin ----------------------*/

/*-- Local Defines --*/
log4c_category_t *cat;
/* None */

/*-- Local Marro Function --*/
/* None */

/*-- Local Typedefs --*/
/* None */

/*-- Global Variables --*/
#ifdef  USE_LOG4C
log4c_category_t *log_category_p;
#else
//static char log_category[NAME_MAX];
#endif

/*-- Extern Variables --*/
/* None */

/*-- Static Function Declarations --*/
/* None */

/* Call log_init in main function, it will add log_destroy to atexit too. */
int log_init(int argc, char *argv[]) {
	int ret = 0;

#ifdef  USE_LOG4C
	ret = log4c_init();
	if (0 == ret) {
		atexit(log_destroy);
	}
#endif

	return ret;
}

void log_destroy(void) {
#ifdef  USE_LOG4C
	log4c_fini();
#endif
}

int log_set_category(const char *name) {
#ifdef  USE_LOG4C
	log_category_p = log4c_category_get(name);
#else
//	strlcpy(log_category, name, NAME_MAX);
#endif

	//return RT_OK;
	return 0;
}

