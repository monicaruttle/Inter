#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include "iotinfo.h"

struct node *head;

struct device_info find_sensor(struct device_info device){
	
	struct node *iterator = head;
	while (iterator->next->device.info.pid != empty_device.info.pid){
		if (iterator->device.info.pid == device.info.pid){
			return iterator->device;
		}
	}

}

void send_message_to_actuator(int queueID){
	struct message_from_controller msg;
	struct node *iterator = head;
	while (iterator->next->device.info.pid != empty_device.info.pid){
		if (iterator->device.info.device_type == ACTUATOR){
			msg.my_message_type = iterator->device.info.pid;
			strcpy(msg.message, "Alert, over threshold");
			if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
            			fprintf(stderr, "msgsnd failed\n");
            			exit(EXIT_FAILURE);
			}
		}
	}


}

int main(){
	struct message_from_controller msg;
	struct node *current_node;
	struct device_info device;
	long int msg_to_receive = 0;
	int running;

	current_node = malloc(sizeof(struct node));
	current_node->next = 0;
	current_node->device = empty_device;

	head = current_node;
	
	int queueID = msgget((key_t)QUEUE, 0666 | IPC_CREAT);

	if (queueID != -1)
		running = 1;

	while(running)
	{
		if (msgrcv(queueID, (void *)&device, sizeof(device.info), msg_to_receive, 0) == -1) {
            		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
            		exit(EXIT_FAILURE);
        	}

		if (device.info.is_new_device == IS_NOT_REGISTERED){
			struct node *device_node;
			device_node = malloc(sizeof(struct node));
			device_node->device = device;
			current_node->next = device_node;
			current_node = device_node;

			printf("Registered device %s/n", device.info.name);

			msg.my_message_type = device.info.pid;
			strcpy(msg.message, ACK);

			if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
            			fprintf(stderr, "msgsnd failed\n");
            			exit(EXIT_FAILURE);
			}
		} else {
			struct device_info existing_device = find_sensor(device);
			if (existing_device.info.threshold < device.info.sensing_data){
				send_message_to_actuator(queueID);
			}
		}
	}
}



