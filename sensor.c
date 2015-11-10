#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "iotinfo.h"


int main(int argc, char *argv[]){
	struct device_info msg1;
	struct message_from_controller message_to_receive;
	int running;
	pid_t pid = getpid();

	strcpy(msg1.info.name, argv[1]);
	msg1.info.threshold = atoi(argv[2]);
	
	msg1.my_message_type = pid;
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
            fprintf(stderr, "msgsnd failed\n");
            exit(EXIT_FAILURE);
        }

	if (msgrcv(queueID, (void *)&message_to_receive, BUFFSIZE,
                   pid, 0) == -1) {
            fprintf(stderr, "msgrcv failed with error: %d\n", errno);
            exit(EXIT_FAILURE);
        }
	if (message_to_receive.message == STOP)
		exit(0);
	else if (message_to_receive.message == ACK)
		running = 1;
	
	while (running)
	{
		srand(time(NULL));
		msg1.info.sensing_data = rand() % msg1.info.threshold+10;
		if (msgsnd(queueID, (void *)&msg1, sizeof(msg1.info), 0) == -1) 		{
            		fprintf(stderr, "msgsnd failed\n");
            		exit(EXIT_FAILURE);
		}
		if (msgrcv(queueID, (void *)&message_to_receive, BUFFSIZE,
                   pid, IPC_NOWAIT) == -1) {
            		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
            		exit(EXIT_FAILURE);
		}
        
		if (message_to_receive.message == STOP)
			running = 0;
	}
}
