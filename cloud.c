#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "iotinfo.h"

int main()
{
	int running = 1;
	int res;
	int pipe_fd;
	int open_mode = O_RDONLY;
	char my_data[BUFFSIZE+1];
	int bytes_read = 0;
	
	//create the fifo and print a confirmation
	res = mkfifo(FIFO_NAME, 0777);
	if (res != 0) {
		fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
		exit(EXIT_FAILURE);
	}

    	printf("Cloud is initialized\n");

	//keep checking for data from the parent process of the controller. Print the data received
	while(running){
		pipe_fd = open(FIFO_NAME, open_mode);
		if(pipe_fd != -1) {
		do {
		    res = read(pipe_fd, my_data, BUFFSIZE);
		    bytes_read += res;
		} while (res > 0);
		printf("%s\n", my_data);	
		(void)close(pipe_fd);
		}
	 }
    
}
