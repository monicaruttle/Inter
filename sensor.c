#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>  
#include <string.h>
#include "iotinfo.h"


int main(int argc, char *argv[]){
	//messages to be passed back and forth between the sensor and the controller
	struct device_info msg1;
	struct message_from_controller message_to_receive;

	//flags to keep the program running
	int running = 0;
	int running1 = 1;

	pid_t pid = getpid();
	long int message_to_send = 3;

	//initialize and send the first registration message to the controller through the queue
	strcpy(msg1.info.name, argv[1]);
	msg1.info.threshold = atoi(argv[2]);
	msg1.my_message_type = message_to_send;
	msg1.info.pid = pid;
	msg1.info.device_type = SENSOR;
	msg1.info.is_new_device = IS_NOT_REGISTERED;
	msg1.info.sensing_data = 0;
	int queueID = msgget((key_t)QUEUE, 0666);
	if(queueID == -1){
		fprintf(stderr, "msgget failed with error: %d\n", errno);
        	exit(EXIT_FAILURE);
	}
	if (msgsnd(queueID, (void *)&msg1, sizeof(msg1.info), 0) == -1){
            fprintf(stderr, "msgsnd4 failed\n");
            exit(EXIT_FAILURE);
        }else{
		while(running1){
			//receive the acknowledgement from the controller and stop if told to
			if (msgrcv(queueID, (void *)&message_to_receive, BUFFSIZE, pid, 0) == -1) {
				fprintf(stderr, "1msgrcv failed with error: %d\n", errno);
				exit(EXIT_FAILURE);
			}else{
				if (strcmp(message_to_receive.message, STOP) == 0){
					exit(0);
				}
				else if (strcmp(message_to_receive.message, ACK) == 0){
					running = 1;
					running1 = 0;
					printf("%s\n", message_to_receive.message);
	
				}		
			}
		}
	}

	//flag the device as registered and begin sending data
	msg1.info.is_new_device = IS_REGISTERED;
	while (running)
	{
		//send random data every 2 seconds to the controller
		sleep(2);
		srand(time(NULL));
		msg1.info.sensing_data = rand() % msg1.info.threshold+10;
		if (msgsnd(queueID, (void *)&msg1, sizeof(msg1.info), 0) == -1) {
			exit(EXIT_FAILURE);
		}
		if (msgrcv(queueID, (void *)&message_to_receive, BUFFSIZE,
                   pid, 0) == -1) {
            		fprintf(stderr, "2msgrcv failed with error: %d\n", errno);
            		exit(EXIT_FAILURE);
		} else {
			//stop if told by controller
			if (strcmp(message_to_receive.message, STOP) == 0){
				running = 0;
			}
		}
	}
}
