#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include "iotinfo.h"


int main(int argc, char *argv[]){
	//messages to be passed back and forth between the actuator and the controller
	struct device_info msg1;
	struct message_from_controller message_to_receive;

	//flags to keep the program running
	int running = 0;
	int running1 =1;

	pid_t pid = getpid();
	long int message_to_send = 3;

	//initialize and send the first registration message to the controller through the queue
	strcpy(msg1.info.name, argv[1]);
	msg1.my_message_type = message_to_send;
	msg1.info.threshold = 0;
	msg1.info.device_type = ACTUATOR;
	msg1.info.pid = pid;
	msg1.info.sensing_data = 0;
	msg1.info.is_new_device = IS_NOT_REGISTERED;

	int queueID = msgget((key_t)QUEUE, 0666);
	if(queueID == -1){
		fprintf(stderr, "msgget failed with error: %d\n", errno);
        	exit(EXIT_FAILURE);
	}
	if (msgsnd(queueID, (void *)&msg1, sizeof(msg1.info), 0) == -1){
            fprintf(stderr, "msgsnd failed\n");
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
	//flag the device as registered and being sending data
	msg1.info.is_new_device = IS_REGISTERED;
	running = 1;
	while(running){
		//attempt to receive a message from the controller and send back a confirmation
		if (msgrcv(queueID, (void *)&message_to_receive, BUFFSIZE,
                   pid, 0) == -1) {
            		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
            		exit(EXIT_FAILURE);
		} else {
			if (strcmp(message_to_receive.message, STOP) == 0){
				exit(0);
			}
			else 
			{
				printf("%s, turn on fan\n", message_to_receive.message);
				strcpy(message_to_receive.message, "Completed.\n");
				if (msgsnd(queueID, (void *)&msg1, sizeof(BUFFSIZE), 0) == -1){
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}
			}
		}
			
	}
}
