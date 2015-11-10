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

long int msg_to_receive = 0;
int queueID;

//search the list of sensors one with a matching pid
struct device_info find_sensor(struct device_info device){
	for (int i = 0; i < 5; i++)
		if (sensors[i].info.pid == device.info.pid)
			return sensors[i];
}

//send a message to an actuator that corresponds to its sensor that went over its threshold
void send_message_to_actuator(int queueID, struct device_info device){
	struct message_from_controller msg;
	int running = 1;
	for (int i = 0; i < 5; i++)
	{
		if(sensors[i].info.pid == device.info.pid){
			msg.my_message_type = actuators[i].info.pid;
			while(running){
				strcpy(msg.message, "Alert, over threshold");
				if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
		    			fprintf(stderr, "msgsnd failed\n");
		    			exit(EXIT_FAILURE);
				}
				if (msgrcv(queueID, (void *)&msg, BUFFSIZE, msg_to_receive, 0) == -1) {
			    		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			    		exit(EXIT_FAILURE);
				} else {
					printf("%s", msg.message);
					running = 0;
				}
			}
		}
	}


}

//handler for when the child process raises a signal to the parent process
void checkSignal(int sig)
{
	long int type = 4;
	char hello[BUFFSIZE+1];
	int open_mode = O_WRONLY;
	int pipe_fd;
	int res = 0;
	int bytes_sent = 0;

	//receive a message from the child process
	struct message_from_controller msg;
	pipe_fd = open(FIFO_NAME, open_mode);
	if (msgrcv(queueID, (void *)&msg, BUFFSIZE, type, 0) == -1) {
		fprintf(stderr, "2342msgrcv failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pipe_fd ==-1) printf("error\n");

	//send a message to the cloud through a pipe to signal data going over threshold
	strcpy(hello, msg.message);
	if (pipe_fd != -1) {
        	while(bytes_sent < sizeof(hello)) {
            		res = write(pipe_fd, hello, BUFFSIZE);
            		if (res == -1) {
                		fprintf(stderr, "Write error on pipe\n");
                		exit(EXIT_FAILURE);
            		}
            		bytes_sent += res;
        	}
        	(void)close(pipe_fd);
    	}
    	else {
        	exit(EXIT_FAILURE);
    	}
}



void childProcess()
{
	//messages to be passed back and forth between the devices and the controller
	struct message_from_controller msg;
	struct device_info device;

	long int message_to_receive = 3;
	long int type = 4;
	int running = 0;
	int running1 = 0;
	int current_sensor = 0;
	int current_actuator = 0;

	//check to see if a queue has been created
	if (queueID == -1){
		fprintf(stderr, "failed with error: %d\n", errno);
            	exit(EXIT_FAILURE);
        }else{
		running = 1;
		running1 = 1;
	}

	//receive data from the sensor 40 times
	while(running<40)
	{	
		running++;
		//keep attempting to receive data from a device
		while(running1){
			if (msgrcv(queueID, (void *)&device, sizeof(device.info), message_to_receive, 0) == -1) {
		    		fprintf(stderr, "999msgrcv failed with error: %d\n", errno);
		    		exit(EXIT_FAILURE);
			}
			else running1 = 0;
		}
		//check to see if the device it just read from is registered or not
		if (device.info.is_new_device == IS_NOT_REGISTERED){
			//if it not registered, register it and send confirmation
			device.info.is_new_device = IS_REGISTERED;
			if (device.info.device_type == SENSOR){
				sensors[current_sensor] = device;
				current_sensor++;
			}
			else {
				actuators[current_actuator] = device;
				current_actuator++;
			}

			msg.my_message_type = device.info.pid;
			strcpy(msg.message, ACK);
			if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
            			fprintf(stderr, "msgsnd failed1\n");
            			exit(EXIT_FAILURE);
			}

			printf("Registered device %s\n", device.info.name);
		} else {
			//if it has been registered, collect the sensing data from the sensor and returns a confirmation
			msg.my_message_type = device.info.pid;
			strcpy(msg.message, ACK);
			if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
            			fprintf(stderr, "msgsnd failed2 with error: %d\n", errno);
            			exit(EXIT_FAILURE);
			}
			printf("%d from device %s\n", device.info.sensing_data, device.info.name);

			//check to see if the data is over the threshold. If so, send an alert to the actuator and raise a signal to the parent process
			struct device_info existing_device = find_sensor(device);
			if (existing_device.info.threshold < device.info.sensing_data){
				send_message_to_actuator(queueID, device);
				msg.my_message_type = type;
				sprintf(msg.message, "Device that went over threshold: %s", device.info.name);
				if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
            				fprintf(stderr, "msgsnd failed3 with error: %d\n", errno);
            				exit(EXIT_FAILURE);
				}			

				kill(getppid(), SIGINT);
			}
		}
		running1 = 1;
	}

	//after the data has been collection 40 times, end all the processes and delete the queue
	kill (getppid(), 9);
	
	for (int i = 0; i < current_sensor; i++)
	{
		msg.my_message_type = sensors[i].info.pid;
		strcpy(msg.message, STOP);
		if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
		    	fprintf(stderr, "msgsnd failed5 with error: %d\n", errno);
		    	exit(EXIT_FAILURE);
		}
		msg.my_message_type = actuators[i].info.pid;
		strcpy(msg.message, STOP);
		if (msgsnd(queueID, (void *)&msg, BUFFSIZE, 0) == -1) {
		    	fprintf(stderr, "msgsnd failed4 with error: %d\n", errno);
		    	exit(EXIT_FAILURE);
		}
	}

	msgctl(queueID, IPC_RMID, NULL);

	printf("Completed.\n");
	exit(0);

}

void parentProcess()
{	
	//use sigaction to wait for a signal from the child process when the sensing data is over a threshold. Send data to the cloud when signal is received.
	struct sigaction act;
	act.sa_handler = checkSignal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);
	

	while (1) {
	}
	
}


int main(){
	program_running = 1;

	//create the message queue
	queueID = msgget((key_t)QUEUE, 0666 | IPC_CREAT);

	pid_t pid;
	
	//run the parent and child process tasks
	printf("Controller is starting \n");
	pid = fork();
	switch(pid)
	{
	case -1:
		perror("fork failed");
		exit(1);
	case 0:
		childProcess();
		break;
	default:
		parentProcess();
		break;
	}
}

