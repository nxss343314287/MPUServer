#include "unistd.h"
#include "../log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define   COPYMOVE_BUF_SIZE   1024


int copy_sourceFile_to_destFile(char *sourcefile, char * destfile)
{
	int from_fd, to_fd, flag = 0;
	int bytes_read, bytes_write;
	char buffer[COPYMOVE_BUF_SIZE];
	char *ptr;

	remove(destfile);

    if((from_fd= open(sourcefile, O_RDONLY)) == -1)
//    if((from_fd= open("/ss/ATRSQL.db", O_RDONLY)) == -1)
    {
    	printf("open source file error!\n");
    	close(from_fd);
    	return 0;
    }

    if((to_fd= open(destfile, O_WRONLY|O_CREAT|O_TRUNC,
    		S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH|S_IWOTH|S_IXOTH)) == -1)
    {
    	printf("open desti file error!\n");
    	close(from_fd);
    	close(to_fd);
    	return 0;
    }
    unsigned int starttime = get_time_msecond();
    while((bytes_read = read(from_fd, buffer, COPYMOVE_BUF_SIZE)))
    {
    	if(bytes_read == -1) // read fatal error
    	{
    		flag = 1;
    		//printf("read fatal error: %s!\n", strerror(errno));
    		break;
    	}
    	else if(bytes_read > 0)
    	{
    		ptr = buffer;

    		while((bytes_write = write(to_fd, ptr, bytes_read)))
    		{
    			if(bytes_write == -1) //write fatal error
    			{
    				flag = 1;
    				printf("write fatal error!\n");
    				break;
    			}
    			else if(bytes_write == bytes_read) // write all ok
    				break;
    			else if(bytes_write > 0) // continue to write
    			{
    				ptr += bytes_write;
    				bytes_read -= bytes_write;
    			}
    		}

    		if(bytes_write == -1) // occur fatal error when write
    		{
    			flag = 1;
    			//printf("write fatal error!\n");
    		    break;
    		}
    	}
    }
    unsigned int endtime = get_time_msecond();
    if(flag == 0)
    {
    	printf("we spend %d mssend to copy source file to desti file!\n",endtime-starttime);
    	printf("copy file sucess!\n");
    }
    close(from_fd);
    close(to_fd);
    return 1;

}


